// partition.h - byte-balanced distribution of the files among workers
#pragma once
#include <cstdint>
#include <vector>
#include "file_list.h"

namespace wc {

// A contiguous byte range [begin, end) of one file.
struct Segment {
    std::size_t   fileIndex;
    std::uint64_t begin;
    std::uint64_t end;
};

// Imagine all files glued into one stream of `totalSize` bytes. Worker `rank` of `numRanks`
// gets bytes [total*rank/P, total*(rank+1)/P). That range is mapped back to segments of
// one or several files (possibly starting/ending in the middle of a file).
// Every rank gets the same amount of bytes (+-1) no matter how file sizes differ.
// Computed locally by every rank from the (broadcast) file list - no scatter needed.
std::vector<Segment> segmentsForRank(const FileList& list, int rank, int numRanks);

}  // namespace wc
