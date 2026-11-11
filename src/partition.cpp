// partition.cpp
#include "partition.h"

#include <algorithm>

namespace wc {

std::vector<Segment> segmentsForRank(const FileList& list, int rank, int numRanks) {
    const std::uint64_t total = list.totalSize;
    const std::uint64_t P     = static_cast<std::uint64_t>(numRanks);
    // This rank's slice of the virtual concatenated stream. Slice sizes differ by at most 1 byte.
    const std::uint64_t gBegin = total * static_cast<std::uint64_t>(rank) / P;
    const std::uint64_t gEnd   = total * static_cast<std::uint64_t>(rank + 1) / P;

    std::vector<Segment> segs;
    std::uint64_t fileStart = 0;  // global offset of the current file's first byte
    for (std::size_t i = 0; i < list.files.size(); ++i) {
        const std::uint64_t fileEnd = fileStart + list.files[i].size;
        const std::uint64_t lo = std::max(gBegin, fileStart);
        const std::uint64_t hi = std::min(gEnd, fileEnd);
        if (lo < hi) segs.push_back({i, lo - fileStart, hi - fileStart});  // may start/end mid-file
        fileStart = fileEnd;
        if (fileStart >= gEnd) break;
    }
    return segs;
}

}  // namespace wc
