#!/usr/bin/env python3

import argparse
import pathlib
import re
import subprocess
import sys

VERSION_RE = re.compile(r"^v(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)$")
CMAKE_VERSION_RE = re.compile(r"project\(QAccelPlot\s+VERSION\s+(\d+\.\d+\.\d+)", re.MULTILINE)


def fail(message: str) -> None:
    print(f"release validation failed: {message}", file=sys.stderr)
    raise SystemExit(1)


def extract_changelog_section(text: str, version: str) -> str:
    header = re.compile(rf"^## \[{re.escape(version)}\](?:\s+[—-]\s+\d{{4}}-\d{{2}}-\d{{2}})?\s*$", re.MULTILINE)
    match = header.search(text)
    if not match:
        fail(f"CHANGELOG.md has no section for {version}")
    start = match.end()
    next_header = re.search(r"^## \[", text[start:], re.MULTILINE)
    end = start + next_header.start() if next_header else len(text)
    body = text[start:end].strip()
    if not body or not re.search(r"^\s*-\s+\S", body, re.MULTILINE):
        fail(f"CHANGELOG.md section for {version} has no release entries")
    return body + "\n"


def validate_pre_one_minor_sequence(current_minor: int, current_tag: str) -> None:
    result = subprocess.run(
        ["git", "tag", "--list", "v0.*.0"],
        check=True,
        capture_output=True,
        text=True,
    )
    previous_minors = []
    for tag in result.stdout.splitlines():
        match = VERSION_RE.fullmatch(tag.strip())
        if not match or tag.strip() == current_tag:
            continue
        if int(match.group("major")) == 0 and int(match.group("patch")) == 0:
            previous_minors.append(int(match.group("minor")))

    if previous_minors:
        expected_minor = max(previous_minors) + 1
        if current_minor != expected_minor:
            fail(
                f"next pre-1.0 release must increment MINOR from "
                f"0.{max(previous_minors)}.0 to 0.{expected_minor}.0"
            )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", required=True)
    parser.add_argument("--cmake", default="CMakeLists.txt")
    parser.add_argument("--changelog", default="CHANGELOG.md")
    parser.add_argument("--notes-output", default="build/release-notes.md")
    args = parser.parse_args()

    tag_match = VERSION_RE.fullmatch(args.tag)
    if not tag_match:
        fail("tag must match vMAJOR.MINOR.PATCH using numeric components only")

    major = int(tag_match.group("major"))
    minor = int(tag_match.group("minor"))
    patch = int(tag_match.group("patch"))
    version = args.tag[1:]

    if major == 0:
        if patch != 0:
            fail("pre-1.0 releases must use 0.MINOR.0; PATCH must remain zero")
        validate_pre_one_minor_sequence(minor, args.tag)

    cmake_text = pathlib.Path(args.cmake).read_text(encoding="utf-8")
    cmake_match = CMAKE_VERSION_RE.search(cmake_text)
    if not cmake_match:
        fail("could not read QAccelPlot project version from CMakeLists.txt")
    if cmake_match.group(1) != version:
        fail(f"tag {args.tag} does not match CMake project version {cmake_match.group(1)}")

    changelog_text = pathlib.Path(args.changelog).read_text(encoding="utf-8")
    notes = extract_changelog_section(changelog_text, version)

    output = pathlib.Path(args.notes_output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(notes, encoding="utf-8")
    print(f"validated release {args.tag}; wrote notes to {output}")


if __name__ == "__main__":
    main()
