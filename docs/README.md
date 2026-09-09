<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Building the documentation

The documentation site combines an authored MkDocs guide with a Doxygen API
reference. GitHub Actions publishes both to GitHub Pages.

## User guide

Install the Python dependencies and run a strict build from the repository
root:

```sh
python -m pip install -r docs/requirements.txt
python -m mkdocs build --strict
```

Use `python -m mkdocs serve` for a local development server. The guide source
is in `docs/guide`, and the generated site is written to `build/site`.

## API reference

Generate Doxygen through CMake:

```sh
cmake -S . -B build-docs -DQACCELPLOT_BUILD_DOCS=ON
cmake --build build-docs --target qaccelplot_docs
```

This route configures the complete QAccelPlot build and therefore requires the
normal Qt development dependencies. The GitHub Actions workflow configures
Doxygen directly so it can publish API documentation without compiling the
library.

For a local copy matching the deployed layout, build MkDocs and copy the
contents of the generated Doxygen `html` directory into `build/site/api`.
