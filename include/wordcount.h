// wordcount.h - local (single process) counting kernels
#pragma once
#include <vector>
#include "common.h"
#include "file_reader.h"

namespace wc {

// Reference implementation: one thread, one hash map.
Histogram countSerial(const std::vector<Chunk>& chunks);

// OpenMP implementation:
//  1. all chunks are cut into tasks of TASK_BYTES; threads take tasks dynamically
//     and count into thread-private histograms (no locks);
//  2. private histograms are merged in parallel: thread t owns keys with hash(word)%T == t
//     (no critical section).
Histogram countOpenMP(const std::vector<Chunk>& chunks, int threads);

}  // namespace wc
