
#include <vector>
#include "scanner.h"
#include "error.h"

scanner::scanner(std::string& source): source(source) {}

const  std::unordered_map<std::string, token_type> scanner::keywords = {
            {"alphabet", token_type::ALPHABET},
            {"state_set", token_type::STATE_SET},
            {"transition_func", token_type::TRANS_FUN},
            {"tm", token_type::TM},
            {"ctm", token_type::CTM},
            {"tape", token_type::TAPE},
            {"run", token_type::RUN},
            {"head", token_type::HEAD},
            {"in", token_type::IN},
            {"halt_yes", token_type::HALT_YES},
            {"halt_no", token_type::HALT_NO},
            {"halt", token_type::HALT}
        };

std::vector<token> scanner::scan_tokens() {
   while(!this->at_end()) {
    this->start = this->current;
    this->scan_token();
   }
   this->tokens.emplace_back(ENDOFFILE, "", liter{},this->line);
    return this->tokens;
}

bool scanner::at_end() const { return this->current >= this->source.length(); }

void scanner::scan_token() {
    char c = this->advance();
    switch(c) {
        case '(': add_token(LEFT_PAREN); break;
        case ')': add_token(RIGHT_PAREN); break;
        case '{': add_token(LEFT_BRACE); break;
        case '}': add_token(RIGHT_BRACE); break;
        case ',': add_token(COMMA); break;
        case '.': add_token(DOT); break;
        case '_': add_token(UNDERSCORE); break;
        case '#': add_token(BLANK); break;
        case '>': add_token(START); break;
        case ';': add_token(SEMICOLON); break;
        case '*': add_token(KLEENE_CLOSURE); break;
        case '~': add_token(BLIP); break;
        case '!': add_token(match('=') ? BANG_EQUAL : BANG); break;
        case '=': add_token(match('=') ? EQUAL_EQUAL : EQUAL); break;
        case '|': add_token(match('-') ? COMPUTES : OR); break;
        case '/':  if(match('/')) {
                        while(this->peek() != '\n' && ! this->at_end()) 
                           {this->advance();} break;}
// TODO: /**/ multi-line comment else if(match('*')) 
        case '&': add_token(AND); break;
        case '\\': add_token(match('/') ? UNION : DIFFERENCE); break;
                           case '-': if(match('>')) { add_token(RIGHT_ARROW); break;}
                           case '<': if(match('-')) { add_token(LEFT_ARROW); break;}
        case ' ': 
        case '\r': 
        case '\t':
                    break;
        case '\n':  this->line++; break;
        case '"': this-> scstring(); break;
        default:
            if(is_alpha(c)) {
                this->identifier();
                }
                else{
             std::string temp = "Unexpected character: "+c;
            error(this-> line, temp ); 

            }
            break;
    }
}

char scanner::advance() { return this->source[current++];}

void scanner::add_token(token_type type) { add_token(type, liter{});}

void scanner::add_token(token_type type, liter lit) { 
    std::string text = this-> source.substr(this->start, this->current - this->start);
    this->tokens.emplace_back(type, text, lit, this->line);
}
bool scanner::match(const char expected) {
    if(this->at_end()) return false;
    if(this->source[current] != expected) return false;
    current++;
    return true; 
}
char scanner::peek() {
    if(this->at_end()) return '\0';
    return this->source[this->current];
}
void scanner::scstring() {
    while(this->peek() != '"' && !this->at_end()) {
        if(this->peek() ==  '\n') this->line++;
        this->advance();
    }
    if(this->at_end()) {
        error(this->line, "Unterminated string.");
        return; 
    }
    this->advance();
    std::string value = this->source.substr(this->start+1, this->current - this->start -2);
    add_token(STRING, value);
}
bool scanner::is_digit(const char c) const { return c >= '0' && c<= '9'; }
/*  TODO: consider adding NUMBER to the grammar
void scanner::numbr() { 
    while( is_digit( peek() )  advance();
    if (peek() == '.'  && is_digit(peek_next()) ) {
        advance();
    while( is_digit( peek() )  advance();
    }
    add_token(NUMBER, 
*/
bool scanner::is_alpha(const char c) const { 
    return (c >= 'a' && c<= 'z') || (c >= 'A' && c<= 'Z') || c == '_';
}
void scanner::identifier() { 
    while( is_alphanum( peek() ))  advance();
    std::string text  = this->source.substr(this->start, this->current- this->start);
    token_type type = IDENTIFIER;
    std::unordered_map<std::string, token_type>::const_iterator iter = this->keywords.find(text);
    if(iter != this->keywords.end())     type =iter->second;
    add_token(type);
    }

bool scanner::is_alphanum(const char c) const { 
        return  is_alpha(c) || is_digit(c) ;
        }

