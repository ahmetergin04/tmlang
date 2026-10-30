#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

#include "error.h"
#include "token.h"
#include "expr.h"
#include "stmt.h"

class parser{
private:
    std::vector<token> tokens;
    int current = 0;

    struct ParseError : public std::runtime_error{
        using std::runtime_error::runtime_error;
    }; 

    ParseError Perror(token token, std::string message){
        report(token.line, token.type == ENDOFFILE ? " at end" : " at '" + token.lexeme + "'", message);
  
        return ParseError(message);
    }

    bool isAtEnd();
    const token& peek();
    const token& previous();
    const token& advance();
    bool check(token_type type);
    bool match(std::initializer_list<token_type> types);
    const token& consume(token_type type, const std::string& message);

    bool is_declaration_start();
    void synchronize();
    std::unique_ptr<stmt> declaration();

    
    std::unique_ptr<expr> string_set();
    std::unique_ptr<expr> set_or_ref();
    token symbol();
    token direction(); 
    token halt_state();
    token next_state();
public:
    explicit parser(std::vector<token> tokens) : tokens(std::move(tokens)) {}
    std::vector<std::unique_ptr<stmt>> parse();

};

#endif 
