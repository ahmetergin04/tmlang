#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

#include "error.h"
#include "token.h"

class parser{
private:
    
    struct ParseError : public std::runtime_error{
        using std::runtime_error::runtime_error;
    }; 

    ParseError Perror(token token, std::string message){
        error(token.line, message);
        return ParseError(message);
    }

    bool isAtEnd();
    const token& peek();
    const token& previous();
    const token& advance();
    bool check(token_type type);
    bool match(std::initializer_list<token_type> types);
    const token& consume(token_type type, const std::string& message);
 public:
    std::vector<token> tokens;
    int current = 0;
};

#endif 
