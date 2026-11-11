# Algorithm notes and experiment plan

## Byte-balanced distribution (example: P = 3, files of 50, 10 and 40 bytes, total = 100)
```
global bytes:  0        50 60                100
               |--file A--|f B|-----file C-----|
rank 0:        [0 ........ 33)            -> A[0,33)
rank 1:                    [33 ....... 66) -> A[33,50) + B[0,10) + C[0,6)
rank 2:                                [66 ... 100) -> C[6,40)
```
Splitting by *file count* would give rank 0 a lot more work than rank 2 — the reason for splitting by bytes.

## Border rule
A word belongs to the range containing its **first byte**.
```
...the quick|brown fox...      range boundary '|' falls inside "quick"? -> no, between words
...the qui|ck brown fox...     left range owns "quick" (reads tail), right range skips "ck"
```
Reading layout per segment: `[1 byte before][segment][<= 4096 bytes after]`.

## Complexity
Counting is O(N) in the number of bytes. With P·T workers the ideal time is O(N/(P·T)).
Overheads: reading (disk bandwidth is shared!), merge O(V) per level (V = distinct words),
communication O(V) bytes (gather: concentrated in rank 0; alltoall: spread over all ranks).

## Experiments (what to show in the presentation)
| Experiment | How | What to show |
|---|---|---|
| Correctness | checksum equal in serial / omp / mpi, `test_partition` passes | table |
| Strong scaling | fixed ~1 GB, cores 1,2,4,8,… | speedup, efficiency (`speedup.png`, `efficiency.png`) |
| Weak scaling | data grows with cores: `gen data\wN <files = 8·N> 4096` | time should stay flat |
| Hybrid trade-off | same cores, e.g. 8×1, 4×2, 2×4, 1×8 | which split is best and why |
| Reduce strategy | `--reduce gather` vs `alltoall` | `reduce_s` column; gap grows with ranks and vocabulary |
| Time breakdown | `time_breakdown.png` | read vs count vs reduce, Amdahl / I/O bound |
| Vocabulary size | text with few vs. many distinct words | reduce cost depends on V |

Expect: `count_s` scales almost linearly; `read_s` saturates (disk/memory bandwidth); `reduce_s` is small for
natural text (small V) but grows for gather; on one machine MPI ranks are slower to start than threads, so
OpenMP usually wins on a single node and the hybrid shines on clusters / NUMA machines.
Explain observed deviations with Amdahl's law and the phase breakdown.
