"""Require a complete passing result for every production screen suite."""
from pathlib import Path
import hashlib
import json
import re

here = Path(__file__).resolve().parent
repo = here.parents[3]
runs = json.loads((here / "screen-runs.json").read_text())
expected = sorted(path.stem for path in (repo / "gui/tests/qml").glob("tst_*.qml"))
assert sorted(row["Suite"] for row in runs) == expected
totals = [0, 0, 0, 0]
for row in runs:
    assert row["ExitCode"] == 0, row
    report = (here / f"screen-{row['Suite']}.txt").read_text()
    result = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped, (\d+) blacklisted", report)
    assert result and "FAIL!" not in report, row["Suite"]
    counts = list(map(int, result.groups()))
    assert counts[1:] == [0, 0, 0], row["Suite"]
    totals = [total + value for total, value in zip(totals, counts)]
binary = Path(r"C:/Users/david/.cache/chiaki-ui-native/psn-dialog-final-bin/chiaki-console-screen-tests.exe")
result = {"suites": len(runs), "passed": totals[0], "failed": totals[1], "skipped": totals[2], "blacklisted": totals[3], "seconds": round(sum(row["Seconds"] for row in runs), 2), "screenExecutableSha256": hashlib.sha256(binary.read_bytes()).hexdigest(), "qtVersion": "6.11.2", "compiler": "GCC 16.2.0 UCRT", "productionQmlWithSuppliedState": True}
(here / "screen-summary.json").write_text(json.dumps(result, indent=2))
print(json.dumps(result))
