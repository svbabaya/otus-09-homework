#include <thread>
#include <vector>
#include <iostream>
#include <mutex>

std::mutex mtx;
void safe_print(int i) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Thread ID: " << std::this_thread::get_id() << '\n';
    std::cout << i << '\n';
}
int main() {
    std::vector<std::thread> threadPool;
    for (int i = 1; i <= 9; ++i) {
        // threadPool.emplace_back([i]() { safe_print(i); });
        threadPool.emplace_back(safe_print, i);

    }
    // Digits 1 to 9 are printed (unordered)
    for (auto& t : threadPool) {
        t.join();
    } 
}