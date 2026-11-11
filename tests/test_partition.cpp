// test_partition.cpp - correctness tests that need NO MPI.
// For many process counts P it simulates all ranks one after another and checks that
//   (1) the byte ranges cover every file exactly once and are balanced (+-1 byte),
//   (2) the merged per-rank histograms equal the serial histogram of the whole data,
//   (3) countOpenMP gives the same histogram as countSerial,
//   (4) (de)serialisation of file lists and histograms round-trips.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "file_list.h"
#include "file_reader.h"
#include "generator.h"
#include "histogram.h"
#include "partition.h"
#include "wordcount.h"

namespace fs = std::filesystem;
using namespace wc;

static int g_fail = 0;
#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) { std::printf("  FAIL: %s\n", msg); ++g_fail; }  \
    } while (0)

static void writeFile(const fs::path& p, const std::string& s) {
    std::ofstream(p, std::ios::binary) << s;
}

static Histogram referenceCount(const FileList& list) {          // whole files, one thread
    std::vector<Segment> all;
    for (std::size_t i = 0; i < list.files.size(); ++i)
        if (list.files[i].size) all.push_back({i, 0, list.files[i].size});
    std::vector<Chunk> chunks;
    std::string err;
    readSegments(list, all, chunks, err);
    return countSerial(chunks);
}

static void runDataset(const std::string& name, const FileList& list) {
    std::printf("dataset '%s': %zu files, %llu bytes\n", name.c_str(), list.files.size(),
                (unsigned long long)list.totalSize);
    const Histogram ref = referenceCount(list);
    const Stats refStats = computeStats(ref);

    for (int P : {1, 2, 3, 4, 5, 7, 8, 13, 31, 64}) {
        Histogram merged;
        std::uint64_t covered = 0, minB = ~0ull, maxB = 0;
        for (int rank = 0; rank < P; ++rank) {
            const auto segs = segmentsForRank(list, rank, P);
            std::uint64_t bytes = 0;
            for (const auto& s : segs) bytes += s.end - s.begin;
            covered += bytes;
            minB = std::min(minB, bytes);
            maxB = std::max(maxB, bytes);

            std::vector<Chunk> chunks;
            std::string err;
            CHECK(readSegments(list, segs, chunks, err), "readSegments failed");

            Histogram local = countOpenMP(chunks, 3);               // OpenMP path
            Histogram serial = countSerial(chunks);                 // serial path on the same chunks
            CHECK(local == serial, "countOpenMP != countSerial");
            for (const auto& kv : local) merged[kv.first] += kv.second;
        }
        CHECK(covered == list.totalSize, "segments do not cover all bytes exactly once");
        CHECK(maxB - minB <= 1, "ranks are not balanced (+-1 byte)");
        CHECK(merged == ref, "merged histogram differs from serial reference");
        const Stats s = computeStats(merged);
        CHECK(s.total == refStats.total && s.checksum == refStats.checksum, "stats differ");
    }
}

int main() {
    const fs::path dir = fs::temp_directory_path() / "wc_test_data";
    fs::remove_all(dir);
    fs::create_directories(dir);

    // ---- hand-made nasty files ----
    writeFile(dir / "a.txt", "hello world");                              // no trailing newline
    writeFile(dir / "b.txt", "");                                          // empty file
    writeFile(dir / "c.txt", "Hello,HELLO;hello\nfoo\n\n\n  bar\t\tfoo");  // case + punctuation
    writeFile(dir / "d.txt", std::string(3000, 'x') + " " + std::string(3000, 'x'));  // very long words
    writeFile(dir / "e.txt", "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 \xD0\xBC\xD0\xB8\xD1\x80 "
                             "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82");     // UTF-8 "privet mir privet"
    writeFile(dir / "f.txt", "   \n\t  ");                                 // whitespace only
    writeFile(dir / "g.txt", "a");                                         // one byte
    {
        std::ofstream m(dir / "master_small.txt");
        for (const char* n : {"a.txt", "b.txt", "c.txt", "d.txt", "e.txt", "f.txt", "g.txt"}) m << n << "\n";
        m << "# comment lines and blank lines are ignored\n\n";
    }
    FileList small;
    std::string err;
    CHECK(loadFileList((dir / "master_small.txt").string(), small, err), "loadFileList (small)");
    runDataset("hand-made", small);

    // ---- generated data: many files with different sizes ----
    CHECK(generateDataset((dir / "gen").string(), 9, 40, err), "generateDataset");
    FileList gen;
    CHECK(loadFileList((dir / "gen" / "00_master.txt").string(), gen, err), "loadFileList (gen)");
    runDataset("generated", gen);

    // ---- one big file (like the "single huge file" scenario) ----
    CHECK(generateDataset((dir / "one").string(), 1, 300, err), "generateDataset one");
    FileList one;
    CHECK(loadFileList((dir / "one" / "00_master.txt").string(), one, err), "loadFileList (one)");
    runDataset("single big file", one);

    // ---- pack / unpack ----
    const FileList back = unpackFileList(packFileList(gen).data(), packFileList(gen).size());
    CHECK(back.files.size() == gen.files.size() && back.totalSize == gen.totalSize, "file list pack/unpack");
    CHECK(back.files.back().path == gen.files.back().path, "file list pack/unpack path");

    Histogram h{{"alpha", 3}, {"beta", 5}, {"gamma", 1}}, h2;
    std::vector<char> buf;
    for (const auto& kv : h) appendPair(buf, kv.first, kv.second);
    mergeFromBuffer(buf.data(), buf.size(), h2);
    CHECK(h == h2, "histogram serialisation round trip");

    fs::remove_all(dir);
    std::printf(g_fail == 0 ? "\nALL TESTS PASSED\n" : "\n%d CHECK(S) FAILED\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
