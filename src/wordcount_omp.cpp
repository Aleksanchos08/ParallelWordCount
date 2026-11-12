#include "wordcount.h"
#include "tokenizer.h"
#include <omp.h>
#include <algorithm>
#include <functional>

namespace wc {
namespace {

struct Task {
    const Chunk* chunk;
    std::size_t  lo, hi;  // words starting in [lo,hi) of chunk->data
};



std::vector<Task> buildTasks(const std::vector<Chunk>& chunks) {
    std::vector<Task> tasks;
    for (const Chunk& c : chunks)
        for (std::size_t lo = c.begin; lo < c.end; lo += TASK_BYTES)
            tasks.push_back({&c, lo, std::min(lo + TASK_BYTES, c.end)});
    return tasks;
}



Histogram mergeByHash(std::vector<Histogram>& local, int T) {
    if (T == 1) return std::move(local[0]);

    std::hash<std::string> H;
    std::vector<Histogram> slices(T);
#pragma omp parallel for schedule(static) num_threads(T)
    for (int t = 0; t < T; ++t) {
        Histogram& out = slices[t];
        for (int s = 0; s < T; ++s)
            for (const auto& kv : local[s])
                if (H(kv.first) % static_cast<std::size_t>(T) == static_cast<std::size_t>(t))
                    out[kv.first] += kv.second;
    }

    Histogram result;
    std::size_t n = 0;
    for (const auto& s : slices) n += s.size();
    result.reserve(n);
    for (auto& s : slices)
        for (auto& kv : s) result.emplace(kv.first, kv.second);
    return result;
}

}

Histogram countOpenMP(const std::vector<Chunk>& chunks, int threads) {
    const int T = std::max(1, threads);
    const std::vector<Task> tasks = buildTasks(chunks);
    const int n = static_cast<int>(tasks.size());

    std::vector<Histogram> local(T);  // one private histogram per thread

#pragma omp parallel num_threads(T)
    {
        Histogram& mine = local[omp_get_thread_num()];
#pragma omp for schedule(dynamic)
        for (int i = 0; i < n; ++i) {
            const Task& t = tasks[i];
            countRange(t.chunk->data.data(), t.chunk->data.size(), t.lo, t.hi, mine);
        }
    }
    return mergeByHash(local, T);
}

}
