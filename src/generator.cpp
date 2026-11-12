#include "generator.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <vector>

namespace fs = std::filesystem;

namespace wc {

bool generateDataset(const std::string& dir, int numFiles, int avgKB, std::string& error) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) { error = "cannot create " + dir + ": " + ec.message(); return false; }

    std::mt19937_64 rng(42);


    const int V = 50000;
    std::vector<std::string> vocab(V);
    for (auto& w : vocab) {
        const std::size_t len = 2 + rng() % 9;
        for (std::size_t j = 0; j < len; ++j) w.push_back(static_cast<char>('a' + rng() % 26));
    }
    std::vector<double> cdf(V);
    double acc = 0;
    for (int i = 0; i < V; ++i) { acc += 1.0 / (i + 1); cdf[i] = acc; }
    std::uniform_real_distribution<double> U(0.0, acc), U01(0.0, 1.0);

    std::ofstream master(fs::path(dir) / "00_master.txt");
    if (!master) { error = "cannot write master list in " + dir; return false; }

    for (int f = 0; f < numFiles; ++f) {
        char name[32];
        std::snprintf(name, sizeof(name), "file_%04d.txt", f + 1);
        std::ofstream out(fs::path(dir) / name, std::ios::binary);
        if (!out) { error = std::string("cannot write ") + name; return false; }


        const std::size_t target = static_cast<std::size_t>(avgKB * 1024.0 * (0.25 + 1.5 * U01(rng)));
        std::string text;
        std::size_t written = 0;
        int col = 0;
        while (written < target) {
            std::string w = vocab[std::lower_bound(cdf.begin(), cdf.end(), U(rng)) - cdf.begin()];
            if (rng() % 10 == 0) w[0] = static_cast<char>(w[0] - 32);  // "Capitalised"
            text += w;
            const unsigned r = static_cast<unsigned>(rng() % 20);
            if (r == 0) text += ',';
            else if (r == 1) text += '.';
            text += (++col % 12 == 0) ? '\n' : ' ';
            if (text.size() > (1u << 16)) { out.write(text.data(), static_cast<std::streamsize>(text.size())); written += text.size(); text.clear(); }
        }
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        master << name << "\n";
    }
    return true;
}

}  
