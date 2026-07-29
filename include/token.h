#ifndef TOKEN_H
#define TOKEN_H

#include <string>


enum token_type {
    // Keywords
    ALPHABET, STATE_SET, TRANS_FUN, TM, CTM, TAPE, RUN, HEAD, NOT, IN, AND, OR,
    H_YES, H_NO, 
    // Single or two character tokens
    UNDERSCORE, LEFT_ARROW, RIGHT_ARROW, BLANK, EQUAL, LEFT_BRACE, RIGHT_BRACE,
    LEFT_PAREN, RIGHT_PAREN, COMPUTATION, COMMA, DOT, EQUAL_EQUAL, BANG_EQUAL,
    UNION, START, BACK_SLASH, SLASH, HALT, SEMICOLON, BLIP,
    // Literals
    IDENTIFIER, STRING,
    ENDOFFILE
};
// TODO Check this struct 
struct liter {
    bool boolean;
    double val;
    std::string str;
};

class token {
    public:
        token_type type;
        std::string lexeme;
        liter lit;
        int line;
        token(token_type type, const std::string& lexeme,const liter& lit, int line);
        std::string to_string() const;
};
#endif 
