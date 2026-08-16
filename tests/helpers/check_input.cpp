#include <iostream>
#include <string>

int main() {
    std::string input;
    // Will return 0 ONLY if it successfully receives and reads "hello"
    if (std::cin >> input && input == "hello") {
        return 0;
    }
    return 1;
}