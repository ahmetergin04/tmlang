#include "tm_error.h"
#include <iostream>

bool had_error = false;
bool had_runtime_error = false;

static void print_error(int line, const std::string_view where, const std::string_view message){
    std::cerr << "[line " << line << "] Error " << where << ": " << message << '\n';
}

// scan/parse errors: the program is not run (exit 65)
void report(int line, const std::string_view where , const std::string_view message){
    print_error(line, where, message);
    had_error = true;
}

// errors while interpreting (exit 70)
void runtime_error_report(int line, const std::string_view where, const std::string_view message){
    print_error(line, where, message);
    had_runtime_error = true;
}

void error(int line, const std::string_view message){
    report(line, "", message);
}
