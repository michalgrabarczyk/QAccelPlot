---
description: "QAccelPlot version numbering, compatibility guarantees, release tags, documentation versions, and dependency pinning."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: MIT
-->

# Versioning and releases

QAccelPlot uses release numbers in the form `MAJOR.MINOR.PATCH` and publishes
releases under matching Git tags such as `v0.1.0`.

## Compatibility

QAccelPlot follows [Semantic Versioning](https://semver.org/):

- A patch release contains backward-compatible fixes.
- A minor release adds backward-compatible functionality.
- A major release may make breaking changes.

## Changelog

The changelog has one dated section per release. Changes not yet released are
under `[Unreleased]`.

## Pinning a dependency

Pin a release tag rather than `main`:

```cmake
FetchContent_Declare(
    QAccelPlot
    GIT_REPOSITORY https://github.com/michalgrabarczyk/QAccelPlot.git
    GIT_TAG v0.5.0
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
must use the same Qt version and toolchain as the application. Rebuild
QAccelPlot whenever the Qt version or compiler changes, in either mode.
