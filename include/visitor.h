#ifndef VISITOR_H
#define VISITOR_H
#include <memory>

class Expr;
class Alphabet;
class State_set;
class Action;
class Transition;
class Final_state;
class Condition;
class Or_logic;
class And_logic;
class Equality;
class Next_tm;
class Ctm_row;

class Stmt;
class Alphabet_def;
class State_set_def;
class Transition_fun;
class Turing_machine;
class Tape_def;
class Run;
class Composite_tm;

class Visitor {
    public:
        virtual ~Visitor () = default;
        virtual void Visit_Alphabet_Expr (const Alphabet&) = 0;
        virtual void Visit_State_set_Expr (const State_set&) = 0;
        virtual void Visit_Action_Expr (const Action&) = 0;
        virtual void Visit_Transition_Expr (const Transition&) = 0;
        virtual void Visit_Final_state_Expr (const Final_state&) = 0;
        virtual void Visit_Condition_Expr (const Condition&) = 0;
        virtual void Visit_Or_logic_Expr (const Or_logic&) = 0;
        virtual void Visit_And_logic_Expr (const And_logic&) = 0;
        virtual void Visit_Equality_Expr (const Equality&) = 0;
        virtual void Visit_Next_tm_Expr (const Next_tm&) = 0;
        virtual void Visit_Ctm_row_Expr (const Ctm_row&) = 0;

        virtual void Visit_Alphabet_def_Stmt (const Alphabet_def&) = 0;
        virtual void Visit_State_set_def_Stmt (const State_set_def&) = 0;
        virtual void Visit_Transition_fun_Stmt (const Transition_fun&) = 0;
        virtual void Visit_Turing_machine_Stmt (const Turing_machine&) = 0;
        virtual void Visit_Tape_def_Stmt (const Tape_def&) = 0;
        virtual void Visit_Run_Stmt (const Run&) = 0;
        virtual void Visit_Composite_tm_Stmt (const Composite_tm&) = 0;
};


#endif
