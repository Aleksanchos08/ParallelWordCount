<#
  Runs every benchmark configuration $Repeats times and appends rows to $Csv.
  Example:
    .\scripts\run_benchmarks.ps1 -Exe .\bin\Release\wc.exe -Master data\bench\00_master.txt
  Prepare data first:   wc.exe gen data\bench 64 16384     (64 files, ~16 MB each, ~1 GB total)
  Adjust the arrays below to the number of physical cores of your machine.
#>
param(
  [string]$Exe      = ".\bin\Release\wc.exe",
  [string]$Master   = "data\bench\00_master.txt",
  [string]$Csv      = "tests-results\results.csv",
  [int]$Repeats     = 5,
  [int[]]$Threads   = @(1, 2, 4, 8),                 # OpenMP only
  [int[]]$Ranks     = @(1, 2, 4, 8),                 # pure MPI (1 thread per rank)
  [string[]]$Hybrid = @("2x4", "4x2")                # "ranks x threads" with the same total core count
)

function Repeat([scriptblock]$cmd) { for ($i = 0; $i -lt $Repeats; $i++) { & $cmd | Out-Null } }

Write-Host "== serial"
Repeat { & $Exe serial $Master --csv $Csv }

foreach ($t in $Threads) {
  Write-Host "== omp x$t"
  Repeat { & $Exe omp $Master --threads $t --csv $Csv }
}

foreach ($red in @("alltoall", "gather")) {
  foreach ($r in $Ranks) {
    Write-Host "== mpi $r ranks x 1 thread, reduce=$red"
    Repeat { mpiexec -n $r $Exe mpi $Master --threads 1 --reduce $red --csv $Csv }
  }
}

foreach ($h in $Hybrid) {
  $r, $t = $h -split "x"
  Write-Host "== hybrid $r ranks x $t threads"
  Repeat { mpiexec -n $r $Exe mpi $Master --threads $t --reduce alltoall --csv $Csv }
}
Write-Host "Done -> $Csv"
