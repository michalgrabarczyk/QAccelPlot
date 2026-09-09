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

A pre-1.0 release may contain new features, fixes, and breaking changes. The
patch component remains zero, and QAccelPlot does not normally maintain older
pre-1.0 release branches. Applications should upgrade to the latest release
and review the changelog for breaking changes and migration instructions.

## Starting with 1.0

Starting with version 1.0, QAccelPlot follows Semantic Versioning:

- A patch release contains backward-compatible fixes.
- A minor release adds backward-compatible functionality.
- A major release may make breaking changes.

## Changelog

`CHANGELOG.md` in the repository root is the source of truth for curated
release notes. User-visible changes are collected under `[Unreleased]` during
development and moved to a dated version section when released. GitHub Release
notes are taken from the matching version section.

## Pinning a dependency

Production applications should depend on a release tag rather than `main`:

```cmake
FetchContent_Declare(
    QAccelPlot
    GIT_REPOSITORY https://github.com/michalgrabarczyk/QAccelPlot.git
    GIT_TAG v0.1.0
)
```

The `main` branch contains development work for the next release and may change
without notice.

## Documentation versions

The default documentation site describes the latest stable release. Each
release also has immutable documentation under its exact version path, for
example `/0.2.0/`, and `/latest/` points to the same current stable content.

Documentation generated from `main` is published separately under `/main/`
and is visibly marked as unreleased. It may describe features that are not
present in a tagged release. Use the documentation matching the QAccelPlot
version selected by your application.

The documentation site also includes the repository changelog so release and
development documentation show the release history that belongs to that source
revision.

## Release downloads

GitHub Releases provide the standard source `.zip` and `.tar.gz` archives for
each release tag. QAccelPlot does not normally publish prebuilt library
packages. The default build enables an optional optimization that uses Qt Quick
private APIs; binaries built with this optimization must match the
application's Qt version and toolchain. Configure with
`-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` to use only public Qt APIs. Building
QAccelPlot from source avoids maintaining a matrix of compatible prebuilt
binaries.

## Qt compatibility

The default optimized build uses one isolated Qt Quick private API, so a
prebuilt library made in that mode must use the same Qt version and toolchain
as the application. No private type is exposed in QAccelPlot's public ABI, but
the implementation dependency must still match. This is handled automatically
when QAccelPlot is built from source with the application.

A build configured with `-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` depends only on
public Qt API and does not link QuickPrivate. Rebuild QAccelPlot whenever the Qt
version or compiler changes regardless of mode.
