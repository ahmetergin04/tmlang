
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

// IDENTIFIER | string_set
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

std::unique_ptr<stmt> parser::declaration() {
    try {
        if (match({ALPHABET})) return alphabet_declaration();
        if (match({STATE_SET})) return state_set_declaration();
        if (match({TRANS_FUN})) return transition_fun_declaration();
        if (match({TM})) return turing_machine_declaration();
        if (match({CTM})) return composite_tm_declaration();
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

