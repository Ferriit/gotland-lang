#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_set>
#include <vector>
#include <cstring>

#define string std::string
#define vector std::vector

enum class tokentype {
    Identifier,
    Keyword,
    Label,
    Constant,
    Block,
    Operator,
    Endoffile
};


struct token {
    int line;
    int col;

    tokentype type;
    string text;
};


enum class ASTNodeType {
    Program,
    Block,
    VarDecl,
    Function,
    Return,
    Identifier,
    Constant,
    BinaryExpr
};


struct ASTNode {
    ASTNodeType type;
    vector<ASTNode*> children;
    token tok;   // token that produced this node (for errors)
};


struct Parser {
    const vector<token>& tokens;
    size_t pos = 0;

    token peek() { return tokens[pos]; }
    token advance() { return tokens[pos++]; }
    bool match(tokentype t);
};


struct coderesult {
    bool status;
    string code;
};


string readfile(char* filename) {
    FILE* f = fopen(filename, "r");

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    if (filesize == -1L) {
        perror("ftell failed");
        fclose(f);
        return "";
    }

    fseek(f, 0, SEEK_SET);

    string buf;
    buf.resize(filesize);

    fread(buf.data(), 1, filesize, f);

    fclose(f);
    return string(buf);
}

vector<string> splitlines(string data) {
    vector<string> split = {""};
    int j = 0;
    for (char i: data) {
        if (i == '\n') {
            split.push_back("");
            j++;
        }
        else {
            split[j] += i;
        }
    }

    // Clean output
    vector<string> out = {};
    for (string s: split) {
        if (s != "") {
            out.push_back(s);
        }
    }

    return out;
}

string removecomments(string data) {
    string out;

    vector<string> lines = splitlines(data);

    for (string& s : lines) {
        bool outsidestring = true;
        char string_delim = 0; // Tracks which quote we're inside, 0 if none

        size_t j = 0;
        while (j < s.size()) {
            char c = s[j];

            // Enter or exit a string
            if (c == '"' || c == '\'') {
                if (outsidestring) {
                    outsidestring = false;
                    string_delim = c;
                } else if (c == string_delim) {
                    outsidestring = true;
                    string_delim = 0;
                }

                out += c;
                j++;
                continue;
            }

            // Check for comment start only if outside string
            if (outsidestring && c == '/' && j + 1 < s.size() && s[j + 1] == '/') {
                // Skip rest of line
                break;
            }

            out += c;
            j++;
        }

        out += '\n';
    }

    return out;
}


vector<string> splitdata(string data) {
    vector<string> split = {""};
    int j = 0;
    
    bool invar = false;
    char vartype = '"';

    for (char i: data) {
        if (i == '"' && !invar) {
            // Enter string
            vartype = '"';
            split[j] += i;
            invar = true;
        }

        else if ((i == '"' && vartype == '"')) {
            // Exit string
            vartype = '"';
            split[j] += i;
            invar = false;
        }

        else if (i == '[' && !invar) {
            // Enter array
            vartype = '[';
            split[j] += i;
            invar = true;
        }

        else if ((i == ']' && vartype == '[')) {
            // Exit array
            vartype = '[';
            split[j] += i;
            invar = false;
        }


        else if ((i == ' ' || i == '\n' || i == '\t') && !invar) {
            split.push_back("");
            j++;
        }
        else {
            split[j] += i;
        }
        
        //std::cout << "Char: " << i << " Invar: " << invar << " Vartype: " << vartype << std::endl;
    }

    // Clean output
    vector<string> out = {};
    for (string s: split) {
        if (s != "") {
            out.push_back(s);
        }
    }

    return out;
}

bool containschar(const char* s, char c) {
    return std::strchr(s, c) != nullptr;
}

bool isvalue(const string& identifier) {
    if (identifier == "true" || identifier == "false") return true;
    if (identifier.empty()) return false;

    char first = identifier[0];
    char last = identifier.back();

    // Detect type
    int type = 0; // int
    if (first == '\'') type = 1;     // char
    else if (last == 'f') type = 2;  // float

    const char* numbers = "0123456789";

    switch (type) {
        case 0: // int
            for (char c : identifier) {
                if (!containschar(numbers, c)) return false;
            }
            break;

        case 1: // char
            if (identifier.size() != 3 || identifier[2] != '\'') return false;
            break;

        case 2: // float
            bool dot_seen = false;
            for (size_t i = 0; i < identifier.size() - 1; i++) {
                char c = identifier[i];
                if (c == '.') {
                    if (dot_seen) return false;
                    dot_seen = true;
                } else if (!containschar(numbers, c)) {
                    return false;
                }
            }
            if (last != 'f') return false;
            break;
    }
    return true;
}


vector<token> tokenizer(vector<string> splits) {
    const std::unordered_set<string> operators = {
        "+", "-", "/", "*", "%", "=", "==", "!", 
        "||", "&&", "|", "&", "^", "!=", "<", 
        ">", "<=", ">="
    };

    const std::unordered_set<string> keywords = {
        "imp", "glb", "loc", "var", "fn", "ret", "begin", 
        "end", "if", "jmp", "true", "false"
    };

    const std::unordered_set<string> blocks = {
        "[using]", "[data]", "[impl]"
    };

    vector<token> tokens;

    for (const string& identifier : splits) {
        if (identifier.empty()) continue; // <--- skip empty strings

        if (operators.find(identifier) != operators.end()) {
            tokens.push_back({0, 0, tokentype::Operator, identifier});
        }
        else if (keywords.find(identifier) != keywords.end()) {
            tokens.push_back({0, 0, tokentype::Keyword, identifier});
        }
        else if (blocks.find(identifier) != blocks.end()) {
            tokens.push_back({0, 0, tokentype::Block, identifier});
        }
        else if (isvalue(identifier)) {
            tokens.push_back({0, 0, tokentype::Constant, identifier});
        }
        else if (!identifier.empty() && identifier.back() == ':') {
            tokens.push_back({0, 0, tokentype::Label, identifier});
        }
        else if (!identifier.empty() && identifier[0] == '"') {
            tokens.push_back({.line=0, .col=0, .type=tokentype::Constant, .text=identifier});
        }
        else if (!identifier.empty() && identifier[0] == '[') {
            tokens.push_back({.line=0, .col=0, .type=tokentype::Constant, .text=identifier});
        }
        else {
            tokens.push_back({.line=0, .col=0, .type=tokentype::Identifier, .text=identifier});
        }
    }

    // End-of-file token
    tokens.push_back({0, 0, tokentype::Endoffile, ""});

    return tokens;
}


bool lexer(vector<token> tokens) {
    string block = "";

    int scope = 0;
    
    bool failed = false;

    for (int i = 0; i < tokens.size(); i++) {
        token current = tokens[i];
        tokentype type = current.type;
    
        token next = token();
        if (i < (tokens.size() - 1)) {
            next = tokens[i + 1];
        }

        if (type == tokentype::Block) {
            block = current.text;
        }

        else if (type == tokentype::Keyword) {
            if (current.text == "begin") {              // Begin and end can only be in [impl]
                scope++;
                if (block != "[impl]") {
                    std::cout << "Block Error on line " << current.line << 
                        ":\n'begin' is in the wrong block, should be in [impl]\n\n";
                
                    failed = true;
                }
            }

            else if (current.text == "end") {
                scope--;

                if (scope < 0) {
                    std::cout << "Scope Error on line " << current.line << 
                        ":\nScope is invalid\n\n";
                    failed = true;
                }

                if (block != "[impl]") {
                    std::cout << "Block Error on line " << current.line << 
                        ":\n'end' is in the wrong block, should be in [impl]\n\n";
                
                    failed = true;
                }
            }


            else if (current.text == "imp") {
                if (next.type != tokentype::Keyword ||      // Next keyword must be glb or loc 
                        (next.text != "loc" && next.text != "glb")) {
                    std::cout << "Syntax Error on line " << current.line <<
                        ":\nUnknown token '" << next.text << "'. Expected 'loc' or 'glb'\n\n";

                    failed = true;
                }

                if (block != "[using]") {
                    std::cout << "Block Error on line " << current.line << 
                        ":\n'imp' is in the wrong block, should be in [using]\n\n";
                
                    failed = true;
                }
            }

            else if (current.text == "var") {
                if (next.type != tokentype::Identifier) {
                    std::cout << "Syntax Error on line " << current.line <<
                        ":\nExpected variable name, got '" << next.text << "'\n\n";
                    failed = true;
                }
                else if (block == "[using]") {
                    std::cout << "Block Error on line " << current.line <<
                        ":\n'var' is in the wrong block, should be in [data] or [impl]\n\n";
                    failed = true;
                }
            }   
            
            else if (current.text == "fn") {
                if (next.type != tokentype::Identifier) {
                    std::cout << "Syntax Error on line " << current.line <<
                        ":\nExpected function name, got '" << next.text << "'\n\n";
                    failed = true;
                }
                else if (block != "[impl]") {
                    std::cout << "Block Error on line " << current.line <<
                        ":\n'fn' is in the wrong block, should be in [impl]\n\n";
                    failed = true;
                }
            }

            else if (current.text == "glb") {
                if (next.type != tokentype::Identifier) {
                    std::cout << "Syntax Error on line" << current.line <<
                        ":\nExpected identifier, got '" << next.text << "'\n\n";
                        failed = true;
                }
                else if (block == "[data]") {
                    std::cout << "Block Error on line " << current.line <<
                        ":\n'glb' is in the wrong block, should be in [using] or [impl]\n\n";
                    failed = true;
                }
            }
            else if (current.text == "loc") {
                if (next.type != tokentype::Identifier) {
                    std::cout << "Syntax Error on line" << current.line <<
                        ":\nExpected identifier, got '" << next.text << "'\n\n";
                        failed = true;
                }
                else if (block != "[using]") {
                    std::cout << "Block Error on line " << current.line <<
                        ":\n'loc' is in the wrong block, should be in [using]\n\n";
                    failed = true;
                }
            }
        }
    }
    if (scope != 0) {
        std::cout << "Scope Error on line " << tokens[tokens.size() - 1].line <<
            ":\nScope is invalid\n\n";
    }

    if (failed) {
        std::cout << "Compilation failed." << std::endl;
        return false;
    }
    return true;
}


ASTNode parser(vector<token> tokens) {
    Parser parsedata = {.tokens=tokens};

    
}


coderesult codegen(string data) {
    string buf = removecomments(data);

    vector<string> splits = splitdata(buf);

    vector<token> tokens = tokenizer(splits);

    bool lexresult = lexer(tokens);

    if (!lexresult) {
        return coderesult({.status=false, .code=""});
    }

    ASTNode tree = parser(tokens);
    
}


string tokentypetostring(tokentype type) {
    switch (type) {
        case tokentype::Identifier: return "Identifier";
        case tokentype::Keyword:    return "Keyword";
        case tokentype::Label:      return "Label";
        case tokentype::Constant:   return "Constant";
        case tokentype::Operator:   return "Operator";
        case tokentype::Block:      return "Block";
        case tokentype::Endoffile:  return "EndOfFile";
        default: return "Unknown";
    }
}

void printtokens(const vector<token>& tokens) { 
    for (const auto& t : tokens) {
        std::cout << "[" << t.line << ":" << t.col << "] "
                  << tokentypetostring(t.type)
                  << " -> \"" << t.text << "\"\n";
    }
}


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "Too few parameters, file name is required" << std::endl;
        return 1;
    }

    // Remove Comments (Removecomments) -> Format (Splitdata) -> Generate Tokens (Tokenizer) -> Analyze Syntax (Lexer) -> Generate AST (Parser) -> Codegen

    string buf = removecomments(readfile(argv[1]));

    vector<string> splits = splitdata(buf);

    vector<token> tokens = tokenizer(splits);

    printtokens(tokens);

    return lexer(tokens);
}

