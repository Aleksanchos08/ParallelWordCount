// main.cpp - entry point: parse options, initialise MPI, run the selected mode, report.
#include <mpi.h>

#include <cstdio>

#include "generator.h"
#include "logger.h"
#include "options.h"
#include "pipeline.h"
#include "report.h"

int main(int argc, char** argv) {
    // FUNNELED: only the main thread calls MPI; OpenMP threads are used between MPI calls.
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    int rank = 0, size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rc = 0;
    const wc::Options opt = wc::parseOptions(argc, argv);
    if (!opt.valid) {
        if (rank == 0) std::fprintf(stderr, "Error: %s\n\n%s", opt.error.c_str(), wc::usageText());
        rc = 1;
    } else {
        wc::Logger::init(rank, size, opt.logFiles, "logs");

        if (opt.mode == "gen") {
            if (rank == 0) {
                std::string err;
                if (wc::generateDataset(opt.input, opt.genFiles, opt.genKB, err))
                    wc::Logger::info("generated %d files in %s (master list: 00_master.txt)", opt.genFiles, opt.input.c_str());
                else { wc::Logger::error("%s", err.c_str()); rc = 1; }
            }
        } else if (opt.mode != "mpi" && size != 1) {
            if (rank == 0) std::fprintf(stderr, "Error: modes serial/omp must run as a single process (no mpiexec).\n");
            rc = 1;
        } else {
            wc::RunResult res;
            if (opt.mode == "serial")   res = wc::runSerial(opt);
            else if (opt.mode == "omp") res = wc::runOpenMP(opt);
            else                        res = wc::runMpi(opt, rank, size);

            if (!res.ok) rc = 1;
            else if (rank == 0) {
                wc::printReport(opt, res);
                if (!opt.csv.empty() && !wc::appendCsv(opt.csv, opt, res))
                    wc::Logger::error("cannot write %s", opt.csv.c_str());
            }
        }
    }
    MPI_Finalize();
    return rc;
}
