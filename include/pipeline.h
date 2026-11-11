// pipeline.h - the three execution modes
#pragma once
#include "common.h"
#include "histogram.h"
#include "options.h"

namespace wc {

struct Timings {
    double setup = 0;  // load + broadcast file list, compute segments
    double read  = 0;  // disk I/O
    double count = 0;  // local counting (the parallel "map" phase)
    double comm  = 0;  // combining histograms (the "reduce" phase)
    double total = 0;  // wall time of the whole run
};

struct RunResult {
    bool          ok = false;
    int           ranks = 1;
    int           threads = 1;
    std::uint64_t files = 0;
    std::uint64_t bytes = 0;
    Stats         stats;   // global (valid on rank 0)
    TopList       top;     // global (valid on rank 0)
    Timings       t;       // slowest rank per phase (valid on rank 0)
};

RunResult runSerial(const Options& opt);
RunResult runOpenMP(const Options& opt);
RunResult runMpi(const Options& opt, int rank, int numRanks);

}  // namespace wc
