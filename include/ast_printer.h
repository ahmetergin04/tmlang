#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include <sstream>
#include <string>
#include <vector>

#include "stmt.h"
#include "visitor.h"

// Prints the tree in a Lisp-like form, e.g. (alphabet numbers {"1" "2"})
class ast_printer : public Visitor {
private:
    std::ostringstream out; // visit methods return void, so the result is collected here

    void print_tokens(const std::vector<token>& tokens, const std::string& sep);
    void print_set(expr* set); // null prints "_"

public:
    std::string print(expr& e);
    std::string print(stmt& s);

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
};

#endif
