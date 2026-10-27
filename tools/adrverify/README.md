# ADR behavior verifier

This tool is separate from `tools/umlverify/`. It runs the host C++ tests, executes a
small C++ probe against the booking classes, compares the observed values with the
reviewed fixed checks in `project/process/specs/adr-behavior-contract.json`, and reports
the Figure of Merit:

`passing fixed in-scope checks / total fixed in-scope checks * 100`

From the repository root, run:

```powershell
python tools\adrverify\verify.py
```

The script configures and builds `project/impl/` with CMake, runs CTest, then writes
`project/dashboard/data/latest-results.json` and appends the run to
`project/dashboard/data/verification-history.json`. CMake and a working C++20 compiler
must be available.

The probe observes selected deterministic scenarios, not every possible execution path.
The checks are explicit examples reviewed against the ADR; the verifier does not parse
English prose or claim formal proof. The separate host benchmark is informational and is
not a micro:bit performance measurement. Dashboard notifications, wall-clock scheduling,
and physical sensor behavior are outside this contract.
