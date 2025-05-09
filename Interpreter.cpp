#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Exception class for syntax errors
class SyntaxError : public runtime_error {
public:
    SyntaxError(const string& message) : runtime_error("error") {}
};

// Exception class for uninitialized variable access
class UninitializedError : public runtime_error {
public:
    UninitializedError(const string& message) : runtime_error("error") {}
};

// Token types
enum TokenType { ID, NUM, OP, ASSIGN, SEMICOLON, LPAREN, RPAREN, END };

// Token structure
struct Token {
    TokenType type;
    string value;
};

// Tokenizer
class Lexer {
    string input;
    size_t pos;
public:
    Lexer(const string& input) : input(input), pos(0) {}

    Token nextToken() {
        while (pos < input.size() && isspace(input[pos])) pos++;
        if (pos >= input.size()) return {END, ""};

        char ch = input[pos];

        if (isalpha(ch) || ch == '_') {
            string id;
            while (pos < input.size() && (isalnum(input[pos]) || input[pos] == '_'))
                id += input[pos++];
            return {ID, id};
        }

        if (isdigit(ch)) {
            if (ch == '0' && isdigit(input[pos + 1]))
                throw SyntaxError("Leading zeros not allowed");
            string num;
            if (ch == '0') {
                num = "0";
                pos++;
            } else {
                while (pos < input.size() && isdigit(input[pos]))
                    num += input[pos++];
            }
            return {NUM, num};
        }

        if (ch == '+' || ch == '-' || ch == '*') {
            pos++;
            return {OP, string(1, ch)};
        }

        if (ch == '=') {
            pos++;
            return {ASSIGN, "="};
        }

        if (ch == ';') {
            pos++;
            return {SEMICOLON, ";"};
        }

        if (ch == '(') {
            pos++;
            return {LPAREN, "("};
        }

        if (ch == ')') {
            pos++;
            return {RPAREN, ")"};
        }

        throw SyntaxError("Invalid character");
    }
};

// Parser and Evaluator
class Interpreter {
    Lexer lexer;
    Token current;
    unordered_map<string, int> vars;
    unordered_map<string, bool> initialized;

    void next() {
        current = lexer.nextToken();
    }

    void expect(TokenType expected) {
        if (current.type != expected)
            throw SyntaxError("Expected different token");
    }

    int parseFact() {
        if (current.type == OP && (current.value == "+" || current.value == "-")) {
            string op = current.value;
            next();
            int value = parseFact();
            return (op == "+") ? value : -value;
        } else if (current.type == NUM) {
            int value = stoi(current.value);
            next();
            return value;
        } else if (current.type == ID) {
            string varName = current.value;
            if (!initialized[varName]) throw UninitializedError("Use of uninitialized variable");
            int value = vars[varName];
            next();
            return value;
        } else if (current.type == LPAREN) {
            next();
            int value = parseExp();
            expect(RPAREN);
            next();
            return value;
        } else {
            throw SyntaxError("Invalid factor");
        }
    }

    int parseTerm() {
        int result = parseFact();
        while (current.type == OP && current.value == "*") {
            next();
            result *= parseFact();
        }
        return result;
    }

    int parseExp() {
        int result = parseTerm();
        while (current.type == OP && (current.value == "+" || current.value == "-")) {
            string op = current.value;
            next();
            int term = parseTerm();
            result = (op == "+") ? result + term : result - term;
        }
        return result;
    }

    void parseAssignment() {
        expect(ID);
        string varName = current.value;
        next();
        expect(ASSIGN);
        next();
        int value = parseExp();
        expect(SEMICOLON);
        next();
        vars[varName] = value;
        initialized[varName] = true;
    }

public:
    Interpreter(const string& input) : lexer(input) {}

    void execute() {
        try {
            next();
            while (current.type != END) {
                parseAssignment();
            }
            for (const auto& pair : vars) {
                cout << pair.first << " = " << pair.second << endl;
            }
        } catch (runtime_error& e) {
            cout << e.what() << endl;
        }
    }
};

// Helper to join multiple lines of input into one string
string readFullInput() {
    string line, fullInput;
    while (getline(cin, line)) {
        if (line.empty()) break;
        fullInput += line + " ";
    }
    return fullInput;
}

int main() {
    string input = readFullInput();
    Interpreter interpreter(input);
    interpreter.execute();
    return 0;
}
