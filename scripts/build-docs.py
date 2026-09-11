#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0

"""Build or serve the combined MkDocs and Doxygen documentation site."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


def documentation_environment() -> dict[str, str]:
    environment = os.environ.copy()
    docs_version = environment.setdefault("QACCELPLOT_DOCS_VERSION", "main")
    environment.setdefault("QACCELPLOT_DOCS_CHANNEL", "development")

    default_title = "QAccelPlot Documentation"
    if docs_version.replace(".", "").isdigit() and docs_version.count(".") == 2:
        default_title = f"QAccelPlot v{docs_version} Documentation"
    environment.setdefault("QACCELPLOT_DOCS_TITLE", default_title)
    return environment


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("build", "serve"), nargs="?", default="build")
    arguments = parser.parse_args()

    repository_root = Path(__file__).resolve().parent.parent
    source_changelog = repository_root / "CHANGELOG.md"
    generated_changelog = repository_root / "docs" / "guide" / "changelog.md"
    if generated_changelog.exists():
        if generated_changelog.read_bytes() != source_changelog.read_bytes():
            raise RuntimeError(f"Refusing to overwrite existing file: {generated_changelog}")
        generated_changelog.unlink()

    shutil.copyfile(source_changelog, generated_changelog)
    try:
        command = [sys.executable, "-m", "mkdocs", arguments.command]
        if arguments.command == "build":
            command.append("--strict")
        subprocess.run(
            command,
            check=True,
            cwd=repository_root,
            env=documentation_environment(),
        )

        if arguments.command == "build":
            site_directory = repository_root / "build" / "site"
            shutil.copyfile(repository_root / "llms.txt", site_directory / "llms.txt")
            subprocess.run(
                [
                    sys.executable,
                    str(repository_root / "scripts" / "generate-llms-full.py"),
                    "--output",
                    str(site_directory / "llms-full.txt"),
                ],
                check=True,
                cwd=repository_root,
            )
    finally:
        generated_changelog.unlink(missing_ok=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
