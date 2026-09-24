"""Create a Shields.io endpoint badge from a gcovr JSON summary."""

import json
import os
import sys
from pathlib import Path


def main() -> None:
    summary_path = Path(sys.argv[1])
    badge_path = Path(sys.argv[2])
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    covered = summary["line_covered"]
    total = summary["line_total"]
    if total <= 0 or not summary["files"] or not 0 <= covered <= total:
        raise ValueError("gcovr did not report valid library line coverage")

    percent = 100 * covered / total
    if percent >= 80:
        color = "brightgreen"
    elif percent >= 60:
        color = "yellow"
    else:
        color = "red"
    badge = {
        "schemaVersion": 1,
        "label": "Code coverage",
        "message": f"{percent:.1f}%",
        "color": color,
    }
    badge_path.write_text(json.dumps(badge) + "\n", encoding="utf-8")

    result = f"C++ line coverage: {covered}/{total} ({percent:.1f}%)"
    print(result)
    step_summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if step_summary:
        with open(step_summary, "a", encoding="utf-8") as output:
            output.write(f"### QAccelPlot library coverage\n\n{result}\n")


if __name__ == "__main__":
    main()
