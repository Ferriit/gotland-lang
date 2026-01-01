#include <iostream>
#include <cstdio>
#include <string>

#define string std::string


string readFile(char* filename) {
    FILE* f = fopen(filename, "r");

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    if (filesize == -1L) {
        perror("ftell failed");
        fclose(f);
        return "";
    }

    fseek(f, 0, SEEK_SET);

    char buf[filesize + 1];
    int ch;
    int i = 0;
    while ((ch = fgetc(f)) != EOF) {
        putchar(ch);
        buf[i] = ch;
        i++;
    }

    buf[i] = '\0';

    fclose(f);
    return string(buf);
}


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "Too few parameters, file name is required" << std::endl;
        return 1;
    }

    string buf = readFile(argv[1]);

    std::cout << buf << std::endl;

    return 0;
}

