#pragma once
#include <string>

namespace wc {

bool generateDataset(const std::string& dir, int numFiles, int avgKB, std::string& error);

}  
