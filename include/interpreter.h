#pragma once

#include "stmt.h"
#include "visitor.h"

using std::unique_ptr;

class Interpreter : public Visitor {
public:
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
private:
};
