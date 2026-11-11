// logger.h - rank-aware logging (console + optional per-rank files in logs/)
#pragma once
#include <string>

namespace wc {

class Logger {
public:
    // toFile=true -> every rank appends to <dir>/rank_<r>.log
    static void init(int rank, int size, bool toFile, const std::string& dir);
    static void info(const char* fmt, ...);   // console: rank 0 only; file: every rank
    static void error(const char* fmt, ...);  // console (stderr): every rank
};

}  // namespace wc
