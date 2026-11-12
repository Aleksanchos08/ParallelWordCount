#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace wc {

using Histogram = std::unordered_map<std::string, std::uint64_t>;  // word -> frequency
using WordCount = std::pair<std::string, std::uint64_t>;
using TopList   = std::vector<WordCount>;

constexpr std::uint64_t MAX_WORD_BYTES = 4096;

constexpr std::size_t TASK_BYTES = 256 * 1024;

enum class ReduceStrategy {
    Gather,   // all ranks -> rank 0 (MPI_Gatherv), rank 0 merges
    AllToAll  // words redistributed by hash(word) % P (MPI_Alltoallv), every rank merges its own slice
};

}
