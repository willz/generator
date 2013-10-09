#include "generator.h"
#include <iostream>
#include <string>
#include <cstring>

class IntRange : public GeneratorCore<int> {
public:
    void generate(int start, int end, int interval = 1) {
        for (int i = start; i < end; ++i) {
            yield(i);
        }
    }
};

class Tokenizer : public GeneratorCore<std::string> {
public:
    void generate(const std::string& str) {
        char* copy = strdup(str.c_str());
        char* p = nullptr;
        p = strtok(copy, " \t\n,");
        while (p != nullptr) {
            yield(std::string(p));
            p = strtok(NULL, " \t\n,");
        }
    }
};

int main(void) {
    for (auto i : Generator<IntRange>(1, 10)) {
        std::cout << i << std::endl;
    }

    std::string text = "Wow, a plain text";
    for (auto i : Generator<Tokenizer>(text)) {
        std::cout << i << std::endl;
    }
    return 0;
}
