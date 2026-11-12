#include "options.h"
#include <cstdlib>

namespace wc {

const char* usageText() {
    return
        "Usage:\n"
        "  wc gen    <outDir> <numFiles> <avgSizeKB>     generate a test data set + 00_master.txt\n"
        "  wc serial <master.txt> [options]              serial baseline\n"
        "  wc omp    <master.txt> [options]              OpenMP, one process\n"
        "  mpiexec -n R wc mpi <master.txt> [options]    hybrid MPI + OpenMP\n"
        "Options:\n"
        "  --threads N              OpenMP threads (per process)\n"
        "  --reduce gather|alltoall how MPI ranks combine histograms (default alltoall)\n"
        "  --topk K                 print K most frequent words (default 10)\n"
        "  --csv FILE               append one result row to FILE\n"
        "  --log                    write per-rank logs to logs/\n";
}

Options parseOptions(int argc, char** argv) {
    Options o;
    if (argc < 2) { o.error = "missing mode"; return o; }
    o.mode = argv[1];

    if (o.mode == "gen") {
        if (argc < 5) { o.error = "gen needs <outDir> <numFiles> <avgSizeKB>"; return o; }
        o.input    = argv[2];
        o.genFiles = std::atoi(argv[3]);
        o.genKB    = std::atoi(argv[4]);
        if (o.genFiles <= 0 || o.genKB <= 0) { o.error = "numFiles and avgSizeKB must be positive"; return o; }
        o.valid = true;
        return o;
    }

    if (o.mode != "serial" && o.mode != "omp" && o.mode != "mpi") { o.error = "unknown mode '" + o.mode + "'"; return o; }
    if (argc < 3) { o.error = "missing master list"; return o; }
    o.input = argv[2];

    for (int i = 3; i < argc; ++i) {
        const std::string a = argv[i];
        const bool hasValue = i + 1 < argc;
        if (a == "--threads" && hasValue)      o.threads = std::atoi(argv[++i]);
        else if (a == "--topk" && hasValue)    o.topk = static_cast<std::size_t>(std::atoi(argv[++i]));
        else if (a == "--csv" && hasValue)     o.csv = argv[++i];
        else if (a == "--log")                 o.logFiles = true;
        else if (a == "--reduce" && hasValue) {
            const std::string v = argv[++i];
            if (v == "gather")        o.reduce = ReduceStrategy::Gather;
            else if (v == "alltoall") o.reduce = ReduceStrategy::AllToAll;
            else { o.error = "--reduce must be gather or alltoall"; return o; }
        } else { o.error = "unknown or incomplete option '" + a + "'"; return o; }
    }
    o.valid = true;
    return o;
}

}
