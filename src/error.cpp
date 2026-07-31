#include "error.h"
#include <iostream>

bool had_error = false;

void report(int line, const std::string& where , const std::string& message){
    std::cerr << "[line " << line << "] Error " << where << ": " << message << '\n';
    had_error = true;
}

void error(int line, const std::string& message){
    report(line, "", message);
}

