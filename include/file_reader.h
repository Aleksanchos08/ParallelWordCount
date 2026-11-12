#pragma once
#include <string>
#include <vector>
#include "common.h"
#include "file_list.h"
#include "partition.h"

namespace wc {

struct Chunk {
    std::vector<char> data;
    std::size_t       begin = 0;
    std::size_t       end   = 0;
};

bool readSegment(const FileInfo& file, const Segment& seg, Chunk& out, std::string& error);
bool readSegments(const FileList& list, const std::vector<Segment>& segs, std::vector<Chunk>& out,
                  std::string& error);

}
