// generator.h - synthetic test data (Zipf-distributed words, like natural language)
#pragma once
#include <string>

namespace wc {

// Creates `numFiles` text files of random size (avgKB * 0.25 .. 1.75) plus <dir>/00_master.txt.
bool generateDataset(const std::string& dir, int numFiles, int avgKB, std::string& error);

}  // namespace wc
