#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# generate-docs-palette.py — Export the QAccelPlot color palettes as CSS custom
# properties for the MkDocs site.
#
# Usage:
#   python scripts/generate-docs-palette.py           # write the stylesheet
#   python scripts/generate-docs-palette.py --check   # fail if it is out of date
#
# Output:
#   docs/guide/stylesheets/palette.css, with Colors.light values under the
#   Material "default" scheme and Colors.dark values under the "slate" scheme.

"""Generate docs/guide/stylesheets/palette.css from ColorPalette.cpp."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PALETTE_HEADER = REPO_ROOT / "QAccelPlot" / "src" / "theme" / "ColorPalette.hpp"
PALETTE_SOURCE = REPO_ROOT / "QAccelPlot" / "src" / "theme" / "ColorPalette.cpp"
OUTPUT = REPO_ROOT / "docs" / "guide" / "stylesheets" / "palette.css"

# Material scheme name for each ColorPalette factory.
SCHEMES = {
    "light": ("createLight", "default"),
    "dark": ("createDark", "slate"),
}

# Foreground/background pairs that docs/guide/stylesheets/theme.css renders as
# text. Each must meet WCAG AA for normal text. A color is a palette name or a
# (color, weight, color) tuple, matching CSS color-mix(in srgb, a weight, b).
MINIMUM_CONTRAST = 4.5
LIGHT_HEADER = "materialAccent"
DARK_HEADER = ("materialAccent", 0.45, "window")
CONTRAST_PAIRS = {
    "light": [
        ("text", "plotArea"),
        ("textSecondary", "plotArea"),
        ("hover", "plotArea"),
        ("materialAccent", "plotArea"),
        ("textOnAccent", LIGHT_HEADER),
        ("text", "axesArea"),
        ("textSecondary", "axesArea"),
        ("seriesPrimary", "axesArea"),
        ("seriesSecondary", "axesArea"),
        ("seriesTertiary", "axesArea"),
        ("seriesQuaternary", "axesArea"),
        ("seriesCyan", "axesArea"),
        ("seriesRose", "axesArea"),
        ("statusError", "axesArea"),
    ],
    "dark": [
        ("text", "window"),
        ("textSecondary", "window"),
        ("seriesPrimary", "window"),
        ("materialAccent", "window"),
        ("text", DARK_HEADER),
        ("text", "axesArea"),
        ("textMuted", "axesArea"),
        ("text", "plotArea"),
        ("textSecondary", "plotArea"),
        ("seriesPrimary", "plotArea"),
        ("seriesSecondary", "plotArea"),
        ("seriesTertiary", "plotArea"),
        ("seriesQuaternary", "plotArea"),
        ("seriesCyan", "plotArea"),
        ("seriesRose", "plotArea"),
        ("statusError", "plotArea"),
    ],
}

PROPERTY_PATTERN = re.compile(r"Q_PROPERTY\(QColor\s+(\w+)\s+MEMBER")
HEX_PATTERN = re.compile(r'p->(\w+)\s*=\s*QColor\("#([0-9a-fA-F]{6}|[0-9a-fA-F]{8})"\);')
TRANSPARENT_PATTERN = re.compile(r"p->(\w+)\s*=\s*QColor\(Qt::transparent\);")
ALIAS_PATTERN = re.compile(r"p->(\w+)\s*=\s*p->(\w+);")
ASSIGNMENT_PATTERN = re.compile(r"p->(\w+)\s*=")


def property_names() -> list[str]:
    return PROPERTY_PATTERN.findall(PALETTE_HEADER.read_text(encoding="utf-8"))


def factory_body(source: str, factory: str) -> str:
    match = re.search(
        rf"ColorPalette\*\s+ColorPalette::{factory}\(\)\s*\{{(.*?)\n\}}", source, re.DOTALL
    )
    if not match:
        raise RuntimeError(f"Cannot find ColorPalette::{factory}() in {PALETTE_SOURCE}")
    return match.group(1)


def to_css(argb_or_rgb: str) -> str:
    """Convert Qt's #RRGGBB or #AARRGGBB notation to CSS #rrggbb or #rrggbbaa."""
    value = argb_or_rgb.lower()
    if len(value) == 8:
        alpha = value[:2]
        return f"#{value[2:]}" if alpha == "ff" else f"#{value[2:]}{alpha}"
    return f"#{value}"


def parse_palette(source: str, factory: str, names: list[str]) -> dict[str, str]:
    body = factory_body(source, factory)
    colors: dict[str, str] = {}
    for name, value in HEX_PATTERN.findall(body):
        colors[name] = to_css(value)
    for name in TRANSPARENT_PATTERN.findall(body):
        colors[name] = "#00000000"
    for name, target in ALIAS_PATTERN.findall(body):
        colors[name] = colors[target]

    unparsed = set(ASSIGNMENT_PATTERN.findall(body)) - colors.keys()
    if unparsed:
        raise RuntimeError(
            f"ColorPalette::{factory}() assigns {', '.join(sorted(unparsed))} in a form this "
            "script does not understand; extend scripts/generate-docs-palette.py"
        )
    missing = [name for name in names if name not in colors]
    if missing:
        raise RuntimeError(f"ColorPalette::{factory}() does not assign {', '.join(missing)}")
    return {name: colors[name] for name in names}


def css_name(name: str) -> str:
    return "--qap-" + re.sub(r"(?<!^)(?=[A-Z])", "-", name).lower()


def resolve(colors: dict[str, str], spec) -> tuple[float, float, float]:
    """Return the sRGB channels (0-255) of a palette name or (color, weight, color) mix."""
    if isinstance(spec, str):
        value = colors[spec]
        return tuple(float(int(value[i : i + 2], 16)) for i in (1, 3, 5))
    first, weight, second = spec
    a, b = resolve(colors, first), resolve(colors, second)
    return tuple(weight * a[i] + (1 - weight) * b[i] for i in range(3))


def describe(spec) -> str:
    if isinstance(spec, str):
        return spec
    first, weight, second = spec
    return f"mix({describe(first)} {weight:.0%}, {describe(second)})"


def relative_luminance(channels: tuple[float, float, float]) -> float:
    linear = [
        c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4
        for c in (channel / 255 for channel in channels)
    ]
    return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2]


def contrast_ratio(foreground, background) -> float:
    lighter, darker = sorted(
        (relative_luminance(foreground), relative_luminance(background)), reverse=True
    )
    return (lighter + 0.05) / (darker + 0.05)


def contrast_failures(palettes: dict[str, dict[str, str]]) -> list[str]:
    failures = []
    for palette_name, pairs in CONTRAST_PAIRS.items():
        colors = palettes[palette_name]
        for foreground, background in pairs:
            ratio = contrast_ratio(resolve(colors, foreground), resolve(colors, background))
            if ratio < MINIMUM_CONTRAST:
                failures.append(
                    f"Colors.{palette_name}: {describe(foreground)} on {describe(background)} "
                    f"is {ratio:.2f}:1, below {MINIMUM_CONTRAST}:1"
                )
    return failures


def render(palettes: dict[str, dict[str, str]]) -> str:
    lines = [
        "/*",
        " * SPDX-FileCopyrightText: 2026 Michal Grabarczyk",
        " * SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0",
        " *",
        " * Generated by scripts/generate-docs-palette.py from",
        " * QAccelPlot/src/theme/ColorPalette.cpp. Do not edit by hand.",
        " */",
    ]
    for palette_name, (_, scheme) in SCHEMES.items():
        lines.append("")
        lines.append(f"/* Colors.{palette_name} */")
        lines.append(f'[data-md-color-scheme="{scheme}"] {{')
        for name, value in palettes[palette_name].items():
            lines.append(f"  {css_name(name)}: {value};")
        lines.append("}")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check", action="store_true", help="fail instead of writing when the output is stale"
    )
    arguments = parser.parse_args()

    names = property_names()
    source = PALETTE_SOURCE.read_text(encoding="utf-8")
    palettes = {
        palette_name: parse_palette(source, factory, names)
        for palette_name, (factory, _) in SCHEMES.items()
    }

    failures = contrast_failures(palettes)
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1

    content = render(palettes)
    relative_output = OUTPUT.relative_to(REPO_ROOT).as_posix()
    current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else None
    if arguments.check:
        if current != content:
            print(
                f"{relative_output} is out of date; run python scripts/generate-docs-palette.py",
                file=sys.stderr,
            )
            return 1
        return 0

    if current != content:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_bytes(content.encode("utf-8"))
        print(f"Wrote {relative_output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
