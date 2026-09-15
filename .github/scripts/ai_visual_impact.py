#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Classify a git diff for paid visual-inspection routing."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "examples" / "test" / "visual"))

from visual_scenarios import DEFAULT_CONTRACTS_DIR, Scenario, discover_scenarios  # noqa: E402


SHARED_VISUAL_PREFIXES = (
    "QAccelPlot/",
    "qml/",
    "examples/common/",
)

SHARED_VISUAL_FILES = {
    ".github/scripts/ai_visual_impact.py",
    ".github/workflows/ai-regression.yml",
    "CMakeLists.txt",
    "cmake/QAccelPlotTestHelpers.cmake",
    "examples/CMakeLists.txt",
    "examples/test/CMakeLists.txt",
    "examples/test/visual/ai_visual_acceptance.py",
    "examples/test/visual/ai_visual_matrix.py",
    "examples/test/visual/visual_scenarios.py",
}


def ai_scenarios(contracts_dir: Path) -> dict[str, Scenario]:
    return {identity: scenario for identity, scenario in discover_scenarios(contracts_dir).items() if scenario.ai_inspection}


def classify_paths(paths: list[str], contracts_dir: Path = DEFAULT_CONTRACTS_DIR) -> dict[str, object]:
    """Selects the scenarios whose screenshots a change can affect.

    A contract change selects its scenario. A change to an example file listed in a
    contract's ``sources`` selects only the scenarios listing it; any other example file,
    such as ``main.qml`` or C++ sources, selects every scenario of that example.
    """
    scenarios = ai_scenarios(contracts_dir)
    examples = {scenario.example for scenario in scenarios.values()}
    selected: set[str] = set()
    shared_change = False

    for raw_path in paths:
        path = raw_path.strip().replace("\\", "/")
        if not path:
            continue

        contract_match = re.fullmatch(r"examples/test/visual/contracts/([^/]+)/([^/]+)\.json", path)
        if contract_match:
            identity = f"{contract_match.group(1)}/{contract_match.group(2)}"
            if identity in scenarios:
                selected.add(identity)
            continue

        example_match = re.match(r"examples/([^/]+)/(.+)", path)
        if example_match and example_match.group(1) in examples:
            example, relative_path = example_match.groups()
            example_scenarios = [scenario for scenario in scenarios.values() if scenario.example == example]
            page_scenarios = [scenario for scenario in example_scenarios if relative_path in scenario.sources]
            selected.update(scenario.identity for scenario in (page_scenarios or example_scenarios))
            continue

        if path in SHARED_VISUAL_FILES or path.startswith(SHARED_VISUAL_PREFIXES):
            shared_change = True

    if shared_change:
        scope = "all"
        selected = set(scenarios)
    elif selected:
        scope = "selected"
    else:
        scope = "none"

    return {
        "scope": scope,
        "scenarios": sorted(selected),
        "changed_count": len([path for path in paths if path.strip()]),
    }


def git_changed_paths(base: str, head: str) -> list[str]:
    completed = subprocess.run(
        ["git", "diff", "--name-only", "--diff-filter=ACMRT", f"{base}...{head}"],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.splitlines()


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Classify changes that can affect example screenshots.")
    parser.add_argument("--base", required=True, help="Base git ref or commit")
    parser.add_argument("--head", default="HEAD", help="Head git ref or commit")
    parser.add_argument("--github-output", type=Path, help="Append outputs for a GitHub Actions step")
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    result = classify_paths(git_changed_paths(args.base, args.head))
    scenarios = ",".join(result["scenarios"])
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as output:
            output.write(f"scope={result['scope']}\n")
            output.write(f"scenarios={scenarios}\n")
            output.write(f"changed_count={result['changed_count']}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
