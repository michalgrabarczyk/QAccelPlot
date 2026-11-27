# Releases

`CHANGELOG.md` is the source of truth for release notes. Before creating a
release tag:

1. Update the top-level CMake project version to the release version.
2. Move the relevant changelog entries from `[Unreleased]` into a dated section
   for that exact version, keeping `[Unreleased]` for future work.
3. Update migration or compatibility documentation for breaking changes.
4. Merge the release change into `main` and ensure normal CI passes.

Create and push the matching tag from `main`, for example:

```sh
git tag v0.2.0
git push origin v0.2.0
```

The release workflow requires the tag to match the CMake version and a
non-empty changelog section. Before 1.0, releases must increment `MINOR` and
keep `PATCH` at zero. The tagged commit must be reachable from `main`.

Published release tags are immutable. Never move or reuse a tag after its
GitHub Release has been created.
