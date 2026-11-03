
#include "parser.h"
#include "token.h"

//basic parser helpers

const token& parser::peek() {
    return tokens[current];
}

const token& parser::previous() {
    return tokens[current - 1];
}

const token& parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool parser::check(token_type type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool parser::match(std::initializer_list<token_type> types) {
    for (token_type type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const token& parser::consume(token_type type, const std::string& message) {
    if (check(type)) return advance();
    
    throw Perror(peek(), message);
}

bool parser::isAtEnd() { 
    return peek().type == ENDOFFILE; 
}

//helpers for parsing expressions and statements

// "{" STRING ( "," STRING )* "}"
std::unique_ptr<expr> parser::string_set() {
    consume(LEFT_BRACE, "Expect '{' to start a set.");
    std::vector<token> items;
    do {
        items.push_back(consume(STRING, "Expect a string in set."));
    } while (match({COMMA}));
    consume(RIGHT_BRACE, "Expect '}' after set.");
    return std::make_unique<Set_Literal>(std::move(items));
}

// IDENTIFIER | string_set => set name or set literal
std::unique_ptr<expr> parser::set_or_ref() {
    if (match({IDENTIFIER})) return std::make_unique<Set_Ref>(previous());
    if (check(LEFT_BRACE)) return string_set();
    throw Perror(peek(), "Expect a set name or '{'.");
}

// STRING | "#" | "~"
token parser::symbol() {
    if (match({STRING, BLANK, BLIP})) return previous();
    throw Perror(peek(), "Expect a symbol (a string, '#' or '~').");
}

// "->" | "<-" | "<>"
token parser::direction() {
    if (match({RIGHT_ARROW, LEFT_ARROW, STAY})) return previous();
    throw Perror(peek(), "Expect a move ('->', '<-' or '<>').");
}

// "halt" | "halt_yes" | "halt_no"
token parser::halt_state() {
    if (match({HALT, HALT_YES, HALT_NO})) return previous();
    throw Perror(peek(), "Expect a halt state ('halt', 'halt_yes' or 'halt_no').");
}

// STRING | halt_state
token parser::next_state() {
    if (match({STRING})) return previous();
    if (check(HALT) || check(HALT_YES) || check(HALT_NO)) return halt_state();
    throw Perror(peek(), "Expect a next state (a string or a halt state).");
}


bool parser::is_declaration_start() {
    switch (peek().type) {
        case ALPHABET:
        case STATE_SET:
        case TRANS_FUN:
        case TM:
        case CTM:
        case TAPE:
        case RUN:
            return true;
        default:
            return false;
    }
}

void parser::synchronize() {
    // don't skip a keyword we are already on, e.g. after a missing ';'
    if (!is_declaration_start()) advance();
    while (!isAtEnd()) {
        if (previous().type == SEMICOLON) return;
        if (is_declaration_start()) return;
        advance();
    }
}


//declerations

std::unique_ptr<stmt> parser::alphabet_declaration() {
    token name = consume(IDENTIFIER, "Expect alphabet name.");
    consume(EQUAL, "Expect '=' after alphabet name.");
    std::unique_ptr<expr> symbols = string_set();
    consume(SEMICOLON, "Expect ';' after alphabet declaration.");
    return std::make_unique<Alphabet_def>(name, std::move(symbols));
}

// set_or_ref ( ( "\/" | "\" ) set_or_ref )*
std::unique_ptr<expr> parser::state_set_expr() {
    std::unique_ptr<expr> e = set_or_ref();
    while (match({UNION, DIFFERENCE})) {
        token op = previous();
        std::unique_ptr<expr> right = set_or_ref();
        e = std::make_unique<Set_Binary>(std::move(e), op, std::move(right));
    }
    return e;
}

std::unique_ptr<stmt> parser::state_set_declaration() {
    token name = consume(IDENTIFIER, "Expect state set name.");
    consume(EQUAL, "Expect '=' after state set name.");
    std::unique_ptr<expr> states = state_set_expr();
    consume(SEMICOLON, "Expect ';' after state set declaration.");
    return std::make_unique<State_set_def>(name, std::move(states));
}

// "(" STRING "," symbol ")" "|--" "(" next_state "," symbol "," move ")"
Transition parser::transition() {
    consume(LEFT_PAREN, "Expect '(' to start a transition.");
    token state = consume(STRING, "Expect a state name.");
    consume(COMMA, "Expect ',' after state.");
    token read = symbol();
    consume(RIGHT_PAREN, "Expect ')' after (state, symbol).");
    consume(COMPUTES, "Expect '|--' after (state, symbol).");
    consume(LEFT_PAREN, "Expect '(' after '|--'.");
    token next = next_state();
    consume(COMMA, "Expect ',' after next state.");
    token write = symbol();
    consume(COMMA, "Expect ',' after written symbol.");
    token move = direction();
    consume(RIGHT_PAREN, "Expect ')' after move.");
    return Transition{state, read, next, write, move};
}

std::unique_ptr<stmt> parser::transition_function_declaration() {
    token name = consume(IDENTIFIER, "Expect transition function name.");
    consume(LEFT_PAREN, "Expect '(' after transition function name.");
    std::unique_ptr<expr> states = set_or_ref();
    consume(COMMA, "Expect ',' after state set.");
    std::unique_ptr<expr> alphabet = set_or_ref();
    consume(RIGHT_PAREN, "Expect ')' after alphabet.");
    consume(EQUAL, "Expect '=' after ')'.");
    consume(LEFT_BRACE, "Expect '{' before transitions.");
    std::vector<Transition> transitions;
    do {
        transitions.push_back(transition());
    } while (match({COMMA}));
    consume(RIGHT_BRACE, "Expect '}' after transitions.");
    consume(SEMICOLON, "Expect ';' after transition function declaration.");
    return std::make_unique<Transition_fun>(name, std::move(states), std::move(alphabet), std::move(transitions));
}


//tm M1(Q1, numbers, "C", d1, {halt});               // computes
//tm M2(Q2, colors,  "x", d2, {halt_yes, halt_no});  // decides
// tm, Identifier, "(", state_set_expr, ",", set_or_ref, ",", STRING, ",", transition_function, ",", "{", (halt_state ("," halt_state)*), "}", ";"

// "{" "halt" "}" | "{" "halt_yes" "," "halt_no" "}"
Final_state parser::final_state() {
    consume(LEFT_BRACE, "Expect '{' before final states.");
    std::vector<token> halts;
    if (match({HALT})) {
        halts.push_back(previous());
    } else if (match({HALT_YES})) {
        halts.push_back(previous());
        consume(COMMA, "Expect ',' after 'halt_yes'.");
        halts.push_back(consume(HALT_NO, "Expect 'halt_no' after 'halt_yes,'."));
    } else {
        throw Perror(peek(), "Expect 'halt' or 'halt_yes, halt_no'.");
    }
    consume(RIGHT_BRACE, "Expect '}' after final states.");
    return Final_state{std::move(halts)};
}

std::unique_ptr<stmt> parser::tm_declaration(){
    token name = consume(IDENTIFIER, "Expect Turing machine name.");
    consume(LEFT_PAREN, "Expect '(' after Turing machine name.");
    std::unique_ptr<expr> states = set_or_ref();
    consume(COMMA, "Expect ',' after state set.");
    std::unique_ptr<expr> alphabet = set_or_ref();
    consume(COMMA, "Expect ',' after alphabet.");
    token start = consume(STRING, "Expect a start state.");
    consume(COMMA, "Expect ',' after start state.");
    token func = consume(IDENTIFIER, "Expect a transition function name.");
    consume(COMMA, "Expect ',' after transition function name.");
    Final_state final_states = final_state();
    consume(RIGHT_PAREN, "Expect ')' after Turing machine declaration.");
    consume(SEMICOLON, "Expect ';' after Turing machine declaration.");
    return std::make_unique<Turing_machine>(name, std::move(states), std::move(alphabet), start, func, final_states);
}


std::unique_ptr<stmt> parser::declaration() {
    try {
        if (match({ALPHABET})) return alphabet_declaration();
        if (match({STATE_SET})) return state_set_declaration();
        if (match({TRANS_FUN})) return transition_function_declaration();
        if (match({TM})) return tm_declaration();
        if (match({CTM})) return ctm_declaration();
        if (match({TAPE})) return tape_declaration();
        if (match({RUN})) return run_statement();

        throw Perror(peek(), "Expected a declaration.");
    } catch (const ParseError&) {
        synchronize();
        return nullptr;
    }
}

std::vector<std::unique_ptr<stmt>> parser::parse() {
    std::vector<std::unique_ptr<stmt>> statements;
    while (!isAtEnd()) {
        std::unique_ptr<stmt> s = declaration();
        if (s) statements.push_back(std::move(s));
    }
    return statements;
}

