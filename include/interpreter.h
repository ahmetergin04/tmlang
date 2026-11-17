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

struct RunResult {
    RunStatus status;
    State stop_state;
    std::vector<Symbol> tape;
    size_t head;
    int steps;
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

	Set set_result;
	const std::map<std::string, Set>* set_scope = nullptr;
	Set eval_set(expr& e, const std::map<std::string, Set>& scope);

	RunResult simulate(const TMachine& m, std::vector<Symbol> tape);
	void print_result(const Run& s, const RunResult& r);
};

//templated func to define things in the environment maps of the interpreter
template <typename T>
void define(std::map<std::string, T>& m, const token& name, T value) {
    if (!m.emplace(name.lexeme, std::move(value)).second) {
        throw RuntimeError(name, "'" + name.lexeme + "' is already defined.");
	}
}
