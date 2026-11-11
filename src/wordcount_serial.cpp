// wordcount_serial.cpp - reference implementation
#include "wordcount.h"
#include "tokenizer.h"

namespace wc {

Histogram countSerial(const std::vector<Chunk>& chunks) {
    Histogram h;
    for (const Chunk& c : chunks)
        countRange(c.data.data(), c.data.size(), c.begin, c.end, h);
    return h;
}

}  // namespace wc
