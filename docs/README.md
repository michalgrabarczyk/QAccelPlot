<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Building the documentation

The documentation site combines an authored MkDocs guide with API reference
pages extracted by Doxygen and rendered by MkDoxy. GitHub Actions publishes the
result as one MkDocs Material site with shared navigation, styling, and search.

## Complete site

Install Python 3.9 or newer and the documentation dependencies, then run a
strict build from the repository root:

```sh
python -m pip install -r docs/requirements.txt
python scripts/build-docs.py build
```

Use `python scripts/build-docs.py serve` for a local development server. The
authored guide source is in `docs/guide`; MkDoxy generates the API pages during
the same build, and the complete site is written to `build/site`.

The CMake documentation target builds the same combined site when its selected
Python environment has the packages from `docs/requirements.txt` installed:

```sh
cmake -S . -B build-docs -DQACCELPLOT_BUILD_DOCS=ON
cmake --build build-docs --target qaccelplot_docs
```

This route configures the complete QAccelPlot build and therefore requires the
normal Qt development dependencies. Doxygen must also be installed and
available to CMake. The API configuration lives in `mkdocs.yml` under the
`mkdoxy` plugin.
