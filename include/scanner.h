#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "token.h"
class scanner {
    public:
        explicit scanner(std::string& source);
        std::vector <token> scan_tokens();
    private:
        std::string source;
        int start =0, current= 0,line = 1;
        std::vector<token> tokens;
        static const  std::unordered_map<std::string, token_type> keywords; 
        bool at_end() const ;
        void scan_token();
        char advance();
       void add_token(token_type type);
       void add_token(token_type type, liter lit);
       bool match(const char expected); 
       char peek();
      void scstring(); 
      bool is_digit(const char c) const;
      bool is_alpha(const char c) const;
      bool is_alphanum(const char c) const;
      void identifier(); 

};
#endif
