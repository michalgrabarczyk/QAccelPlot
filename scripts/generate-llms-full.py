#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# generate-llms-full.py — Flatten all QAccelPlot documentation into a single
# llms-full.txt file for AI agent ingestion.
#
# Usage:
#   python scripts/generate-llms-full.py
#
# Output:
#   llms-full.txt in the repository root (or build/site/llms-full.txt when
#   called with --output build/site/llms-full.txt).

import argparse
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
GUIDE_DIR = REPO_ROOT / "docs" / "guide"
SITE_URL = "https://michalgrabarczyk.github.io/QAccelPlot/"

# Ordered list of documentation files to include.
DOC_FILES = [
    GUIDE_DIR / "index.md",
    GUIDE_DIR / "getting-started.md",
    GUIDE_DIR / "concepts.md",
    GUIDE_DIR / "performance.md",
    GUIDE_DIR / "faq.md",
    GUIDE_DIR / "troubleshooting.md",
    GUIDE_DIR / "cookbook" / "index.md",
    GUIDE_DIR / "cookbook" / "basic-plot.md",
    GUIDE_DIR / "cookbook" / "realtime.md",
    GUIDE_DIR / "cookbook" / "axis-formats.md",
    GUIDE_DIR / "cookbook" / "multiple-axes.md",
    GUIDE_DIR / "cookbook" / "styling.md",
    GUIDE_DIR / "cookbook" / "annotations.md",
    GUIDE_DIR / "cookbook" / "background-data.md",
    GUIDE_DIR / "api.md",
]

# Pattern to strip SPDX comment blocks and YAML front matter.
SPDX_COMMENT = re.compile(r"^<!--\s*\n(?:.*?\n)*?-->\s*\n", re.MULTILINE)
YAML_FRONT_MATTER = re.compile(r"^---\s*\n(?:.*?\n)*?---\s*\n", re.MULTILINE)
RELATIVE_MARKDOWN_LINK = re.compile(
    r"(?P<prefix>\]\()"
    r"(?P<path>(?:(?:\.\.?|[A-Za-z0-9_.-]+)/)*[A-Za-z0-9_.-]+\.md)"
    r"(?P<fragment>#[^)\s]+)?\)"
)


def clean_content(text: str) -> str:
    """Remove SPDX comment blocks and YAML front matter from markdown."""
    text = SPDX_COMMENT.sub("", text)
    text = YAML_FRONT_MATTER.sub("", text)
    return text.strip()


def rewrite_relative_links(text: str, doc_path: Path) -> str:
    """Rewrite source-relative Markdown links to deployed documentation URLs."""
    guide_dir = GUIDE_DIR.resolve()

    def replace_link(match: re.Match) -> str:
        target = (doc_path.parent / match.group("path")).resolve()
        try:
            relative_target = target.relative_to(guide_dir)
        except ValueError:
            return match.group(0)

        route = relative_target.with_suffix("")
        if route.name == "index":
            route = route.parent

        route_path = route.as_posix().strip("/")
        url = SITE_URL if not route_path else f"{SITE_URL}{route_path}/"
        fragment = match.group("fragment") or ""
        return f'{match.group("prefix")}{url}{fragment})'

    return RELATIVE_MARKDOWN_LINK.sub(replace_link, text)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate llms-full.txt from QAccelPlot documentation."
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO_ROOT / "llms-full.txt",
        help="Output file path (default: repo root llms-full.txt)",
    )
    args = parser.parse_args()

    parts: list[str] = []

    # Header
    parts.append("# QAccelPlot — Complete Documentation\n")
    parts.append(
        "> This file is auto-generated from the QAccelPlot documentation "
        "source.\n> It provides the complete library documentation in a single "
        "file for AI agent ingestion.\n"
    )

    for doc_path in DOC_FILES:
        if not doc_path.exists():
            print(f"  WARNING: {doc_path} not found, skipping")
            continue

        content = doc_path.read_text(encoding="utf-8")
        content = clean_content(content)
        content = rewrite_relative_links(content, doc_path)

        # Add a separator between sections
        rel = doc_path.relative_to(GUIDE_DIR).as_posix()
        parts.append(f"\n---\n\n<!-- source: docs/guide/{rel} -->\n\n{content}\n")

    output = "\n".join(parts)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(output, encoding="utf-8")
    print(f"Generated {args.output} ({len(output):,} bytes)")


if __name__ == "__main__":
    main()
