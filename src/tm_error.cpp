
#include "tm_error.h"
#include <iostream>

bool had_error = false;
void report(int line, const std::string_view where , const std::string_view message){
    std::cerr << "[line " << line << "] Error " << where << ": " << message << '\n';
    had_error = true;
}

void error(int line, const std::string_view message){
    report(line, "", message);
}