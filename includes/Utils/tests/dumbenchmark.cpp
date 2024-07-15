#include <iostream>
#include <vector>
#include <chrono>

int main() {
    const int numIterations = 10000000;  // Number of iterations for testing
    std::vector<std::vector<int>> vectors(4);
    
    // Fill the vectors with some test data (for example, make them empty)
    for (auto& vec : vectors) {
        vec.clear();
    }

    // First approach
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i) {
        int result = (vectors[0].size() == 0 +
                      vectors[1].size() == 0) +
                     (vectors[2].size() == 0 +
                      vectors[3].size() == 0);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed1 = end1 - start1;
    std::cout << "First approach took: " << elapsed1.count() << " seconds.\n";

    // Second approach
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i) {
        int result = (vectors[0].empty() ? 1 : 0) +
                     (vectors[1].empty() ? 1 : 0) +
                     (vectors[2].empty() ? 1 : 0) +
                     (vectors[3].empty() ? 1 : 0);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = end2 - start2;
    std::cout << "Second approach took: " << elapsed2.count() << " seconds.\n";

    // Third approach with loop
    auto start3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i) {
        int result = 0;
        for (int j = 0; j < 4; ++j) {
            if (vectors[j].empty()) {
                ++result;
            }
        }
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed3 = end3 - start3;
    std::cout << "Third approach took: " << elapsed3.count() << " seconds.\n";

    return 0;
}
