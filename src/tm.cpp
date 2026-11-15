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

void run(std::string& source){
    scanner  scanner(source);
	std::vector<token> tokens = scanner.scan_tokens();
    parser parser(tokens);
	std::vector<std::unique_ptr<stmt>> statements = parser.parse();
    if(had_error) return;
		//   ast_printer printer;
		//   for(const auto& s: statements){
		// std::cout << printer.print(*s) << "\n";
		//   }
	Interpreter interpreter(global_env);
	interpreter.interpret(statements);
}

void run_file(const std::string& path) {
    std::string line;
	std::ifstream input_file(path);
    if (!input_file.is_open()){
	std::cerr<< "Cannot open path: " << path << "\n"; return; }
    std::stringstream ss;
    ss  << input_file.rdbuf();
    std::string source = ss.str();
    run(source);
    if(had_error) exit(65);
	if(had_runtime_error) exit(70);
}

void run_prompt() {
    for(;;){
        std::cout << "--> ";
        std::string prompt;
        if(std::getline(std::cin, prompt)){
            run(prompt);
        }
        else{break;}

        had_error = false;
    }
}

int main(int argc, char *argv[]) {
    if(argc > 2) { std::cout << "Usage: tm [script]\n";}
    else if (argc == 2 ) { run_file(argv[1]);}
    else { run_prompt();}
return 0;
}
