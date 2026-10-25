#pragma once 

#include "expr.h"

struct stmt {
    virtual ~stmt() = default;
    virtual void accept(Visitor& visitor) = 0;
};
// data structures for statements
struct Transition  {
    token state;
    token symbol;
    token new_state;
    token new_symbol;
    token direction;
};

struct Final_state {
    std::vector<token> state;
};

struct Ctm_row  {
    token state;
    std::unique_ptr<expr> condition;
    std::vector<token> next_tm;
};
// end 
struct Alphabet_def : public stmt {
    token name;
    std::unique_ptr<expr> symbols;

    Alphabet_def(token name, std::unique_ptr<expr> symbols)
        : name(name), symbols(std::move(symbols)) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct State_set_def : public stmt {
    token name;
    std::unique_ptr<expr> states;

    State_set_def(token name, std::unique_ptr<expr> states)
        : name(name), states(std::move(states)) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Transition_fun : public stmt {
    token name;
    std::unique_ptr<expr> states;
    std::unique_ptr<expr> alphabet;
    std::vector<Transition> transitions;

    Transition_fun(token name, std::unique_ptr<expr> states, std::unique_ptr<expr> alphabet, std::vector<Transition> transitions)
        : name(name), states(std::move(states)), alphabet(std::move(alphabet)), transitions(std::move(transitions)) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Turing_machine : public stmt {
    token name;
    std::unique_ptr<expr> states;
    std::unique_ptr<expr> alphabet;
    token start;
    token func;
    Final_state final_state;

    Turing_machine(token name, std::unique_ptr<expr> states, std::unique_ptr<expr> alphabet, token start, token func, Final_state final_state)
        : name(name), states(std::move(states)), alphabet(std::move(alphabet)), start(start), func(func), final_state(final_state) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Tape_def : public stmt {
    token name;
    std::unique_ptr<expr> alphabet;
    std::vector<token> cells;

    Tape_def(token name, std::unique_ptr<expr> alphabet, std::vector<token> cells)
        : name(name), alphabet(std::move(alphabet)), cells(std::move(cells)) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Composite_tm : public stmt {
    token name;
    std::unique_ptr<expr> alphabet; // can be null for "_"
    std::vector<Ctm_row> rows;

    Composite_tm(token name, std::unique_ptr<expr> alphabet, std::vector<Ctm_row> rows)
        : name(name), alphabet(std::move(alphabet)), rows(std::move(rows)) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Run : public stmt {
    token machine;
    token tape;
    Run(token machine, token tape)
        : machine(machine), tape(tape) {}
    
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};


