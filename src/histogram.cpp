// histogram.cpp
#include "histogram.h"

#include <algorithm>
#include <cstring>
#include <functional>

namespace wc {

Stats computeStats(const Histogram& h) {
    std::hash<std::string> H;
    Stats s;
    for (const auto& kv : h) {
        s.total += kv.second;
        s.unique += 1;
        s.checksum += static_cast<unsigned long long>(H(kv.first)) * kv.second;  // wraps, order independent
    }
    return s;
}

TopList topK(const Histogram& h, std::size_t k) {
    TopList v(h.begin(), h.end());
    k = std::min(k, v.size());
    std::partial_sort(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k), v.end(),
                      [](const WordCount& a, const WordCount& b) {
                          return a.second != b.second ? a.second > b.second : a.first < b.first;
                      });
    v.resize(k);
    return v;
}

void appendPair(std::vector<char>& buf, const std::string& word, std::uint64_t count) {
    buf.insert(buf.end(), word.begin(), word.end());
    buf.push_back('\0');
    const char* p = reinterpret_cast<const char*>(&count);
    buf.insert(buf.end(), p, p + sizeof(count));
}

void mergeFromBuffer(const char* data, std::size_t size, Histogram& into) {
    std::size_t i = 0;
    while (i < size) {
        const std::size_t len = std::strlen(data + i);
        std::string word(data + i, len);
        i += len + 1;
        std::uint64_t count;
        std::memcpy(&count, data + i, sizeof(count));
        i += sizeof(count);
        into[word] += count;
    }
}

std::size_t ownerOf(const std::string& word, int numRanks) {
    return std::hash<std::string>()(word) % static_cast<std::size_t>(numRanks);
}

}  // namespace wc
