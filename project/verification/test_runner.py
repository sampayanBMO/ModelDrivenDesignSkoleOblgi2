#!/usr/bin/env python3
"""Run the project's CTest suite and write a presentation-friendly JSON report."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, default=Path("project/impl/build"))
    parser.add_argument("--config", default="Debug")
    parser.add_argument("--output", type=Path, default=Path("project/verification/unit-test-results.json"))
    args = parser.parse_args()

    command = ["ctest", "--test-dir", str(args.build_dir), "-C", args.config, "--output-on-failure"]
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    passed = completed.returncode == 0
    result = {
        "format": "lab-booking-unit-tests",
        "version": 1,
        "command": " ".join(command),
        "passed": passed,
        "exitCode": completed.returncode,
        "output": completed.stdout + completed.stderr,
        "summary": {
            "passed": None,
            "failed": None,
            "total": None,
        },
    }
    match = re.search(r"(\d+)% tests passed(?:, (\d+) tests failed)? out of (\d+)", result["output"])
    if match:
        failed = int(match.group(2) or 0)
        total = int(match.group(3))
        result["summary"]["passed"] = total - failed
        result["summary"]["total"] = total
        result["summary"]["failed"] = failed

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result["summary"], indent=2))
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
