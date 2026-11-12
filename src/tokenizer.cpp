#include "tokenizer.h"
#include <string>

namespace wc {

void countRange(const char* data, std::size_t size, std::size_t begin, std::size_t end, Histogram& out) {
    if (begin >= size) return;
    if (end > size) end = size;

    std::size_t i = begin;

    if (i > 0 && isWordByte(static_cast<unsigned char>(data[i - 1])))
        while (i < size && isWordByte(static_cast<unsigned char>(data[i]))) ++i;

    std::string word;
    while (i < end) {
        if (!isWordByte(static_cast<unsigned char>(data[i]))) { ++i; continue; }
        word.clear();

        while (i < size && isWordByte(static_cast<unsigned char>(data[i]))) {
            word.push_back(toLowerAscii(static_cast<unsigned char>(data[i])));
            ++i;
        }
        ++out[word];
    }
}

}
