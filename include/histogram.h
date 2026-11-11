// histogram.h - statistics, top-K and (de)serialisation of histograms
#pragma once
#include <string>
#include <vector>
#include "common.h"

namespace wc {

struct Stats {
    unsigned long long total    = 0;  // total number of words
    unsigned long long unique   = 0;  // number of distinct words
    unsigned long long checksum = 0;  // order-independent fingerprint, used to verify correctness
};

Stats   computeStats(const Histogram& h);
TopList topK(const Histogram& h, std::size_t k);  // ties broken alphabetically (deterministic)

// Wire format for one pair: word bytes, '\0', 8-byte count.  ('\0' is never part of a word.)
void appendPair(std::vector<char>& buf, const std::string& word, std::uint64_t count);
void mergeFromBuffer(const char* data, std::size_t size, Histogram& into);

// Which rank owns `word` in the AllToAll reduction.
std::size_t ownerOf(const std::string& word, int numRanks);

}  // namespace wc
