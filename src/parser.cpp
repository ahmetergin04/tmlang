
#include "parser.h"
#include "token.h"

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
    
    throw error;
}

bool parser::isAtEnd() { 
    return peek().type == ENDOFFILE; 
}