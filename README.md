# Parallel Word Count — MPI + OpenMP (C++17)

Final project: *Parallelization of Algorithms* by Alexandr Shashkov and Kira Viryasova MCS-2502.
Counts word frequencies over a **large set of text files** with three implementations that share
the same counting kernel, so results are directly comparable:

| mode     | what it is                                                             |
|----------|------------------------------------------------------------------------|
| `serial` | single-thread baseline (reference for correctness and speedup)         |
| `omp`    | OpenMP only, one process                                               |
| `mpi`    | **hybrid**: MPI processes × OpenMP threads, parallel disk reads        |

## Layout
```
include/   headers (one per module, documented)
src/       implementation + main.cpp
tests/     test_partition.cpp  - correctness tests, no MPI needed
scripts/   run_benchmarks.ps1  - runs all configurations -> CSV
           plot_results.py     - speedup / efficiency / time-breakdown graphs
data/      generated input (git-ignored)      logs/  per-rank logs (--log)
tests-results/  CSV + graphs                  doc/   ALGORITHM.md, notes
```

## Input
A **master list**: a text file with one path per line (relative paths are resolved against the
list's folder; `#` starts a comment). Files may have any sizes. Generate a data set:

```
wc gen data\bench 64 16384        # 64 files, avg 16 MB (sizes vary 0.25x..1.75x) -> ~1 GB
```

## Run
```
wc serial data\bench\00_master.txt
wc omp    data\bench\00_master.txt --threads 8
mpiexec -n 4 wc mpi data\bench\00_master.txt --threads 2 --reduce alltoall --csv tests-results\results.csv
```
Options: `--threads N`, `--reduce gather|alltoall`, `--topk K`, `--csv FILE`, `--log`.
Output: word count, distinct words, **checksum** (must be identical for every mode and
configuration), phase times (setup / read / count / reduce / total) and the top-K words.

## Algorithm in short
1. **Setup.** Rank 0 reads the master list and `stat`s the files; the list is `MPI_Bcast` to all.
2. **Byte-balanced partition.** All files are viewed as one stream of `total` bytes; rank *r* of *P* takes
   bytes `[total·r/P, total·(r+1)/P)`, i.e. the same amount (±1 byte) regardless of file sizes.
   A rank may get part of one file, several files, or begin in the middle of a file.
   Every rank computes its own segments locally — no scatter.
3. **Parallel read.** Each rank reads only its own segments.
4. **Word borders.** *A word belongs to the range in which it starts* (`countRange`). A range that begins
   inside a word skips it; a range that ends inside a word finishes it (reads a small tail). Used identically
   between ranks and between OpenMP tasks, so no word is lost or double counted.
5. **OpenMP map.** Data is cut into 256 KB tasks, scheduled dynamically; each thread fills a private hash map
   (no locks). The private maps are merged in parallel by hash partition (thread *t* owns `hash % T == t`),
   no `critical` section.
6. **MPI reduce** (selectable):
   * `gather`   — `MPI_Gatherv` to rank 0, which merges everything (baseline, rank 0 is a bottleneck);
   * `alltoall` — `MPI_Alltoallv` sends every word to rank `hash(word) % P`; each rank merges only its slice;
     afterwards only 3 numbers + top-K candidates are gathered. Scales with the vocabulary size.

MPI features used: `Bcast, Gather, Gatherv, Alltoall, Alltoallv, Reduce, Allreduce, Barrier`
(initialised with `MPI_THREAD_FUNNELED`).

## Tests (no MPI required)
```
test_partition.exe
```
Simulates 1…64 ranks one after another on nasty files (empty, no trailing newline, 3000-char words, UTF-8,
whitespace only, one byte) and on generated data, and checks: full coverage, ±1 byte balance, merged result ==
serial result, OpenMP == serial, serialisation round-trip.

## Limitations
* Word = `[A-Za-z0-9]` or any byte ≥128; only ASCII letters are lower-cased (Cyrillic case is not folded).
* Words longer than 4096 bytes are truncated at a rank border (`MAX_WORD_BYTES`).
* A rank's data and per-rank message must be < 2 GB (MPI `int` counts) — use more ranks for huge inputs.

See `INSTALL.md` for building in Visual Studio (open `ParallelWordCount.sln`) and `doc/ALGORITHM.md` for the experiment plan.
