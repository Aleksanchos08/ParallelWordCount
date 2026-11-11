// report.h - console summary and CSV output
#pragma once
#include "options.h"
#include "pipeline.h"

namespace wc {

void printReport(const Options& opt, const RunResult& r);
bool appendCsv(const std::string& path, const Options& opt, const RunResult& r);

}  // namespace wc
