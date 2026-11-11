// mpi_comm.cpp - every MPI call of the project is in this file
#include "mpi_comm.h"

#include <cstdio>
#include <mpi.h>

#include <algorithm>

namespace wc {
namespace {

// Variable-size gather of byte blocks to rank 0 (MPI_Gather for sizes + MPI_Gatherv for data).
void gatherBytes(const std::vector<char>& mine, std::vector<char>& all, int rank, int P) {
    int n = static_cast<int>(mine.size());
    std::vector<int> counts(P), displs(P);
    MPI_Gather(&n, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    int total = 0;
    if (rank == 0) {
        for (int p = 0; p < P; ++p) { displs[p] = total; total += counts[p]; }
        all.resize(static_cast<std::size_t>(total));
    }
    MPI_Gatherv(mine.data(), n, MPI_CHAR, all.data(), counts.data(), displs.data(), MPI_CHAR, 0, MPI_COMM_WORLD);
}

// Hash-partitioned all-to-all exchange: every word travels to rank ownerOf(word).
Histogram exchangeByOwner(Histogram& local, int P) {
    std::vector<std::vector<char>> out(P);
    for (const auto& kv : local) appendPair(out[ownerOf(kv.first, P)], kv.first, kv.second);
    local.clear();

    std::vector<int> sendCounts(P), sendDispls(P), recvCounts(P), recvDispls(P);
    int sendTotal = 0;
    for (int p = 0; p < P; ++p) { sendCounts[p] = static_cast<int>(out[p].size()); sendDispls[p] = sendTotal; sendTotal += sendCounts[p]; }
    std::vector<char> sendBuf;
    sendBuf.reserve(static_cast<std::size_t>(sendTotal));
    for (int p = 0; p < P; ++p) sendBuf.insert(sendBuf.end(), out[p].begin(), out[p].end());
    out.clear();

    MPI_Alltoall(sendCounts.data(), 1, MPI_INT, recvCounts.data(), 1, MPI_INT, MPI_COMM_WORLD);
    int recvTotal = 0;
    for (int p = 0; p < P; ++p) { recvDispls[p] = recvTotal; recvTotal += recvCounts[p]; }
    std::vector<char> recvBuf(static_cast<std::size_t>(recvTotal));

    MPI_Alltoallv(sendBuf.data(), sendCounts.data(), sendDispls.data(), MPI_CHAR,
                  recvBuf.data(), recvCounts.data(), recvDispls.data(), MPI_CHAR, MPI_COMM_WORLD);

    Histogram mine;
    mergeFromBuffer(recvBuf.data(), recvBuf.size(), mine);
    return mine;
}

}  // namespace

void barrier() { MPI_Barrier(MPI_COMM_WORLD); }

bool broadcastFileList(FileList& list, bool rootOk, int rank) {
    std::vector<char> buf;
    int n = -1;  // -1 = loading failed on rank 0
    if (rank == 0 && rootOk) { buf = packFileList(list); n = static_cast<int>(buf.size()); }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (n < 0) return false;

    if (rank != 0) buf.resize(static_cast<std::size_t>(n));
    MPI_Bcast(buf.data(), n, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank != 0) list = unpackFileList(buf.data(), buf.size());
    return true;
}

bool agreeAll(bool ok) {
    int mine = ok ? 1 : 0, all = 0;
    MPI_Allreduce(&mine, &all, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    return all == 1;
}

Histogram reduceHistograms(Histogram& local, ReduceStrategy strategy, int rank, int P) {
    Histogram result;
    if (P == 1) { result.swap(local); return result; }

    if (strategy == ReduceStrategy::AllToAll) return exchangeByOwner(local, P);

    // Gather: everything to rank 0, rank 0 merges (simple, but rank 0 becomes the bottleneck)
    std::vector<char> mine, all;
    for (const auto& kv : local) appendPair(mine, kv.first, kv.second);
    local.clear();
    gatherBytes(mine, all, rank, P);
    if (rank == 0) mergeFromBuffer(all.data(), all.size(), result);
    return result;
}

Stats reduceStats(const Stats& mine) {
    Stats g;
    MPI_Reduce(&mine.total,    &g.total,    1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&mine.unique,   &g.unique,   1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&mine.checksum, &g.checksum, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    return g;
}

// The global top-K is contained in the union of the per-rank top-K lists
// (words are never split between ranks after the reduce step).
TopList gatherTop(const Histogram& owned, std::size_t k, int rank, int P) {
    std::vector<char> mine, all;
    for (const auto& wcnt : topK(owned, k)) appendPair(mine, wcnt.first, wcnt.second);
    gatherBytes(mine, all, rank, P);
    if (rank != 0) return TopList();
    Histogram candidates;
    mergeFromBuffer(all.data(), all.size(), candidates);
    return topK(candidates, k);
}

std::vector<double> maxOverRanks(const std::vector<double>& mine) {
    std::vector<double> out(mine.size(), 0.0);
    MPI_Reduce(mine.data(), out.data(), static_cast<int>(mine.size()), MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    return out;
}

}  // namespace wc
