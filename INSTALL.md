# Build & run on Windows (Visual Studio + MS-MPI)

## 1. Prerequisites
* Visual Studio with workload **Desktop development with C++** (in Russian UI: «Разработка классических приложений на C++»)
  and a **complete Windows 10/11 SDK** (this project is pinned to `10.0.26100.0`).
* **MS-MPI**: install both `msmpisetup.exe` (runtime, `mpiexec`) and `msmpisdk.msi` (headers/libs).
  Expected SDK location: `C:\Program Files (x86)\Microsoft SDKs\MPI`.

## 2. Build (recommended: the ready-made solution)
1. Open **`ParallelWordCount.sln`**. If Visual Studio offers to *retarget* / upgrade the solution, accept.
2. In the toolbar choose **Release** and **x64**  (Release matters for timings!).
3. *Build → Build Solution*. Outputs: `bin\Release\wc.exe` and `bin\Release\test_partition.exe`.

What is preconfigured (Project → Properties, x64):
| Setting | Value |
|---|---|
| General → Windows SDK Version | `10.0.26100.0` |
| C/C++ → Language Standard | ISO C++17 |
| C/C++ → OpenMP Support | Yes (`/openmp`) |
| C/C++ → Additional Include Directories | `$(ProjectDir)include; C:\Program Files (x86)\Microsoft SDKs\MPI\Include; ...\Include\x64` |
| Linker → Additional Library Directories | `C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64` |
| Linker → Additional Dependencies | `msmpi.lib` |

If your SDK folder has another version, change *General → Windows SDK Version* (list installed ones with
`dir "C:\Program Files (x86)\Windows Kits\10\Lib"`; the version must contain `um\x64\gdi32.lib`).
If MPI is installed elsewhere, edit the two MPI paths above.

## 3. Check it works
```
bin\Release\test_partition.exe                       -> ALL TESTS PASSED
bin\Release\wc.exe gen data\small 20 512
bin\Release\wc.exe serial data\small\00_master.txt
mpiexec -n 4 bin\Release\wc.exe mpi data\small\00_master.txt --threads 2
```
The `checksum` must be the same in all three modes. (On the first `mpiexec` run Windows Firewall may ask
for permission - allow it.)

## 4. Benchmarks and graphs
```
bin\Release\wc.exe gen data\bench 64 16384
powershell -ExecutionPolicy Bypass -File scripts\run_benchmarks.ps1
python scripts\plot_results.py
```
Tips: close other programs, plug in the laptop, use physical-core counts, ignore the first (cold-cache) run.
If ranks × threads > physical cores the results are not meaningful.

## Alternative: CMake
`CMakeLists.txt` is provided too (Visual Studio *Open Folder*, or `cmake -B build && cmake --build build`
on Linux). On Windows it needs a *complete* newest Windows SDK; if CMake fails on
`cannot open gdi32.lib`, that SDK version is broken - use the `.sln` above instead.

## Linux / cluster
`cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`, then
`mpirun -np 4 build/wc mpi data/bench/00_master.txt --threads 2`.
