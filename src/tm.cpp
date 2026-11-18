#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "scanner.h"
#include "tm_error.h"
#include "token.h"
#include "parser.h"
#include "ast_printer.h"
#include "environment.h"
#include "interpreter.h"

Environment global_env;

void run(std::string& source, const InterpreterOptions& options){
    scanner  scanner(source);
	std::vector<token> tokens = scanner.scan_tokens();
    parser parser(tokens);
	std::vector<std::unique_ptr<stmt>> statements = parser.parse();
    if(had_error) return;
		//   ast_printer printer;
		//   for(const auto& s: statements){
		// std::cout << printer.print(*s) << "\n";
		//   }
	Interpreter interpreter(global_env, options);
	interpreter.interpret(statements);
	// TODO: keep the AST alive for the REPL. CtmRow::condition is a raw expr* into
	// `statements`, which is freed when run() returns, so a CTM declared on one REPL
	// line and run on a later one uses a dangling pointer. Fix: move the statements
	// into a global std::vector<std::unique_ptr<stmt>> program after interpreting.
}

void run_file(const std::string& path, const InterpreterOptions& options) {
    std::string line;
	std::ifstream input_file(path);
    if (!input_file.is_open()){
	std::cerr<< "Cannot open path: " << path << "\n"; return; }
    std::stringstream ss;
    ss  << input_file.rdbuf();
    std::string source = ss.str();
    run(source, options);
    if(had_error) exit(65);
	if(had_runtime_error) exit(70);
}

void run_prompt(const InterpreterOptions& options) {
    for(;;){
        std::cout << "--> ";
        std::string prompt;
        if(std::getline(std::cin, prompt)){
            run(prompt, options);
        }
        else{break;}

        had_error = false;
        had_runtime_error = false;
    }
}

int main(int argc, char *argv[]) {
    InterpreterOptions options;
    std::string script_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-printMachineTrace") {
            options.print_machine_trace = true;
        }
        else if (arg[0] != '-' && script_path.empty()) {
            script_path = arg;
        }
        else {
            std::cout << "Usage: tm [-printMachineTrace] [script]\n";
            return 64;
        }
    }

    if (!script_path.empty()) { run_file(script_path, options); }
    else { run_prompt(options); }
    return 0;
}
