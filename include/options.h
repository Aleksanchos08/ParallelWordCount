#pragma once
#include <string>
#include "common.h"

namespace wc {

struct Options {
    std::string    mode;                     // gen | serial | omp | mpi
    std::string    input;                    // master list (or output dir for gen)
    int            threads  = 0;             // 0 = default (omp: all cores, mpi: 1)
    std::size_t    topk     = 10;
    ReduceStrategy reduce   = ReduceStrategy::AllToAll;
    std::string    csv;                      // append a result row to this file
    bool           logFiles = false;         // write logs/rank_<r>.log
    int            genFiles = 0;             // gen: number of files
    int            genKB    = 0;             // gen: average file size in KB
    bool           valid    = false;
    std::string    error;
};

Options parseOptions(int argc, char** argv);
const char* usageText();

}
