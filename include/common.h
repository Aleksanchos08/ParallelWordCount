// common.h - shared types and constants
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

// How many bytes past the end of its range a worker may read to finish its last word.
// Words longer than this are truncated (documented limitation).
constexpr std::uint64_t MAX_WORD_BYTES = 4096;

// Granularity of OpenMP tasks (smaller = better load balance, larger = less overhead).
constexpr std::size_t TASK_BYTES = 256 * 1024;

// How local histograms are combined across MPI ranks.
enum class ReduceStrategy {
    Gather,   // all ranks -> rank 0 (MPI_Gatherv), rank 0 merges   (baseline, like the reference project)
    AllToAll  // words redistributed by hash(word) % P (MPI_Alltoallv), every rank merges its own slice
};

}  // namespace wc
