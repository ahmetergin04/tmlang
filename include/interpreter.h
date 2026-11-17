#pragma once

#include <stdexcept>

#include "stmt.h"
#include "visitor.h"
#include "environment.h"

using std::unique_ptr;

struct RuntimeError : std::runtime_error {
    token tok;
    RuntimeError(const token& t, const std::string& msg) : std::runtime_error(msg), tok(t) {}
};

enum class RunStatus { HALTED, STUCK, STEP_LIMIT };

struct RunExec {
    std::vector<Symbol> tape;
    size_t head = 1;
    int steps = 0;
};

struct RunOutcome {
    RunStatus status;
    State stop_state;
};

class Interpreter : public Visitor {
public:
	Interpreter(Environment& env) : env(env) {}
	void interpret(const std::vector<std::unique_ptr<stmt>>& statements);

private:
	void visit(Set_Literal& expr) override;
	void visit(Set_Ref& expr) override;
	void visit(Set_Binary& expr) override;
	void visit(WildCard& expr) override;
	void visit(Logical& expr) override;
	void visit(Membership& expr) override;
	void visit(Equality& expr) override;

	void visit(Alphabet_def& stmt) override;
	void visit(State_set_def& stmt) override;
	void visit(Transition_fun& stmt) override;
	void visit(Turing_machine& stmt) override;
	void visit(Tape_def& stmt) override;
	void visit(Composite_tm& stmt) override;
	void visit(Run& stmt) override;
	 
	Environment& env;

	Set set_result; // helper member for the 3 different declaration methods of a set (state or alphabet)
	const std::map<std::string, Set>* set_scope = nullptr;

    // ctm conditions: current_head_symbol goes in, condition_result comes out
    Symbol current_head_symbol;
    bool condition_result = false;

    RunOutcome run_tm(const TMachine& tm, RunExec& exec);
    RunOutcome run_ctm(const Ctm& ctm, RunExec& exec);

	void print_result(const Run& s, const RunOutcome& o, const RunExec& x);

    // helpers
	Set eval_set(expr& e, const std::map<std::string, Set>& scope);
    const CtmRow* pick_row(const Ctm& ctm, const std::string& label, const RunExec& exec);
	void check_machine_name(const token& name);
    void check_condition_def(expr& condition, const Ctm& ctm); //ctm conditions to be evaluated
    CtmAction resolve_action(const token& step, bool is_last, const Set& labels, const Ctm& ctm); //turns one chain item of a ctm in to a CtmAction object
};

//templated func to define things in the environment maps of the interpreter
template <typename T>
void define(std::map<std::string, T>& m, const token& name, T value) {
    if (!m.emplace(name.lexeme, std::move(value)).second) {
        throw RuntimeError(name, "'" + name.lexeme + "' is already defined.");
	}
}
