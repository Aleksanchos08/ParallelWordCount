#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "timer.h"

namespace wc {
namespace {

int           g_rank = 0, g_size = 1;
std::ofstream g_file;
Timer         g_clock;  // timestamps are "seconds since start of the process"

void emit(const char* level, std::FILE* console, bool toConsole, const char* fmt, std::va_list ap) {
    char msg[1024];
    std::vsnprintf(msg, sizeof(msg), fmt, ap);
    if (toConsole) std::fprintf(console, "[rank %d/%d] %s%s\n", g_rank, g_size, level, msg);
    if (g_file.is_open())
        g_file << "[" << g_clock.elapsed() << "s] " << level << msg << std::endl;
}

}

void Logger::init(int rank, int size, bool toFile, const std::string& dir) {
    g_rank = rank;
    g_size = size;
    g_clock.reset();
    if (toFile) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        g_file.open(dir + "/rank_" + std::to_string(rank) + ".log", std::ios::app);
    }
}

void Logger::info(const char* fmt, ...) {
    std::va_list ap;
    va_start(ap, fmt);
    emit("", stdout, g_rank == 0, fmt, ap);
    va_end(ap);
}

void Logger::error(const char* fmt, ...) {
    std::va_list ap;
    va_start(ap, fmt);
    emit("ERROR: ", stderr, true, fmt, ap);
    va_end(ap);
}

}
