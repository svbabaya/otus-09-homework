// Read files and prints top k word by frequency

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <chrono>
#include <thread>

const size_t TOPK = 10;

using Counter = std::map<std::string, std::size_t>;

std::string tolower(const std::string& str);

void count_words(char* file_name, Counter&);
Counter sum_up_counters(std::vector<Counter>);

class FailedToOpenFileException {
public: 
    FailedToOpenFileException(std::string message) : m_message{message} {}
    std::string getMessage() const {
        return m_message;
    }
private:
    std::string m_message;
};

void print_topk(std::ostream& stream, const Counter&, const size_t k);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: topk_words [FILES...]\n";
        return EXIT_FAILURE;
    }

    auto start = std::chrono::high_resolution_clock::now();

    Counter freq_all_dict;
    std::vector<Counter> counters(static_cast<size_t>(argc) - 1);
    std::vector<std::thread> thread_pool;

    try {
        for (size_t i = 1; i < static_cast<size_t>(argc); ++i) {
            thread_pool.push_back(std::thread(count_words, argv[i], std::ref(counters[i - 1])));
        }
        for (auto& t : thread_pool) {
            t.join();
        } 
    } catch (FailedToOpenFileException ex) {
        std::cout << ex.getMessage() << std::endl;
    }

    freq_all_dict = sum_up_counters(counters);

    print_topk(std::cout, freq_all_dict, TOPK);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Elapsed time is " << elapsed_ms.count() << " us\n";
}

Counter sum_up_counters(std::vector<Counter> counters) {
    Counter tmp = counters[0];
    for (size_t i = 1; i < counters.size(); ++i) {
        for (auto el : counters[i]) {
            if (tmp.count(el.first)) {
                tmp[el.first] += counters[i][el.first];
            } else {
                tmp[el.first] = el.second;
            }
        }
    }
    return tmp;
}

std::string tolower(const std::string& str) {
    std::string lower_str;
    std::transform(std::cbegin(str), std::cend(str),
                   std::back_inserter(lower_str),
                   [](unsigned char ch) { return std::tolower(ch); });
    return lower_str;
};

void count_words(char* file_name, Counter& counter) {

    std::ifstream stream{file_name};
    if (!stream.is_open()) {
        throw FailedToOpenFileException("Faild to open file");
    }

    std::for_each(std::istream_iterator<std::string>(stream),
                  std::istream_iterator<std::string>(),
                  [&counter](const std::string& s) { ++counter[tolower(s)]; });    
}

void print_topk(std::ostream& stream, const Counter& counter, const size_t k) {
    std::vector<Counter::const_iterator> words;
    words.reserve(counter.size());
    for (auto it = std::cbegin(counter); it != std::cend(counter); ++it) {
        words.push_back(it);
    }

    std::partial_sort(
        std::begin(words), std::begin(words) + k, std::end(words),
        [](auto lhs, auto &rhs) { return lhs->second > rhs->second; });

    std::for_each(
        std::begin(words), std::begin(words) + k,
        [&stream](const Counter::const_iterator &pair) {
            stream << std::setw(4) << pair->second << " " << pair->first
                      << '\n';
        });
}
