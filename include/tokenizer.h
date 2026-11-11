// tokenizer.h - word definition and the range-based counting primitive
#pragma once
#include "common.h"

namespace wc {

// A word is a maximal run of [A-Za-z0-9] or bytes >= 128 (keeps UTF-8 words intact).
inline bool isWordByte(unsigned char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 128;
}
inline char toLowerAscii(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : static_cast<char>(c);
}

// Counts every word whose FIRST byte lies in [begin, end) of data[0..size).
//  - a word that starts before `end` may run past `end` (it is still counted here);
//  - a word that started before `begin` is skipped (the previous range owns it).
// This single rule makes any partition of the data (between threads, ranks, files)
// count every word exactly once.
void countRange(const char* data, std::size_t size, std::size_t begin, std::size_t end, Histogram& out);

}  // namespace wc
