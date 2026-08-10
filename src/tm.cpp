#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "scanner.h"
#include "error.h"
#include "token.h"

using namespace std;

void run(string& source){
    scanner  scanner(source);
    vector<token> tokens = scanner.scan_tokens();
    for(const token& t : tokens){
        cout << t.to_string();
    }
}

void run_file(const string& path) {
    string line;
    ifstream input_file(path);
    if (!input_file.is_open()){
        cerr<< "Cannot open path: " << path << "\n"; return; }
    stringstream ss;
    ss  << input_file.rdbuf();
    string source = ss.str();
    run(source);
    if(had_error) exit(65);
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
    if(argc > 2) { cout << "Usage: tm [script]\n";}
    else if (argc == 2 ) { run_file(argv[1]);}
    else { run_prompt();}
return 0;
}
