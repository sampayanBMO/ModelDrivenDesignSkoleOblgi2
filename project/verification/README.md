# Project verification tools

These tools are separate from `tools/verify.py`, which verifies UML alignment.

## Run

From the repository root:

```powershell
cmake -S ModelDrivenDesignSkoleOblgi2/project/impl -B ModelDrivenDesignSkoleOblgi2/project/impl/build
cmake --build ModelDrivenDesignSkoleOblgi2/project/impl/build --parallel
python ModelDrivenDesignSkoleOblgi2/project/verification/test_runner.py `
  --build-dir ModelDrivenDesignSkoleOblgi2/project/impl/build
python ModelDrivenDesignSkoleOblgi2/project/verification/verify.py `
  --probe ModelDrivenDesignSkoleOblgi2/project/impl/build/Debug/verification_probe.exe
ModelDrivenDesignSkoleOblgi2/project/impl/build/Debug/benchmark_runner.exe `
  ModelDrivenDesignSkoleOblgi2/project/verification/benchmark-results.json
```

The test runner executes CTest and writes one JSON record per run. The benchmark
replays real domain operations and records every event separately. The verifier
executes the C++ behavior probe against a fixed ADR-derived checklist. Its FoM is
`passed checks / total fixed checks * 100`; it exits non-zero if any check fails.

The dashboard reads the generated JSON files from this folder when served over
HTTP. It also stores user-created bookings and event history in `localStorage`.

## Figure of Merit

`verification-history.json` contains labelled results from actual tool runs. The
checklist version must remain the same for points to be comparable. A changed
checklist starts a new history series rather than changing old points.
