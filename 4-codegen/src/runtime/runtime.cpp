#include <cstdint>
#include <iostream>

// Micro-C standard library

extern "C" {

void print(std::int64_t i) { std::cout << i << "\n"; }

std::int64_t read() {
    std::int64_t i;
    std::cout << "> ";
    std::cin >> i;

    return i;
}

void print8(std::int64_t arg0, std::int64_t arg1, std::int64_t arg2,
            std::int64_t arg3, std::int64_t arg4, std::int64_t arg5,
            std::int64_t arg6, std::int64_t arg7) {
    std::cout << arg0 << " " << arg1 << " " << arg2 << " " << arg3 << " "
              << arg4 << " " << arg5 << " " << arg6 << " " << arg7 << "\n";
}

std::int64_t sum8(std::int64_t arg0, std::int64_t arg1, std::int64_t arg2,
                  std::int64_t arg3, std::int64_t arg4, std::int64_t arg5,
                  std::int64_t arg6, std::int64_t arg7) {
    return arg0 + arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7;
}

};
