#ifndef ERROR_H
#define ERROR_H
#include <string>

extern bool had_error;

void report(int line, const std::string& where , const std::string& message);
void error(int line, const std::string& message);

#endif
