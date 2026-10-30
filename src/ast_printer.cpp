
#include "ast_printer.h"

std::string ast_printer::print(expr& e) {
    this->out.str("");
    e.accept(*this);
    return this->out.str();
}

std::string ast_printer::print(stmt& s) {
    this->out.str("");
    s.accept(*this);
    return this->out.str();
}

//helpers

void ast_printer::print_tokens(const std::vector<token>& tokens, const std::string& sep) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (i > 0) this->out << sep;
        this->out << tokens[i].lexeme;
    }
}

void ast_printer::print_set(expr* set) {
    if (set == nullptr) this->out << "_";
    else set->accept(*this);
}

//expressions

void ast_printer::visit(Set_Literal& expr) {
    this->out << "{";
    print_tokens(expr.items, " ");
    this->out << "}";
}

void ast_printer::visit(Set_Ref& expr) {
    this->out << expr.name.lexeme;
}

void ast_printer::visit(Set_Binary& expr) {
    this->out << "(" << expr.op.lexeme << " ";
    expr.left->accept(*this);
    this->out << " ";
    expr.right->accept(*this);
    this->out << ")";
}

void ast_printer::visit(WildCard& expr) {
    this->out << expr.name.lexeme;
}

void ast_printer::visit(Logical& expr) {
    this->out << "(" << (expr.op.type == OR ? "or" : "and") << " ";
    expr.left->accept(*this);
    this->out << " ";
    expr.right->accept(*this);
    this->out << ")";
}

void ast_printer::visit(Membership& expr) {
    this->out << "(in head ";
    expr.set->accept(*this);
    this->out << ")";
}

void ast_printer::visit(Equality& expr) {
    this->out << "(" << expr.op.lexeme << " head " << expr.symbol.lexeme << ")";
}

//statements

void ast_printer::visit(Alphabet_def& stmt) {
    this->out << "(alphabet " << stmt.name.lexeme << " ";
    stmt.symbols->accept(*this);
    this->out << ")";
}

void ast_printer::visit(State_set_def& stmt) {
    this->out << "(state_set " << stmt.name.lexeme << " ";
    stmt.states->accept(*this);
    this->out << ")";
}

void ast_printer::visit(Transition_fun& stmt) {
    this->out << "(transition_func " << stmt.name.lexeme << " ";
    stmt.states->accept(*this);
    this->out << " ";
    stmt.alphabet->accept(*this);
    for (const Transition& t : stmt.transitions) {
        this->out << "\n  (" << t.state.lexeme << " " << t.symbol.lexeme << " "
                  << t.new_state.lexeme << " " << t.new_symbol.lexeme << " "
                  << t.direction.lexeme << ")";
    }
    this->out << ")";
}

void ast_printer::visit(Turing_machine& stmt) {
    this->out << "(tm " << stmt.name.lexeme << " ";
    stmt.states->accept(*this);
    this->out << " ";
    stmt.alphabet->accept(*this);
    this->out << " " << stmt.start.lexeme << " " << stmt.func.lexeme << " {";
    print_tokens(stmt.final_state.state, " ");
    this->out << "})";
}

void ast_printer::visit(Tape_def& stmt) {
    this->out << "(tape " << stmt.name.lexeme << " ";
    stmt.alphabet->accept(*this);
    this->out << " [";
    print_tokens(stmt.cells, " ");
    this->out << "])";
}

void ast_printer::visit(Composite_tm& stmt) {
    this->out << "(ctm " << stmt.name.lexeme << " ";
    print_set(stmt.alphabet.get());
    for (const Ctm_row& row : stmt.rows) {
        this->out << "\n  (" << row.state.lexeme << " ";
        row.condition->accept(*this);
        this->out << " ";
        print_tokens(row.next_tm, ".");
        this->out << ")";
    }
    this->out << ")";
}

void ast_printer::visit(Run& stmt) {
    this->out << "(run " << stmt.machine.lexeme << " " << stmt.tape.lexeme << ")";
}
