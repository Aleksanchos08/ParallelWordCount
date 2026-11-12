#pragma once
#include <vector>
#include "common.h"
#include "file_list.h"
#include "histogram.h"

namespace wc {

void barrier();

bool broadcastFileList(FileList& list, bool rootOk, int rank);   // MPI_Bcast
bool agreeAll(bool ok);

Histogram reduceHistograms(Histogram& local, ReduceStrategy strategy, int rank, int numRanks);

Stats   reduceStats(const Stats& mine);                                         // sums, valid on rank 0
TopList gatherTop(const Histogram& owned, std::size_t k, int rank, int numRanks);  // valid on rank 0
std::vector<double> maxOverRanks(const std::vector<double>& mine);              // valid on rank 0

} 
