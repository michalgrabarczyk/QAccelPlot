#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Install Windows Qt 6.11 despite aqtinstall 3.3's repository-path bug."""

from __future__ import annotations

import sys


def windows_qt_repository_path(version: str, arch: str) -> str:
    """Return the architecture-specific repository introduced for Qt 6.11."""
    version_parts = version.split(".")
    if len(version_parts) != 3 or not all(part.isdigit() for part in version_parts):
        raise ValueError(f"Expected a three-part Qt version, got {version!r}")
    if not arch.startswith("win64_"):
        raise ValueError(f"Expected a win64 Qt architecture, got {arch!r}")

    version_code = "".join(version_parts)
    repository_arch = arch[len("win64_") :]
    return f"qt{version_parts[0]}_{version_code}/qt{version_parts[0]}_{version_code}_{repository_arch}"


def main() -> int:
    from aqt import main as aqt_main
    from aqt.archives import QtArchives
    from aqt.metadata import Version

    original_get_archives = QtArchives._get_archives

    def get_archives_from_qt611_windows_layout(self: QtArchives) -> None:
        if (
            self.os_name == "windows"
            and self.target == "desktop"
            and Version("6.11.0") <= self.version < Version("6.12.0")
        ):
            repository = windows_qt_repository_path(str(self.version), self.arch)
            self._get_archives_base(repository, self._target_packages())
            return
        original_get_archives(self)

    QtArchives._get_archives = get_archives_from_qt611_windows_layout
    return aqt_main()


if __name__ == "__main__":
    sys.exit(main())
