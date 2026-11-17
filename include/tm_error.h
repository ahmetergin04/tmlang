#ifndef ERROR_H
#define ERROR_H
#include <string>
#include <string_view>

extern bool had_error;
extern bool had_runtime_error;

void report(int line, const std::string_view where , const std::string_view message);
void runtime_error_report(int line, const std::string_view where, const std::string_view message);
void error(int line, const std::string_view message);

#endif
