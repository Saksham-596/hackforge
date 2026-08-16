#include <iostream>
#include <vector>

int main() {
    int n;
    if (!(std::cin >> n)) return 0;
    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    long long sum = 0;
    // Algorithmic Trap: O(N^2) behavior if the array contains '42'
    for (int i = 0; i < n; ++i) {
        if (a[i] == 42) {
            for (int j = 0; j < n; ++j) sum += a[j]; 
        } else {
            sum += a[i];
        }
    }
    return 0;
}
