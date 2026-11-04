#ifndef ERROR_H
#define ERROR_H
#include <string>
#include <string_view>

extern bool had_error;

void report(int line, const std::string_view where , const std::string_view message);
void error(int line, const std::string_view message);

#endif
