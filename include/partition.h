#pragma once
#include <cstdint>
#include <vector>
#include "file_list.h"

namespace wc {

struct Segment {
    std::size_t   fileIndex;
    std::uint64_t begin;
    std::uint64_t end;
};

std::vector<Segment> segmentsForRank(const FileList& list, int rank, int numRanks);

}
