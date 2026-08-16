#include <cmath>

int main() {
    // Volatile forces the compiler to run the computation (prevents dead code elimination)
    volatile double x = 0.0;
    for (long long i = 0; i < 20000000; ++i) {
        x = x + std::sin(i) * std::cos(i);
    }
    return 0;
}