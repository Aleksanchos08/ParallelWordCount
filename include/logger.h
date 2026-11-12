#pragma once
#include <string>

namespace wc {

class Logger {
public:

    static void init(int rank, int size, bool toFile, const std::string& dir);
    static void info(const char* fmt, ...);   // console: rank 0 only; file: every rank
    static void error(const char* fmt, ...);  // console (stderr): every rank
};

}
