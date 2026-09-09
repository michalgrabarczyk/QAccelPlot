#!/usr/bin/env bash
set -euo pipefail

DOCS_VERSION="${QACCELPLOT_DOCS_VERSION:-main}"
DOCS_CHANNEL="${QACCELPLOT_DOCS_CHANNEL:-development}"
DOCS_TITLE="QAccelPlot Documentation"

if [[ "$DOCS_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  DOCS_TITLE="QAccelPlot v${DOCS_VERSION} Documentation"
fi

rm -rf build/site build/api-docs
mkdir -p build/site build/api-docs

cp CHANGELOG.md docs/guide/changelog.md
trap 'rm -f docs/guide/changelog.md' EXIT

export QACCELPLOT_DOCS_VERSION="$DOCS_VERSION"
export QACCELPLOT_DOCS_CHANNEL="$DOCS_CHANNEL"
export QACCELPLOT_DOCS_TITLE="$DOCS_TITLE"
mkdocs build --strict

sed \
  -e 's|@PROJECT_NAME@|QAccelPlot|g' \
  -e "s|@PROJECT_VERSION@|${DOCS_VERSION}|g" \
  -e 's|@PROJECT_DESCRIPTION@|High-performance Qt Quick plotting library|g' \
  -e 's|@DOXYGEN_OUTPUT_DIR@|build/api-docs|g' \
  -e 's|@QACCELPLOT_DOXYGEN_INPUT@|QAccelPlot/src|g' \
  -e 's|@DOXYGEN_HAVE_DOT@|YES|g' \
  -e 's|@DOXYGEN_DOT_PATH@||g' \
  docs/Doxyfile.in > build/api-docs/Doxyfile

doxygen build/api-docs/Doxyfile
rm -rf build/site/api
mkdir -p build/site/api
cp -R build/api-docs/html/. build/site/api/

cp llms.txt build/site/llms.txt
python scripts/generate-llms-full.py --output build/site/llms-full.txt
