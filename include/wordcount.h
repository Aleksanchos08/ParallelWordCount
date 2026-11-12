#pragma once
#include <vector>
#include "common.h"
#include "file_reader.h"

namespace wc {

Histogram countSerial(const std::vector<Chunk>& chunks);
Histogram countOpenMP(const std::vector<Chunk>& chunks, int threads);

}
