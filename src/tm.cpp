

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
static bool hadError = false;

void report(int line, string& where , string& message){
    cerr << "[line " << line << "] Error" << where << ": " << message << '\n';
    had_error = true;
}

static bool hadError = false;
void error(int line, string& message){
    report(line, "", message);
}

void run(string& source){
    Scanner scanner(source);
    vector<Token> tokens = scanner.scan_tokens();
    for(Token t : tokens){
        cout << t;
    }
}

void run_file(const string& path) {
    string line;
    ifstream input_file(path);
    if (input_file.is_open()){
       while(getline(input_file, line)){
            run(line);
            if(had_error) exit(65);
       } 
       input_file.close();
        }
        else {cerr<< "Cannot open path:" << path; return;}
  }



void run_prompt() {cout <<"run_prompt";}

int main(int argc, char *argv[]) {
    cout << argc; 
    if(argc > 2) { cout << "Usage: tm [script]\n";}
    else if (argc == 2 ) { run_file(argv[1]);}
    else { run_prompt();}
        
return 0;
}
