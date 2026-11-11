// file_reader.h - reads a segment with the extra bytes needed for correct word borders
#pragma once
#include <string>
#include <vector>
#include "common.h"
#include "file_list.h"
#include "partition.h"

namespace wc {

// Bytes read from disk plus the sub-range of them this worker owns.
// Layout of `data`: [1 byte before the segment][segment][up to MAX_WORD_BYTES after it]
//  - the byte before tells whether the segment starts in the middle of a word;
//  - the bytes after let us finish the last word, which starts inside the segment.
// A word belongs to this chunk iff its first byte is in [begin, end) (see countRange).
struct Chunk {
    std::vector<char> data;
    std::size_t       begin = 0;
    std::size_t       end   = 0;
};

bool readSegment(const FileInfo& file, const Segment& seg, Chunk& out, std::string& error);
bool readSegments(const FileList& list, const std::vector<Segment>& segs, std::vector<Chunk>& out,
                  std::string& error);

}  // namespace wc
