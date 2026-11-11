// file_reader.cpp
#include "file_reader.h"

#include <algorithm>
#include <fstream>

namespace wc {

bool readSegment(const FileInfo& file, const Segment& seg, Chunk& out, std::string& error) {
    std::ifstream in(file.path, std::ios::binary);
    if (!in) { error = "cannot open " + file.path; return false; }

    // one byte BEFORE the segment (are we inside a word?) and a tail AFTER it (finish the last word)
    const std::uint64_t readStart = seg.begin > 0 ? seg.begin - 1 : 0;
    const std::uint64_t readEnd   = std::min<std::uint64_t>(file.size, seg.end + MAX_WORD_BYTES);

    out.data.resize(static_cast<std::size_t>(readEnd - readStart));
    in.seekg(static_cast<std::streamoff>(readStart));
    in.read(out.data.data(), static_cast<std::streamsize>(out.data.size()));
    out.data.resize(static_cast<std::size_t>(in.gcount()));  // file may have changed on disk

    out.begin = static_cast<std::size_t>(seg.begin - readStart);
    out.end   = static_cast<std::size_t>(seg.end - readStart);
    return true;
}

bool readSegments(const FileList& list, const std::vector<Segment>& segs, std::vector<Chunk>& out,
                  std::string& error) {
    out.clear();
    out.reserve(segs.size());
    for (const Segment& s : segs) {
        Chunk c;
        if (!readSegment(list.files[s.fileIndex], s, c, error)) return false;
        out.push_back(std::move(c));
    }
    return true;
}

}  // namespace wc
