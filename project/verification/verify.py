#!/usr/bin/env python3
"""Deterministic ADR-derived behavioral verification; independent of UML verification."""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

CHECKLIST = [
    "new_booking_is_pending",
    "student_can_view_requested_booking",
    "admin_can_approve",
    "approved_booking_activates_equipment",
    "ending_booking_releases_equipment",
    "pending_booking_can_be_cancelled",
    "overlapping_booking_is_rejected",
    "touching_booking_boundary_is_accepted",
]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", type=Path, default=Path("project/impl/build/Debug/verification_probe.exe"))
    parser.add_argument("--output", type=Path, default=Path("project/verification/verification-results.json"))
    parser.add_argument("--label", default="unlabelled")
    args = parser.parse_args()
    completed = subprocess.run([str(args.probe)], capture_output=True, text=True, check=False)
    observed = {}
    for line in completed.stdout.splitlines():
        try:
            item = json.loads(line)
        except json.JSONDecodeError:
            continue
        if "check" in item:
            observed[item["check"]] = bool(item["observed"])
    checks = [
        {"id": check, "description": check.replace("_", " "), "passed": observed.get(check, False)}
        for check in CHECKLIST
    ]
    passed = sum(check["passed"] for check in checks)
    total = len(checks)
    report = {
        "format": "lab-booking-adr-verification",
        "version": 1,
        "label": args.label,
        "checklistVersion": 1,
        "passed": passed,
        "total": total,
        "fomPercent": round(passed / total * 100, 2),
        "probeExitCode": completed.returncode,
        "checks": checks,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": passed, "total": total, "fomPercent": report["fomPercent"]}, indent=2))
    return 0 if passed == total and completed.returncode == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
