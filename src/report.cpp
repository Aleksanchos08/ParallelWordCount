// report.cpp
#include "report.h"

#include <cstdio>
#include <filesystem>
#include <fstream>

namespace wc {

static const char* modeName(const Options& o) { return o.mode.c_str(); }
static const char* reduceName(const Options& o) {
    if (o.mode != "mpi") return "-";
    return o.reduce == ReduceStrategy::Gather ? "gather" : "alltoall";
}

void printReport(const Options& opt, const RunResult& r) {
    std::printf("\n=== Parallel Word Count: %s | %d rank(s) x %d thread(s) | reduce=%s ===\n", modeName(opt), r.ranks,
                r.threads, reduceName(opt));
    std::printf("files: %llu   bytes: %llu\n", (unsigned long long)r.files, (unsigned long long)r.bytes);
    std::printf("words: %llu   distinct: %llu   checksum: %llu\n", r.stats.total, r.stats.unique, r.stats.checksum);
    std::printf("time [s]  setup %.4f | read %.4f | count %.4f | reduce %.4f | TOTAL %.4f\n", r.t.setup, r.t.read,
                r.t.count, r.t.comm, r.t.total);
    std::printf("top %zu:\n", r.top.size());
    for (std::size_t i = 0; i < r.top.size(); ++i)
        std::printf("  %2zu. %-24s %llu\n", i + 1, r.top[i].first.c_str(), (unsigned long long)r.top[i].second);
}

bool appendCsv(const std::string& path, const Options& opt, const RunResult& r) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const bool fresh = !fs::exists(path, ec) || fs::file_size(path, ec) == 0;
    if (fs::path(path).has_parent_path()) fs::create_directories(fs::path(path).parent_path(), ec);

    std::ofstream out(path, std::ios::app);
    if (!out) return false;
    if (fresh)
        out << "mode,ranks,threads,reduce,files,bytes,setup_s,read_s,count_s,reduce_s,total_s,words,distinct,checksum\n";
    char line[512];
    std::snprintf(line, sizeof(line), "%s,%d,%d,%s,%llu,%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%llu,%llu,%llu\n", modeName(opt),
                  r.ranks, r.threads, reduceName(opt), (unsigned long long)r.files, (unsigned long long)r.bytes,
                  r.t.setup, r.t.read, r.t.count, r.t.comm, r.t.total, r.stats.total, r.stats.unique,
                  r.stats.checksum);
    out << line;
    return true;
}

}  // namespace wc
