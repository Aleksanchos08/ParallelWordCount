// pipeline.cpp - serial / OpenMP / hybrid execution modes
//
//   Phase       serial & omp                       mpi
//   ---------   --------------------------------   ---------------------------------------------
//   setup       load master list                   rank 0 loads list -> MPI_Bcast -> every rank computes its segments
//   read        read all files                     every rank reads only its own byte range
//   count       one thread / OpenMP threads        OpenMP threads inside every rank
//   comm        -                                  MPI_Gatherv or MPI_Alltoallv, then tiny reductions
#include "pipeline.h"

#include <omp.h>

#include <algorithm>

#include "file_list.h"
#include "file_reader.h"
#include "logger.h"
#include "mpi_comm.h"
#include "partition.h"
#include "timer.h"
#include "wordcount.h"

namespace wc {
namespace {

RunResult runLocal(const Options& opt, bool serial) {
    RunResult r;
    r.ranks   = 1;
    r.threads = serial ? 1 : (opt.threads > 0 ? opt.threads : omp_get_max_threads());

    Timer wall, t;
    FileList list;
    std::string err;
    if (!loadFileList(opt.input, list, err)) { Logger::error("%s", err.c_str()); return r; }
    const std::vector<Segment> segs = segmentsForRank(list, 0, 1);  // everything
    r.files = list.files.size();
    r.bytes = list.totalSize;
    r.t.setup = t.elapsed();
    Logger::info("%zu files, %llu bytes", list.files.size(), (unsigned long long)list.totalSize);

    t.reset();
    std::vector<Chunk> chunks;
    if (!readSegments(list, segs, chunks, err)) { Logger::error("%s", err.c_str()); return r; }
    r.t.read = t.elapsed();

    t.reset();
    Histogram h = serial ? countSerial(chunks) : countOpenMP(chunks, r.threads);
    r.t.count = t.elapsed();

    r.stats = computeStats(h);
    r.top   = topK(h, opt.topk);
    r.t.total = wall.elapsed();
    r.ok = true;
    return r;
}

}  // namespace

RunResult runSerial(const Options& opt) { return runLocal(opt, true); }
RunResult runOpenMP(const Options& opt) { return runLocal(opt, false); }

RunResult runMpi(const Options& opt, int rank, int P) {
    RunResult r;
    r.ranks   = P;
    r.threads = opt.threads > 0 ? opt.threads : 1;

    barrier();  // start all ranks together so timings are comparable
    Timer wall, t;

    // ---- 1. setup: only rank 0 reads the master list, then it is broadcast ----
    FileList list;
    std::string err;
    bool rootOk = true;
    if (rank == 0) {
        rootOk = loadFileList(opt.input, list, err);
        if (!rootOk) Logger::error("%s", err.c_str());
    }
    if (!broadcastFileList(list, rootOk, rank)) return r;  // same decision on all ranks

    // each rank derives its own part of the work - no scatter needed
    const std::vector<Segment> segs = segmentsForRank(list, rank, P);
    std::uint64_t myBytes = 0;
    for (const Segment& s : segs) myBytes += s.end - s.begin;
    r.files = list.files.size();
    r.bytes = list.totalSize;
    const double tSetup = t.elapsed();
    Logger::info("%zu files, %llu bytes total, ~%llu bytes per rank, %d thread(s) per rank", list.files.size(),
                 (unsigned long long)list.totalSize, (unsigned long long)myBytes, r.threads);

    // ---- 2. read: every rank reads only its own segments ----
    t.reset();
    std::vector<Chunk> chunks;
    const bool readOk = readSegments(list, segs, chunks, err);
    if (!readOk) Logger::error("%s", err.c_str());
    if (!agreeAll(readOk)) return r;                       // never leave some ranks waiting in a collective
    const double tRead = t.elapsed();

    // ---- 3. count: local histogram, OpenMP inside the rank ("map") ----
    t.reset();
    Histogram local = countOpenMP(chunks, r.threads);
    chunks.clear();
    chunks.shrink_to_fit();
    const double tCount = t.elapsed();
    Logger::info("local histogram: %zu distinct words", local.size());

    // ---- 4. reduce: combine histograms across ranks ----
    t.reset();
    Histogram owned = reduceHistograms(local, opt.reduce, rank, P);
    const Stats ls = computeStats(owned);
    r.stats = reduceStats(ls);
    r.top   = gatherTop(owned, opt.topk, rank, P);
    const double tComm = t.elapsed();

    // slowest rank defines the phase time
    const std::vector<double> mx = maxOverRanks({tSetup, tRead, tCount, tComm, wall.elapsed()});
    if (rank == 0) { r.t.setup = mx[0]; r.t.read = mx[1]; r.t.count = mx[2]; r.t.comm = mx[3]; r.t.total = mx[4]; }
    r.ok = true;
    return r;
}

}  // namespace wc
