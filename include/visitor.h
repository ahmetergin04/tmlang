#ifndef VISITOR_H
#define VISITOR_H

struct expr;
struct Set_Literal;
struct Set_Ref;
struct Set_Binary;
struct WildCard;
struct Logical;
struct Membership;
struct Equality;

struct stmt;
struct Alphabet_def;
struct State_set_def;
struct Transition_fun;
struct Turing_machine;
struct Tape_def;
struct Composite_tm;
struct Run;

struct Visitor {
    public:
        virtual void visit(Set_Literal& expr) = 0;
        virtual void visit(Set_Ref& expr) = 0;
        virtual void visit(Set_Binary& expr) = 0;
        virtual void visit(WildCard& expr) = 0;
        virtual void visit(Logical& expr) = 0;
        virtual void visit(Membership& expr) = 0;
        virtual void visit(Equality& expr) = 0;

        virtual void visit(Alphabet_def& stmt) = 0;
        virtual void visit(State_set_def& stmt) = 0;
        virtual void visit(Transition_fun& stmt) = 0;
        virtual void visit(Turing_machine& stmt) = 0;
        virtual void visit(Tape_def& stmt) = 0;
        virtual void visit(Composite_tm& stmt) = 0;
        virtual void visit(Run& stmt) = 0;

        virtual ~Visitor() = default;
};


#endif
