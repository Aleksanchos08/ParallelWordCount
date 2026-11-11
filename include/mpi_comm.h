// mpi_comm.h - all MPI communication of the project lives here
#pragma once
#include <vector>
#include "common.h"
#include "file_list.h"
#include "histogram.h"

namespace wc {

void barrier();

// Rank 0 passes its list (rootOk=false if loading failed). Returns false on every rank on failure.
bool broadcastFileList(FileList& list, bool rootOk, int rank);   // MPI_Bcast

// True only if `ok` is true on every rank (MPI_Allreduce).
bool agreeAll(bool ok);

// Combines local histograms. Returns the part of the global histogram held by this rank:
//   Gather   -> rank 0 holds everything, the others return an empty histogram;
//   AllToAll -> every rank holds a disjoint slice (words with ownerOf(word) == rank).
Histogram reduceHistograms(Histogram& local, ReduceStrategy strategy, int rank, int numRanks);

Stats   reduceStats(const Stats& mine);                                         // sums, valid on rank 0
TopList gatherTop(const Histogram& owned, std::size_t k, int rank, int numRanks);  // valid on rank 0
std::vector<double> maxOverRanks(const std::vector<double>& mine);              // valid on rank 0

}  // namespace wc
