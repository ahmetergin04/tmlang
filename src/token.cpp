#include "token.h"

token::token(token_type type,const std::string& lexeme, const liter& lit, int line): type(type), lexeme(lexeme), lit(lit), line(line) {}

std::string token::to_string() const {
    std::string s = "";
    switch(this->type ) {
        case LEFT_PAREN : s = "LEFT_PAREN"; break;
        case RIGHT_PAREN : s = "RIGHT_PAREN"; break;
        case ALPHABET : s = "ALPHABET"; break;
        case STATE_SET : s = "STATE_SET"; break;
        case TRANS_FUN : s = "TRANS_FUN"; break;
        case TM : s = "TM"; break;
        case CTM : s = "CTM"; break;
        case TAPE : s = "TAPE"; break;
        case RUN : s = "RUN"; break;
        case HEAD : s = "HEAD"; break;
        case NOT : s = "NOT"; break;
        case IN : s = "IN"; break;
        case AND : s = "AND"; break;
        case OR : s = "OR"; break;
        case HALT_YES : s = "HALT_YES"; break;
        case HALT_NO : s = "HALT_NO"; break;
        case UNDERSCORE : s = "UNDERSCORE"; break;
        case LEFT_ARROW : s = "LEFT_ARROW"; break;
        case RIGHT_ARROW : s = "RIGHT_ARROW"; break;
        case COMPUTES : s = "COMPUTES"; break;
        case BLANK : s = "BLANK"; break;
        case EQUAL : s = "EQUAL"; break;
        case LEFT_BRACE : s = "LEFT_BRACE"; break;
        case RIGHT_BRACE : s = "RIGHT_BRACE"; break;
        case COMMA : s = "COMMA"; break;
        case DOT : s = "DOT"; break;
        case EQUAL_EQUAL : s = "EQUAL_EQUAL"; break;
        case BANG_EQUAL : s = "BANG_EQUAL"; break;
        case UNION : s = "UNION"; break;
        case START : s = "START"; break;
        case BACK_SLASH : s = "BACK_SLASH"; break;
        case HALT : s = "HALT"; break;
        case SEMICOLON : s = "SEMICOLON"; break;
        case BLIP : s = "BLIP"; break;
        case ENDOFFILE : s = "ENDOFFILE"; break;
        case IDENTIFIER : s = "IDENTIFIER(" + this->lexeme+ ")";break;
        case STRING : s = "STRING(" + this->lexeme+ ")"; break;
    }
    s = s + "\n"; 
    return s;
}
