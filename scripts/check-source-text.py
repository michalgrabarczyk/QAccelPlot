#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: MIT
#
"""Check C++ sources for control bytes and non-ASCII string literals.

Fails on:
  - any control byte except tab, LF, and CR;
  - any non-ASCII character inside a string or character literal, including
    prefixed (u8"", u"", U"", L"") and raw (R"delim(...)delim") literals.

Write non-ASCII characters in literals as \\u escapes, for example
QStringLiteral("\\u00B5m"). Comments may contain non-ASCII text.

Without paths, checks every tracked *.cpp and *.hpp file (git ls-files).
"""

import argparse
import pathlib
import re
import subprocess
import sys

ALLOWED_CONTROLS = {"\t", "\n", "\r"}
LITERAL_PREFIXES = {"", "L", "u", "U", "u8"}
IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
NUMBER_RE = re.compile(r"\.?[0-9](?:[eEpP][+-]|'[0-9A-Za-z_]|[0-9A-Za-z_.])*")


def is_control(char: str) -> bool:
    return (ord(char) < 0x20 or ord(char) == 0x7F) and char not in ALLOWED_CONTROLS


def literal_spans(text: str):
    """Yield (start, end) offsets of the contents of every string and character literal."""
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            end = text.find("\n", i)
            i = n if end < 0 else end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = n if end < 0 else end + 2
        elif c.isdigit() or (c == "." and i + 1 < n and text[i + 1].isdigit()):
            i = NUMBER_RE.match(text, i).end()
        elif c.isalpha() or c == "_":
            word = IDENTIFIER_RE.match(text, i).group()
            i += len(word)
            if i < n and text[i] == '"' and word.endswith("R") and word[:-1] in LITERAL_PREFIXES:
                i = yield from raw_literal(text, i)
            elif i < n and text[i] in "\"'" and word in LITERAL_PREFIXES:
                i = yield from quoted_literal(text, i)
        elif c in "\"'":
            i = yield from quoted_literal(text, i)
        else:
            i += 1


def quoted_literal(text: str, start: int):
    quote = text[start]
    i = start + 1
    while i < len(text) and text[i] not in (quote, "\n"):
        i += 2 if text[i] == "\\" else 1
    yield start + 1, i
    return i + 1


def raw_literal(text: str, start: int):
    open_paren = text.find("(", start)
    delimiter = text[start + 1 : open_paren]
    end = text.find(")" + delimiter + '"', open_paren)
    if open_paren < 0 or end < 0 or len(delimiter) > 16:
        raise ValueError(f"unterminated raw string literal at offset {start}")
    yield open_paren + 1, end
    return end + len(delimiter) + 2


def location(text: str, offset: int) -> str:
    line = text.count("\n", 0, offset) + 1
    column = offset - (text.rfind("\n", 0, offset) + 1) + 1
    return f"{line}:{column}"


def check_file(path: str) -> list:
    data = pathlib.Path(path).read_bytes()
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as error:
        return [f"{path}: invalid UTF-8 at byte {error.start}"]
    problems = [
        f"{path}:{location(text, i)}: control character U+{ord(c):04X}"
        for i, c in enumerate(text)
        if is_control(c)
    ]
    try:
        for start, end in literal_spans(text):
            for i in range(start, end):
                if ord(text[i]) > 0x7F:
                    problems.append(
                        f"{path}:{location(text, i)}: non-ASCII U+{ord(text[i]):04X} in literal, "
                        f"write it as \\u{ord(text[i]):04X}"
                    )
    except ValueError as error:
        problems.append(f"{path}: {error}")
    return problems


def tracked_sources() -> list:
    output = subprocess.run(
        ["git", "ls-files", "*.cpp", "*.hpp"], check=True, capture_output=True, text=True
    ).stdout
    return output.split()


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("paths", nargs="*", help="files to check (default: tracked *.cpp and *.hpp)")
    args = parser.parse_args()
    paths = args.paths or tracked_sources()
    if not paths:
        print("check-source-text: no files to check", file=sys.stderr)
        return 1
    problems = [problem for path in paths for problem in check_file(path)]
    for problem in problems:
        print(problem)
    print(f"Checked {len(paths)} files, {len(problems)} problems", file=sys.stderr)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
