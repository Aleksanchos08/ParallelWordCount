#pragma once
#include "common.h"

namespace wc {

inline bool isWordByte(unsigned char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 128;
}
inline char toLowerAscii(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : static_cast<char>(c);
}
void countRange(const char* data, std::size_t size, std::size_t begin, std::size_t end, Histogram& out);

}
