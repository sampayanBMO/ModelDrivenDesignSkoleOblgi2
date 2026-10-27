#!/usr/bin/env python3
"""Run host tests, extract observable C++ behavior, and score the ADR contract."""

import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
IMPL = ROOT / "project" / "impl"
BUILD = ROOT / "project" / "build-adrverify"
CONTRACT_PATH = ROOT / "project" / "process" / "specs" / "adr-behavior-contract.json"
DATA_DIR = ROOT / "project" / "dashboard" / "data"
HISTORY_PATH = DATA_DIR / "verification-history.json"
LATEST_PATH = DATA_DIR / "latest-results.json"


def run(command):
    return subprocess.run(
        command,
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    ).stdout


def find_executable(stem):
    suffixes = (".exe", "") if sys.platform == "win32" else ("",)
    for suffix in suffixes:
        matches = sorted(BUILD.rglob(stem + suffix))
        if matches:
            return matches[0]
    raise FileNotFoundError(f"Could not find built executable {stem!r} under {BUILD}")


def value_at_path(model, dotted_path):
    value = model
    for component in dotted_path.split("."):
        if not isinstance(value, dict) or component not in value:
            return None, False
        value = value[component]
    return value, True


def main():
    run(["cmake", "-S", str(IMPL), "-B", str(BUILD)])
    run(["cmake", "--build", str(BUILD), "--parallel"])

    test_result = subprocess.run(
        ["ctest", "--test-dir", str(BUILD), "--output-on-failure"],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    test_summary = re.search(
        r"(\d+)% tests passed, (\d+) tests failed out of (\d+)",
        test_result.stdout + test_result.stderr,
    )
    if test_summary is None:
        raise RuntimeError("CTest did not report a recognizable test summary.")
    passed_tests, failed_tests, total_tests = (
        int(test_summary.group(3)) - int(test_summary.group(2)),
        int(test_summary.group(2)),
        int(test_summary.group(3)),
    )

    model = json.loads(run([str(find_executable("booking_model"))]))
    benchmark = json.loads(run([str(find_executable("booking_benchmark"))]))
    contract = json.loads(CONTRACT_PATH.read_text(encoding="utf-8"))
    checks = []
    for item in contract["checks"]:
        actual, found = value_at_path(model, item["path"])
        expected = item["equals"]
        passed = found and type(actual) is type(expected) and actual == expected
        checks.append(
            {
                "id": item["id"],
                "description": item["description"],
                "passed": passed,
                "expected": expected,
                "actual": actual if found else None,
            }
        )

    passed_checks = sum(check["passed"] for check in checks)
    total_checks = len(checks)
    result = {
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "contract": {"name": contract["name"], "version": contract["version"]},
        "verification": {
            "passed": passed_checks,
            "total": total_checks,
            "fom_percent": round(100 * passed_checks / total_checks, 2),
            "checks": checks,
        },
        "unit_tests": {
            "passed": test_result.returncode == 0,
            "passed_count": passed_tests,
            "failed_count": failed_tests,
            "total_count": total_tests,
            "output": test_result.stdout.strip(),
        },
        "benchmark": benchmark,
    }

    DATA_DIR.mkdir(parents=True, exist_ok=True)
    if HISTORY_PATH.exists():
        history = json.loads(HISTORY_PATH.read_text(encoding="utf-8"))
    else:
        history = {"format": "lab-booking-verification-history", "runs": []}
    result["iteration"] = len(history["runs"]) + 1
    history["runs"].append(result)
    HISTORY_PATH.write_text(json.dumps(history, indent=2) + "\n", encoding="utf-8")
    LATEST_PATH.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    print(
        f"ADR behavior FoM: {passed_checks}/{total_checks} "
        f"({result['verification']['fom_percent']:.2f}%)"
    )
    print(f"Host unit tests: {passed_tests}/{total_tests} passed")
    print(
        "Host benchmark: "
        f"{benchmark['microseconds_per_lifecycle']:.3f} microseconds/lifecycle "
        f"({benchmark['iterations_per_sample']} iterations x {benchmark['samples']} samples)"
    )
    print(f"Saved iteration {result['iteration']} to {HISTORY_PATH}")
    return 0 if all(check["passed"] for check in checks) and test_result.returncode == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
