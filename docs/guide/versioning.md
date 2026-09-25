---
description: "QAccelPlot version numbering, compatibility guarantees, release tags, documentation versions, and dependency pinning."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Versioning and releases

QAccelPlot uses release numbers in the form `MAJOR.MINOR.PATCH` and publishes
releases under matching Git tags such as `v0.1.0`.

## Before 1.0

Before version 1.0, every regular release increments the minor version:

```text
0.1.0 → 0.2.0 → 0.3.0
```

A pre-1.0 release may contain breaking changes. The patch component stays zero,
and older pre-1.0 releases are not maintained. Check the changelog for breaking
changes and migration notes when upgrading.

## Starting with 1.0

Starting with version 1.0, QAccelPlot follows Semantic Versioning:

- A patch release contains backward-compatible fixes.
- A minor release adds backward-compatible functionality.
- A major release may make breaking changes.

## Changelog

`CHANGELOG.md` is the source of release notes. Changes collect under
`[Unreleased]` and move to a dated version section on release; GitHub Release
notes are taken from that section.

## Pinning a dependency

Pin a release tag rather than `main`:

```cmake
FetchContent_Declare(
    QAccelPlot
    GIT_REPOSITORY https://github.com/michalgrabarczyk/QAccelPlot.git
    GIT_TAG v0.3.0
)
```

`main` holds unreleased work and may change without notice.

## Documentation versions

| Path | Content |
| --- | --- |
| `/` and `/latest/` | Latest stable release |
| `/<version>/`, e.g. `/0.2.0/` | That release, unchanged after publishing |
| `/main/` | Unreleased `main`, marked as such |

Use the documentation matching the version your application pins. Each version
includes the changelog from the same source revision.

## Release downloads and Qt compatibility

GitHub Releases provide source `.zip` and `.tar.gz` archives only; build
QAccelPlot from source with your application.

A build with `QACCELPLOT_USE_QT_PRIVATE_API=ON` depends on a Qt private API and
must use the same Qt version and toolchain as the application. No private type
appears in QAccelPlot's public ABI. Rebuild QAccelPlot whenever the Qt version
or compiler changes, in either mode.
