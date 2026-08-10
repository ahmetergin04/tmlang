#ifndef TOKEN_H
#define TOKEN_H

#include <string>


enum token_type {
    // Keywords
    ALPHABET, STATE_SET, TRANS_FUN, TM, CTM, TAPE, RUN, HEAD, IN,
    HALT_YES, HALT_NO, HALT, 
    // Single or two character tokens
    UNDERSCORE, LEFT_ARROW, RIGHT_ARROW, BLANK, EQUAL, LEFT_BRACE, RIGHT_BRACE,
    LEFT_PAREN, RIGHT_PAREN, COMPUTES, COMMA, DOT, EQUAL_EQUAL, BANG_EQUAL,
    UNION, START, DIFFERENCE, SEMICOLON, BLIP, KLEENE_CLOSURE, NOT, BACK_SLASH,
    BANG, AND, OR,
    // Literals
    IDENTIFIER, STRING, NUMBER,
    ENDOFFILE
};
// TODO Check this struct
struct liter {
    bool boolean;
    double val;
    std::string str;
    liter() = default;
    liter(bool b) : boolean(b) {}
    liter(double d) : val(d) {}
    liter(std::string& s) : str(s) {}
};

class token {
    public:
        token_type type;
        std::string lexeme;
        liter lit;
        int line;
        token(token_type type, const std::string& lexeme,const liter& lit, int line);
        std::string to_string() const;
        char advance();
};
#endif 
