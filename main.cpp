#include <cmath>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>

#define string std::string
#define vector std::vector


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
    string out = "";

    vector<string> lines = splitlines(data);

    for (string s : lines) {
        bool outsidestring = true;
        char string_delim = 0; // tracks which quote we're inside, 0 if none

        for (int j = 0; j < s.size(); j++) {
            char c = s[j];

            // Check if entering or exiting a string
            if (c == '"' || c == '\'') {
                if (outsidestring) {
                    outsidestring = false;
                    string_delim = c;
                } else if (c == string_delim) {
                    outsidestring = true;
                    string_delim = 0;
                }
                // Always append the quote character
                out.push_back(c);
                continue; // move to next character
            }

            // Original comment-handling logic
            if (j != 0) {
                string current;
                string previous;

                current.push_back(c);
                previous.push_back(s[j - 1]);

                if ((previous + current) != "//" && outsidestring) {
                    out.push_back(c);
                } else {
                    out.pop_back(); // safe because j != 0
                    break;
                }
            } else {
                out.push_back(c);
            }
        }
        out.push_back('\n');
    }

    return out;
}


vector<string> splitdata(string data) {
    vector<string> split = {""};
    int j = 0;
    for (char i: data) {
        if (i == ' ' || i == '\n' || i == '\t') {
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


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "Too few parameters, file name is required" << std::endl;
        return 1;
    }

    string buf = removecomments(readfile(argv[1]));
    
    vector<string> splits = splitdata(buf);

    for (string s: splits) {
        std::cout << s << std::endl;
    }

    std::cout << buf << std::endl;

    return 0;
}

