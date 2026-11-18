#include <algorithm>
#include <iterator>
#include <iostream>

#include "interpreter.h"
#include "tm_error.h"

static constexpr int STEP_LIMIT = 10000;

void Interpreter::interpret(const std::vector<std::unique_ptr<stmt>>& statements) {
    try {
        for (const auto& s : statements) s->accept(*this);
    } catch (const RuntimeError& e) {
        runtime_error_report(e.tok.line, " at '" + e.tok.lexeme + "'", e.what());
    }
}

// set could be either state sets or alphabets so the caller will pass in the right map to differantiate
Set Interpreter::eval_set(expr& e, const std::map<std::string, Set>& scope) {
    auto saved = set_scope;
    set_scope = &scope;
    e.accept(*this);
    set_scope = saved;
    return std::move(set_result);
}

bool is_halt(const State& s) {
    return s == "halt" || s == "halt_yes" || s == "halt_no"; 
}

// the tape up to the last non-blank cell or the head, whichever is further right, head in [ ]
std::string format_tape(const RunExec& execution) {
    size_t end = execution.tape.size();
    while (end > 0 && execution.tape[end - 1] == "#") --end;
    end = std::max(end, execution.head + 1);
    std::string text;
    for (size_t i = 0; i < end; ++i) {
        const Symbol& cell = execution.tape[i];   // in range: move_right keeps head < tape.size()
        text += (i == execution.head ? "[" + cell + "]" : cell) + ' ';
    }
    return text;
}

// how a run ended, e.g. "halt after 4 steps"; steps is passed in because a call's own step count differs from the run's
std::string format_outcome(const RunOutcome& outcome, const RunExec& execution, int steps) {
    switch (outcome.status) {
        case RunStatus::HALTED:
            return outcome.stop_state + " after " + std::to_string(steps) + " steps";
        case RunStatus::STUCK:
            return "stuck in stop_state " + outcome.stop_state + " reading " + execution.tape[execution.head]
                + " after " + std::to_string(steps) + " steps";
        case RunStatus::STEP_LIMIT:
            return "did not halt within " + std::to_string(STEP_LIMIT) + " steps (stop_state " + outcome.stop_state + ")";
    }
    return "";
}

void Interpreter::print_result(const Run& run, const RunOutcome& outcome, const RunExec& execution) {
    std::cout << "run(" << run.machine.lexeme << ", " << run.tape.lexeme << "): "
              << format_outcome(outcome, execution, execution.steps) << '\n'
              << format_tape(execution) << '\n';
}

// prints one node and its children as an ascii tree
void Interpreter::print_trace(const TraceNode& node, const std::string& prefix, bool is_last) {
    std::cout << prefix << (is_last ? "`-- " : "|-- ") << node.text << '\n';
    std::string child_prefix = prefix + (is_last ? "    " : "|   ");
    for (size_t i = 0; i < node.children.size(); ++i) {
        print_trace(node.children[i], child_prefix, i + 1 == node.children.size());
    }
}

// every machine call (from run or from a ctm chain) goes through here so the trace sees it
RunOutcome Interpreter::call_machine(const std::string& name, RunExec& exec) {
    bool is_ctm = env.ctms.count(name) > 0;
    auto run_machine = [&] {
        return is_ctm ? run_ctm(env.ctms.at(name), exec) : run_tm(env.machines.at(name), exec);
    };
    if (!options.print_machine_trace) return run_machine();

    // the parent's children only grow after this call returns, so this reference stays valid
    TraceNode& node = current_trace->children.emplace_back();
    int steps_before = exec.steps;

    TraceNode* parent = current_trace;
    current_trace = &node;            // calls made inside this machine attach under it
    RunOutcome outcome = run_machine();
    current_trace = parent;

    node.text = name + (is_ctm ? " (ctm): " : " (tm): ")
        + format_outcome(outcome, exec, exec.steps - steps_before)
        + "  |  " + format_tape(exec);
    return outcome;
}

void move_right(RunExec& x) {
    ++x.head;
    if (x.head == x.tape.size()) x.tape.push_back("#");
}

void move_left(RunExec& x) {
    if (x.tape[x.head] == "~") move_right(x);   // built-in L on ~ moves right instead
    else --x.head;
}

// helper to check for alphabet rules
// a (= b
bool subset(const Set& a, const Set& b) {
    return std::includes(b.begin(), b.end(), a.begin(), a.end());
}

// run a tm from its start state, with whereever the head is
RunOutcome Interpreter::run_tm(const TMachine& tm, RunExec& exec) {
    State state = tm.start_state;

    while (!is_halt(state)) {
        if (exec.steps == STEP_LIMIT) return {RunStatus::STEP_LIMIT, state};

        // look for the current state + symbol combo in the tmms deltas
        TmAction a;
        auto it = tm.deltas->find({state, exec.tape[exec.head]});

        if (it != tm.deltas->end()) {
            a = it->second; // succesful find
        }
        else if (exec.tape[exec.head] == "~") {
            a = TmAction{state, "~", RIGHT_ARROW};   // default ~ rule
        }
        else {
            return {RunStatus::STUCK, state};
        }

        exec.tape[exec.head] = a.write;
        if (a.action == RIGHT_ARROW) {
            move_right(exec);
        }
        else if (a.action == LEFT_ARROW) {
            --exec.head;   // never below 0: reading ~ always moves right
        }
        state = a.next;
        ++exec.steps;
    }
    return {RunStatus::HALTED, state};
}

// the first row with this label whose condition holds for the symbol under the head
const CtmRow* Interpreter::pick_row(const Ctm& ctm, const std::string& label, const RunExec& exec) {
    current_head_symbol = exec.tape[exec.head];   // input for the condition visitors

    for (const CtmRow& row : ctm.rows) {
        if (row.label != label) continue;
        row.condition->accept(*this);             // writes condition_result
        if (condition_result) return &row;
    }
    return nullptr;
}

// runs a ctm: execute the > row's chain, then keep jumping to rows by label
RunOutcome Interpreter::run_ctm(const Ctm& ctm, RunExec& exec) {
    const CtmRow* row = &ctm.rows[0];   // the > row: the parser puts it first, and its condition is _

    while (true) {
        for (const CtmAction& action : row->chain) {
            if (exec.steps == STEP_LIMIT) {
                return {RunStatus::STEP_LIMIT, row->label};
            }

            switch (action.type) {
                case CtmActionType::MOVE_RIGHT:
                    move_right(exec);
                    ++exec.steps;
                    break;

                case CtmActionType::MOVE_LEFT:
                    move_left(exec);
                    ++exec.steps;
                    break;

                case CtmActionType::WRITE:
                    if (exec.tape[exec.head] == "~") {
                        return {RunStatus::STUCK, row->label};   // ~ can never be overwritten
                    }
                    exec.tape[exec.head] = action.name;
                    ++exec.steps;
                    break;

                case CtmActionType::CALL_TM:
                case CtmActionType::CALL_CTM: {
                    RunOutcome called_outcome = call_machine(action.name, exec);
                    if (called_outcome.status != RunStatus::HALTED) {
                        return called_outcome;   // stuck or step limit inside the called machine ends the whole run
                    }
                    break;                       // any halt state: continue the chain
                }

                case CtmActionType::HALT:
                    return {RunStatus::HALTED, action.name};

                case CtmActionType::GOTO:
                    ++exec.steps;   // counts as a step, so (A, head == #, A) still hits the limit
                    break;
            }
        }

        // the chain did not halt, so its last action is a GOTO (guaranteed by visit(Composite_tm&))
        const std::string& next_label = row->chain.back().name;
        row = pick_row(ctm, next_label, exec);
        if (row == nullptr) {
            return {RunStatus::STUCK, next_label};   // no row of this label matches the head symbol
        }
    }
}

void Interpreter::check_machine_name(const token& name) {
    if (name.lexeme == "R" || name.lexeme == "L")
        throw RuntimeError(name, "'" + name.lexeme + "' is a built-in machine.");
    if (env.machines.count(name.lexeme) || env.ctms.count(name.lexeme))
        throw RuntimeError(name, "'" + name.lexeme + "' is already defined.");
}

// check if a ctm condition is valid
void Interpreter::check_condition_def(expr& condition, const Ctm& ctm) {
    if (auto* logical = dynamic_cast<Logical*>(&condition)) {
        check_condition_def(*logical->left, ctm);
        check_condition_def(*logical->right, ctm);
    }
    else if (auto* equality = dynamic_cast<Equality*>(&condition)) {
        const token& symbol = equality->symbol;
        if (symbol.type == BLANK) return;
        if (ctm.any_alphabet) {
            throw RuntimeError(symbol, "A '_' CTM can only compare the head with '#'.");
        }
        if (!ctm.alphabet.count(symbol.lexeme)) {
            throw RuntimeError(symbol, "Symbol is not in the CTM's alphabet.");
        }
    }
    else if (auto* membership = dynamic_cast<Membership*>(&condition)) {
        if (ctm.any_alphabet) {
            throw RuntimeError(membership->in, "A '_' CTM cannot use 'head in'.");
        }
        if (!subset(eval_set(*membership->set, env.alphabets), ctm.alphabet)) {
            throw RuntimeError(membership->in, "Set is not a subset of the CTM's alphabet.");
        }
    }
    // WildCard: always allowed
}

// the last token of a ctm action chain must be another row of the ctm or halt
// before that it can be write, built in (L or R), a ctm or a tm
CtmAction Interpreter::resolve_action(const token& step, bool is_last, const Set& labels, const Ctm& ctm) {
    switch (step.type) {
        case HALT: case HALT_YES: case HALT_NO: //last step halt
            if (!is_last) {
                throw RuntimeError(step, "A halt state must be the last step of a chain.");
            }
            return {CtmActionType::HALT, step.lexeme};

        case STRING: case BLANK: case BLIP: // write action
            if (step.type != BLANK) {
                if (ctm.any_alphabet) {
                    throw RuntimeError(step, "A '_' CTM can only write '#'.");
                }
                if (!ctm.alphabet.count(step.lexeme)) {
                    throw RuntimeError(step, "Written symbol is not in the CTM's alphabet.");
                }
            }
            return {CtmActionType::WRITE, step.lexeme};

        default:
            break;   // IDENTIFIER: handled below
    }

    const std::string& name = step.lexeme;

    if (labels.count(name)) {
        if (!is_last) {
            throw RuntimeError(step, "A row label must be the last step of a chain.");
        }
        return {CtmActionType::GOTO, name}; // last step another row
    }

    if (name == "R") return {CtmActionType::MOVE_RIGHT, name}; // built-in actions
    if (name == "L") return {CtmActionType::MOVE_LEFT, name};

    if (auto found_tm = env.machines.find(name); found_tm != env.machines.end()) { // call tm
        if (ctm.any_alphabet) {
            throw RuntimeError(step, "A '_' CTM can only call R, L and other '_' CTMs.");
        }
        if (!subset(ctm.alphabet, found_tm->second.alphabet)) {
            throw RuntimeError(step, "The CTM's alphabet is not a subset of the alphabet of '" + name + "'.");
        }
        return {CtmActionType::CALL_TM, name};
    }

    if (auto found_ctm = env.ctms.find(name); found_ctm != env.ctms.end()) { // call ctm
        if (!found_ctm->second.any_alphabet) {
            if (ctm.any_alphabet) {
                throw RuntimeError(step, "A '_' CTM can only call R, L and other '_' CTMs.");
            }
            if (!subset(ctm.alphabet, found_ctm->second.alphabet)) {
                throw RuntimeError(step, "The CTM's alphabet is not a subset of the alphabet of '" + name + "'.");
            }
        }
        return {CtmActionType::CALL_CTM, name};
    }

    throw RuntimeError(step, "Undefined machine or row label '" + name + "'.");
}

void Interpreter::visit(Set_Literal& set) {
    set_result.clear();
    for (const token& t : set.items) set_result.insert(t.lexeme);
}

void Interpreter::visit(Set_Ref& set) {
    auto it = set_scope->find(set.name.lexeme);
    if (it == set_scope->end()) {
        throw RuntimeError(set.name, "Undefined set '" + set.name.lexeme + "'.");
    }
    set_result = it->second;
}

void Interpreter::visit(Set_Binary& set) {
    Set l = eval_set(*set.left,  *set_scope);
    Set r = eval_set(*set.right, *set_scope);
    Set out;
    if (set.op.type == UNION) {
        std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(out, out.end()));
    }
    else {
        std::set_difference(l.begin(), l.end(), r.begin(), r.end(), std::inserter(out, out.end()));
    }
    set_result = std::move(out);
}

void Interpreter::visit(Alphabet_def& alp_def)  {
    define(env.alphabets,  alp_def.name, eval_set(*alp_def.symbols, env.alphabets));
}

void Interpreter::visit(State_set_def& state_set_def) {
    define(env.state_sets, state_set_def.name, eval_set(*state_set_def.states,  env.state_sets));
}

void Interpreter::visit(Transition_fun& t_func) {
    Set states = eval_set(*t_func.states,   env.state_sets);
    Set alpha  = eval_set(*t_func.alphabet, env.alphabets);
    auto is_symbol = [&](const token& t) {
        return t.type == BLANK || t.type == BLIP || alpha.count(t.lexeme);
    };

    Deltas d;
    for (const Transition& t : t_func.transitions) {
        if (!states.count(t.state.lexeme))
            throw RuntimeError(t.state, "State is not in the state set.");
        if (!is_symbol(t.symbol) || !is_symbol(t.new_symbol))
            throw RuntimeError(t.symbol, "Symbol is not in the alphabet.");
        if (t.new_state.type == STRING && !states.count(t.new_state.lexeme))
            throw RuntimeError(t.new_state, "Next state is not in the state set.");
        if (t.symbol.type == BLIP && (t.new_symbol.type != BLIP || t.direction.type != RIGHT_ARROW))
            throw RuntimeError(t.symbol, "Reading '~' must write '~' and move '->'.");
        if (t.symbol.type != BLIP && t.new_symbol.type == BLIP)
            throw RuntimeError(t.new_symbol, "'~' cannot be written.");

        auto key = std::pair{t.state.lexeme, t.symbol.lexeme};
        if (!d.emplace(key, TmAction{t.new_state.lexeme, t.new_symbol.lexeme, t.direction.type}).second)
            throw RuntimeError(t.state, "Duplicate transition for this (state, symbol).");
    }
    define(env.trans_funcs, t_func.name, std::move(d));
}

void Interpreter::visit(Turing_machine& tm) {
    check_machine_name(tm.name);
    Set states = eval_set(*tm.states,   env.state_sets);
    Set alpha  = eval_set(*tm.alphabet, env.alphabets);
    if (!states.count(tm.start.lexeme)) {
        throw RuntimeError(tm.start, "Start state is not in the state set.");
    }

    auto f = env.trans_funcs.find(tm.func.lexeme);
    if (f == env.trans_funcs.end()) {
        throw RuntimeError(tm.func, "Undefined transition function '" + tm.func.lexeme + "'.");
    }

    bool deciding = tm.final_state.state.size() == 2;   // {halt_yes, halt_no}
    for (const auto& [key, act] : f->second) { //loop through the deltas of the machine
        bool bad = false;
        if (deciding) {
            bad = act.next == "halt";
        } else {
            bad = (act.next == "halt_yes" || act.next == "halt_no");
        }
        if (bad) throw RuntimeError(tm.func, "Transition enters '" + act.next + "', which this machine does not declare.");
    }
    define(env.machines, tm.name, TMachine{std::move(states), std::move(alpha), tm.start.lexeme, &f->second, deciding});
}

void Interpreter::visit(Tape_def& tape_def) {
    Set alpha = eval_set(*tape_def.alphabet, env.alphabets);
    std::vector<Symbol> cells;
    for (const token& c : tape_def.cells) {
        if (c.type == STRING && !alpha.count(c.lexeme)) {
            throw RuntimeError(c, "Tape symbol is not in the alphabet.");
        }
        cells.push_back(c.lexeme);
    }
    define(env.tapes, tape_def.name, Tape{std::move(alpha), std::move(cells)});
}

void Interpreter::visit(Run& run) {
    auto t = env.tapes.find(run.tape.lexeme);
    if (t == env.tapes.end()) {
        throw RuntimeError(run.tape, "Undefined tape '" + run.tape.lexeme + "'.");
    }
    const Tape& tape = t->second;

    RunExec exec {tape.cells};   // a copy: the declared tape can be reused

    if (auto found_tm = env.machines.find(run.machine.lexeme); found_tm != env.machines.end()) {
        if (!subset(tape.alphabet, found_tm->second.alphabet)) {
            throw RuntimeError(run.tape, "The tape's alphabet is not a subset of the alphabet of '" + run.machine.lexeme + "'.");
        }
    }
    else if (auto found_ctm = env.ctms.find(run.machine.lexeme); found_ctm != env.ctms.end()) {
        const Ctm& ctm = found_ctm->second;
        if (!ctm.any_alphabet && !subset(tape.alphabet, ctm.alphabet)) {
            throw RuntimeError(run.tape, "The tape's alphabet is not a subset of the alphabet of '" + run.machine.lexeme + "'.");
        }
    }
    else {
        throw RuntimeError(run.machine, "Undefined machine '" + run.machine.lexeme + "'.");
    }

    TraceNode trace_root;            // holds the top-level call
    current_trace = &trace_root;
    RunOutcome outcome = call_machine(run.machine.lexeme, exec);
    current_trace = nullptr;

    print_result(run, outcome, exec);
    if (options.print_machine_trace) {
        print_trace(trace_root.children.front(), "", true);
    }
}

// defining a ctm in the environment
void Interpreter::visit(Composite_tm& ctm_def) {
    check_machine_name(ctm_def.name);

    Ctm ctm;
    ctm.any_alphabet = ctm_def.alphabet == nullptr;
    if (!ctm.any_alphabet) {
        ctm.alphabet = eval_set(*ctm_def.alphabet, env.alphabets);
    }

    Set labels;
    for (const Ctm_row& ast_row : ctm_def.rows) {
        if (ast_row.state.type != IDENTIFIER) continue; // the > row
        const std::string& label = ast_row.state.lexeme;
        if (label == "R" || label == "L" || env.machines.count(label) || env.ctms.count(label)) {
            throw RuntimeError(ast_row.state, "Row label '" + label + "' has the same name as a machine.");
        }
        labels.insert(label);
    }

    for (Ctm_row& ast_row : ctm_def.rows) {
        check_condition_def(*ast_row.condition, ctm);

        CtmRow row{ast_row.state.lexeme, ast_row.condition.get(), {}};
        for (size_t i = 0; i < ast_row.next_tm.size(); ++i) {
            bool is_last = (i + 1 == ast_row.next_tm.size());
            row.chain.push_back(resolve_action(ast_row.next_tm[i], is_last, labels, ctm));
        }

        CtmActionType end_type = row.chain.back().type;
        if (end_type != CtmActionType::HALT && end_type != CtmActionType::GOTO) {
            throw RuntimeError(ast_row.next_tm.back(), "A chain must end with a halt state or a row label.");
        }
        ctm.rows.push_back(std::move(row));
    }

    define(env.ctms, ctm_def.name, std::move(ctm));
}

// _ in the > row: always true
void Interpreter::visit(WildCard& wildcard) {
    condition_result = true;
}

void Interpreter::visit(Equality& equality) {
    bool head_matches = (current_head_symbol == equality.symbol.lexeme);
    bool is_equal_op  = (equality.op.type == EQUAL_EQUAL);
    condition_result  = (head_matches == is_equal_op);
}

void Interpreter::visit(Membership& membership) {
    Set symbols = eval_set(*membership.set, env.alphabets);
    condition_result = symbols.count(current_head_symbol) > 0;
}

void Interpreter::visit(Logical& logical) {
    logical.left->accept(*this);
    bool left_result = condition_result;

    if (logical.op.type == OR ? left_result : !left_result) return;

    logical.right->accept(*this);
}
