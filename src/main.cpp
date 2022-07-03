using namespace std;
#include <iostream>
#include <cstring>
#include<vector>
#include "Tomasulo.hpp"
int main() {
    simulator cpu;
    try {
        cpu.init(std::cin);
        cpu.run();
    }
    catch (const char *s) {
        std::cerr << s << std::endl;
    }
}