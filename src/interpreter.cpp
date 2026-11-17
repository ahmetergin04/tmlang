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
		report(e.tok.line, " at '" + e.tok.lexeme + "'", e.what());
		had_runtime_error = true;
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

void Interpreter::print_result(const Run& run, const RunOutcome& outcome, const RunExec& execution) {
    std::cout << "run(" << run.machine.lexeme << ", " << run.tape.lexeme << "): ";
    switch (outcome.status) {
        case RunStatus::HALTED:     std::cout << outcome.stop_state << " after " << execution.steps << " steps\n"; break;
        case RunStatus::STUCK:      std::cout << "stuck in stop_state " << outcome.stop_state << " reading " << execution.tape[execution.head]
                                               << " after " << execution.steps << " steps\n"; break;
        case RunStatus::STEP_LIMIT: std::cout << "did not halt within " << STEP_LIMIT
                                               << " steps (stop_state " << outcome.stop_state << ")\n"; break;
    }
    // print up to the last non-blank cell or the head, whichever is further right
    size_t end = execution.tape.size();
    while (end > 0 && execution.tape[end - 1] == "#") --end;
    end = std::max(end, execution.head + 1);
    for (size_t i = 0; i < end; ++i) {
        const Symbol& c = execution.tape[i];   // in range: move_right keeps head < tape.size()
        std::cout << (i == execution.head ? "[" + c + "]" : c) << ' ';
    }
    std::cout << '\n';
}

void move_right(RunExec& x) {
        ++x.head;
        if (x.head == x.tape.size()) x.tape.push_back("#");
}

void move_left(RunExec& x) {
        if (x.tape[x.head] == "~") move_right(x);   // built-in L on ~ moves right instead
        else --x.head;
}

// run a tm from its start state, with whereever the head is
RunOutcome Interpreter::run_tm(const TMachine& tm, RunExec& x) {
	State state = tm.start_state;

    while (!is_halt(state)) {
        if (x.steps == STEP_LIMIT) return {RunStatus::STEP_LIMIT, state};

		// look for the current state + symbol combo in the tmms deltas
        TmAction a;
        auto it = tm.deltas->find({state, x.tape[x.head]});

        if (it != tm.deltas->end()) {
			a = it->second; // succesful find
		}
        else if (x.tape[x.head] == "~") {
		   a = TmAction{state, "~", RIGHT_ARROW};   // default ~ rule
		}
        else {
			return {RunStatus::STUCK, state};
		}

        x.tape[x.head] = a.write;
        if (a.action == RIGHT_ARROW) {
			move_right(x);
		}
        else if (a.action == LEFT_ARROW) {
			--x.head;   // never below 0: reading ~ always moves right
		}
        state = a.next;
        ++x.steps;
    }
    return {RunStatus::HALTED, state};
}

void Interpreter::visit(Set_Literal& e) {
	set_result.clear();
	for (const token& t : e.items) set_result.insert(t.lexeme);
}

void Interpreter::visit(Set_Ref& e) {
	auto it = set_scope->find(e.name.lexeme);
	if (it == set_scope->end()) {
		throw RuntimeError(e.name, "Undefined set '" + e.name.lexeme + "'.");
	}
	set_result = it->second;
}

void Interpreter::visit(Set_Binary& e) {
	Set l = eval_set(*e.left,  *set_scope);
	Set r = eval_set(*e.right, *set_scope);
	Set out;
	if (e.op.type == UNION) {
		std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(out, out.end()));
	}
	else {
		std::set_difference(l.begin(), l.end(), r.begin(), r.end(), std::inserter(out, out.end()));
	}
	set_result = std::move(out);
}

void Interpreter::visit(Alphabet_def& s)  {
	define(env.alphabets,  s.name, eval_set(*s.symbols, env.alphabets));
}

void Interpreter::visit(State_set_def& s) {
	define(env.state_sets, s.name, eval_set(*s.states,  env.state_sets));
}

void Interpreter::visit(Transition_fun& s) {
	Set states = eval_set(*s.states,   env.state_sets);
	Set alpha  = eval_set(*s.alphabet, env.alphabets);
	auto is_symbol = [&](const token& t) {
		return t.type == BLANK || t.type == BLIP || alpha.count(t.lexeme);
	};

	Deltas d;
	for (const Transition& t : s.transitions) {
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
	define(env.trans_funcs, s.name, std::move(d));
}

void Interpreter::visit(Turing_machine& s) {
    Set states = eval_set(*s.states,   env.state_sets);
    Set alpha  = eval_set(*s.alphabet, env.alphabets);
    if (!states.count(s.start.lexeme))
        throw RuntimeError(s.start, "Start state is not in the state set.");

    auto f = env.trans_funcs.find(s.func.lexeme);
    if (f == env.trans_funcs.end())
        throw RuntimeError(s.func, "Undefined transition function '" + s.func.lexeme + "'.");

    bool deciding = s.final_state.state.size() == 2;   // {halt_yes, halt_no}
    for (const auto& [key, act] : f->second) { //loop through the deltas of the machine
		bool bad = false;
		if (deciding) {
			bad = act.next == "halt";
		} else {
			bad = (act.next == "halt_yes" || act.next == "halt_no");
		}
        if (bad) throw RuntimeError(s.func, "Transition enters '" + act.next + "', which this machine does not declare.");
    }
    define(env.machines, s.name, TMachine{std::move(states), std::move(alpha), s.start.lexeme, &f->second, deciding});
}

void Interpreter::visit(Tape_def& s) {
    Set alpha = eval_set(*s.alphabet, env.alphabets);
    std::vector<Symbol> cells;
    for (const token& c : s.cells) {
        if (c.type == STRING && !alpha.count(c.lexeme)) {
            throw RuntimeError(c, "Tape symbol is not in the alphabet.");
		}
        cells.push_back(c.lexeme);
    }
    define(env.tapes, s.name, Tape{std::move(alpha), std::move(cells)});
}

void Interpreter::visit(Run& run) {
    auto machine = env.machines.find(run.machine.lexeme);
    if (machine == env.machines.end()) {     // later: also check env.ctms
        throw RuntimeError(run.machine, "Undefined machine '" + run.machine.lexeme + "'.");
	}

    auto tape = env.tapes.find(run.tape.lexeme);
    if (tape == env.tapes.end()) {
        throw RuntimeError(run.tape, "Undefined tape '" + run.tape.lexeme + "'.");
	}

	RunExec execution{tape->second.cells};
	RunOutcome outcome = run_tm(machine->second, execution);
	print_result(run, outcome, execution);
}

void Interpreter::visit(Composite_tm& stmt) {}
void Interpreter::visit(WildCard& expr) {}
void Interpreter::visit(Logical& expr) {}
void Interpreter::visit(Membership& expr) {}
void Interpreter::visit(Equality& expr) {}
