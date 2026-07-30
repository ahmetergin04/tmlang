#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "scanner.h"
#include "error.h"

using namespace std;

void run(string& source){
    Scanner scanner(source);
    vector<Token> tokens = scanner.scan_tokens();
    for(const Token& t : tokens){
        cout << t;
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

void run_prompt() {cout <<"run_prompt";}

int main(int argc, char *argv[]) {
    cout << argc; 
    if(argc > 2) { cout << "Usage: tm [script]\n";}
    else if (argc == 2 ) { run_file(argv[1]);}
    else { run_prompt();}
return 0;
}
