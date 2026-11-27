#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: MIT
#
"""Runs the Static analysis CI checks (clang-format and clang-tidy 19) on changed sources.

    py scripts/check-changed.py origin/main --qt-dir C:/Qt/6.7.3/msvc2022_64

Changed sources are the .cpp and .hpp files under QAccelPlot/src/, QAccelPlot/test/, and
examples/ that differ from the merge base with the given base, including uncommitted and
untracked files. clang-tidy also checks every .cpp that includes a changed header.

The first run configures a compile database in --build-dir (Ninja, clang-cl on Windows,
clang++ elsewhere); every run builds the generated sources the checks need. On Windows the
build and clang-tidy run inside the MSVC environment found with vswhere.
"""

import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parent.parent
AREAS = ("QAccelPlot/src/", "QAccelPlot/test/", "examples/")
SOURCE_SUFFIXES = (".cpp", ".hpp")
REQUIRED_MAJOR = 19
IS_WINDOWS = sys.platform == "win32"
WINDOWS_LLVM_DIR = pathlib.Path("C:/Program Files/LLVM/bin")
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)
FORMAT_BATCH_SIZE = 100
FORMAT_ERROR_RE = re.compile(r"^(.+?):\d+:\d+: (?:error|warning):", re.MULTILINE)


def fail(message: str) -> None:
    print(f"check-changed: {message}", file=sys.stderr)
    raise SystemExit(1)


def run(args, env=None, check=True, capture=True) -> subprocess.CompletedProcess:
    try:
        result = subprocess.run(
            args,
            cwd=ROOT,
            env=env,
            capture_output=capture,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
    except FileNotFoundError:
        fail(f"command not found: {args[0]}")
    if check and result.returncode != 0:
        if capture:
            sys.stderr.write(result.stdout + result.stderr)
        fail(f"command failed with exit code {result.returncode}: {' '.join(map(str, args))}")
    return result


def git_lines(*args) -> list:
    return [line for line in run(["git", *args]).stdout.splitlines() if line]


def in_areas(path: str, suffixes=SOURCE_SUFFIXES) -> bool:
    return path.startswith(AREAS) and path.endswith(suffixes)


def changed_sources(base: str) -> list:
    merge_base = run(["git", "merge-base", base, "HEAD"]).stdout.strip()
    paths = set(git_lines("diff", "--name-only", "--diff-filter=d", merge_base))
    paths.update(git_lines("ls-files", "--others", "--exclude-standard"))
    return sorted(path for path in paths if in_areas(path) and (ROOT / path).is_file())


def all_sources() -> list:
    paths = git_lines("ls-files", "--cached", "--others", "--exclude-standard", "--", *AREAS)
    return sorted(path for path in set(paths) if in_areas(path) and (ROOT / path).is_file())


def tidy_targets(changed: list, sources: list) -> list:
    """Changed .cpp files plus every .cpp that directly includes a changed header."""
    targets = {path for path in changed if path.endswith(".cpp")}
    header_names = {pathlib.PurePosixPath(path).name for path in changed if path.endswith(".hpp")}
    if header_names:
        for path in sources:
            if not path.endswith(".cpp") or path in targets:
                continue
            text = (ROOT / path).read_text(encoding="utf-8", errors="replace")
            if any(pathlib.PurePosixPath(inc).name in header_names for inc in INCLUDE_RE.findall(text)):
                targets.add(path)
    return sorted(targets)


def tool_major_version(tool: str) -> int:
    output = run([tool, "--version"]).stdout
    match = re.search(r"version (\d+)\.", output)
    if not match:
        fail(f"cannot read the version of {tool}")
    return int(match.group(1))


def find_tool(names: list, llvm_dir) -> str:
    search_dirs = [llvm_dir] if llvm_dir else []
    if not llvm_dir and IS_WINDOWS:
        search_dirs.append(WINDOWS_LLVM_DIR)
    for name in names:
        for directory in search_dirs:
            found = shutil.which(name, path=str(directory))
            if found:
                return found
        if not llvm_dir:
            found = shutil.which(name)
            if found:
                return found
    where = f" in {llvm_dir}" if llvm_dir else " on PATH"
    fail(f"none of {', '.join(names)} found{where}; pass --llvm-dir or set QACCELPLOT_LLVM_DIR")


def checked_tool(base_name: str, llvm_dir) -> str:
    tool = find_tool([f"{base_name}-{REQUIRED_MAJOR}", base_name], llvm_dir)
    major = tool_major_version(tool)
    if major != REQUIRED_MAJOR:
        fail(f"{tool} is version {major}; CI uses {REQUIRED_MAJOR}. Pass --llvm-dir with LLVM {REQUIRED_MAJOR}")
    return tool


def msvc_environment() -> dict:
    vswhere = pathlib.Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)"))
    vswhere = vswhere / "Microsoft Visual Studio/Installer/vswhere.exe"
    if not vswhere.is_file():
        fail(f"vswhere not found at {vswhere}; install Visual Studio with the C++ tools")
    installation = run(
        [
            str(vswhere),
            "-latest",
            "-products",
            "*",
            "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property",
            "installationPath",
        ]
    ).stdout.strip()
    vcvars = pathlib.Path(installation) / "VC/Auxiliary/Build/vcvars64.bat"
    if not installation or not vcvars.is_file():
        fail("no Visual Studio installation with the x64 C++ tools found")
    # shell=True runs cmd /c, which strips the outer quotes and keeps the quoted batch path.
    output = subprocess.run(
        f'"{vcvars}" >nul && set',
        shell=True,
        capture_output=True,
        text=True,
        errors="replace",
    )
    if output.returncode != 0:
        fail(f"{vcvars} failed:\n{output.stdout}{output.stderr}")
    env = {}
    for line in output.stdout.splitlines():
        key, sep, value = line.partition("=")
        if sep and key:
            env[key.upper()] = value
    return env


def tool_environment(llvm_dir) -> dict:
    if IS_WINDOWS and "VCINSTALLDIR" not in os.environ:
        env = msvc_environment()
    else:
        env = {key.upper() if IS_WINDOWS else key: value for key, value in os.environ.items()}
    if llvm_dir:
        env["PATH"] = str(llvm_dir) + os.pathsep + env.get("PATH", "")
    elif IS_WINDOWS and WINDOWS_LLVM_DIR.is_dir():
        env["PATH"] = str(WINDOWS_LLVM_DIR) + os.pathsep + env.get("PATH", "")
    return env


def cxx_compiler(env: dict) -> str:
    names = ["clang-cl"] if IS_WINDOWS else [f"clang++-{REQUIRED_MAJOR}", "clang++"]
    for name in names:
        found = shutil.which(name, path=env.get("PATH"))
        if found:
            return found
    fail(f"none of {', '.join(names)} found; pass --llvm-dir")


def prepare_compile_database(args, env: dict) -> pathlib.Path:
    build_dir = (ROOT / args.build_dir).resolve()
    database = build_dir / "compile_commands.json"
    if not database.is_file():
        configure = [
            "cmake",
            "-S",
            str(ROOT),
            "-B",
            str(build_dir),
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=Debug",
            f"-DCMAKE_CXX_COMPILER={cxx_compiler(env)}",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            "-DQACCELPLOT_BUILD_TESTS=ON",
            "-DQACCELPLOT_BUILD_EXAMPLES=ON",
            "-DQACCELPLOT_DEPLOY_EXAMPLES=OFF",
            "-DCMAKE_GLOBAL_AUTOGEN_TARGET=ON",
        ]
        if args.qt_dir:
            configure.append(f"-DCMAKE_PREFIX_PATH={args.qt_dir}")
        timed("configure", lambda: run(configure, env=env, capture=False))
    build = ["cmake", "--build", str(build_dir), "--target", "QAccelPlot", "autogen"]
    if args.jobs:
        build += ["--parallel", str(args.jobs)]
    timed("build generated sources", lambda: run(build, env=env, capture=False))
    if not database.is_file():
        fail(f"{database} was not generated")
    return deduplicated_database(database, build_dir / "tidy-db")


def normalized(path: str, directory: str = "") -> str:
    return os.path.normcase(os.path.abspath(os.path.join(directory, path)))


def deduplicated_database(database: pathlib.Path, output_dir: pathlib.Path) -> pathlib.Path:
    """Keeps one entry per source file; shared test sources appear once per test target."""
    entries = json.loads(database.read_text(encoding="utf-8"))
    unique = {}
    for entry in entries:
        key = normalized(entry["file"], entry.get("directory", ""))
        if key not in unique:
            unique[key] = entry
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "compile_commands.json").write_text(json.dumps(list(unique.values()), indent=1), encoding="utf-8")
    return output_dir


def database_files(database_dir: pathlib.Path) -> set:
    entries = json.loads((database_dir / "compile_commands.json").read_text(encoding="utf-8"))
    return {normalized(entry["file"], entry.get("directory", "")) for entry in entries}


def timed(label: str, action):
    print(f"== {label}", flush=True)
    start = time.monotonic()
    result = action()
    print(f"== {label}: {time.monotonic() - start:.1f} s", flush=True)
    return result


def check_format(clang_format: str, files: list) -> list:
    failed = []
    for start in range(0, len(files), FORMAT_BATCH_SIZE):
        batch = files[start : start + FORMAT_BATCH_SIZE]
        result = run([clang_format, "--dry-run", "--Werror", *batch], check=False)
        output = result.stdout + result.stderr
        if result.returncode != 0 or output.strip():
            sys.stdout.write(output)
            reported = {match.replace("\\", "/") for match in FORMAT_ERROR_RE.findall(output)}
            failed.extend(path for path in batch if path in reported)
            if result.returncode != 0 and not reported:
                failed.extend(batch)
    return sorted(set(failed))


def check_tidy(clang_tidy: str, database_dir: pathlib.Path, files: list, env: dict, jobs: int) -> list:
    def tidy(path):
        result = run([clang_tidy, "-p", str(database_dir), "--quiet", str(ROOT / path)], env=env, check=False)
        return path, result

    failed = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        futures = [pool.submit(tidy, path) for path in files]
        for done, future in enumerate(concurrent.futures.as_completed(futures), start=1):
            path, result = future.result()
            findings = result.returncode != 0 or result.stdout.strip()
            print(f"[{done}/{len(files)}] {'FAIL' if findings else 'ok'} {path}", flush=True)
            if findings:
                failed.append(path)
                sys.stdout.write(result.stdout + result.stderr)
                sys.stdout.flush()
    return sorted(failed)


def run_tidy(args, files: list) -> list:
    llvm_dir = pathlib.Path(args.llvm_dir) if args.llvm_dir else None
    clang_tidy = checked_tool("clang-tidy", llvm_dir)
    env = tool_environment(llvm_dir)
    database_dir = prepare_compile_database(args, env)
    known = database_files(database_dir)
    targets = []
    for path in files:
        if normalized(str(ROOT / path)) in known:
            targets.append(path)
        else:
            print(f"warning: {path} is not in the compile database; skipping clang-tidy", file=sys.stderr)
    if not targets:
        return []
    jobs = args.jobs or os.cpu_count() or 1
    return timed(
        f"clang-tidy ({len(targets)} files, {jobs} jobs)",
        lambda: check_tidy(clang_tidy, database_dir, targets, env, jobs),
    )


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("base", nargs="?", default="origin/main", help="branch or commit to compare with")
    parser.add_argument("--build-dir", default="build-tidy", help="compile database build directory")
    parser.add_argument(
        "--qt-dir", default=os.environ.get("QACCELPLOT_QT_DIR"), help="Qt kit passed as CMAKE_PREFIX_PATH"
    )
    parser.add_argument(
        "--llvm-dir", default=os.environ.get("QACCELPLOT_LLVM_DIR"), help="directory with the LLVM 19 tools"
    )
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="parallel clang-tidy and build jobs")
    parser.add_argument("--no-format", action="store_true", help="skip clang-format")
    parser.add_argument("--no-tidy", action="store_true", help="skip clang-tidy")
    parser.add_argument("--all", action="store_true", help="check every source instead of the changed ones")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    sources = all_sources()
    changed = sources if args.all else changed_sources(args.base)
    scope = "all" if args.all else f"changed since {args.base}"
    print(f"{len(changed)} sources ({scope})", flush=True)

    format_failures = []
    if not args.no_format and changed:
        llvm_dir = pathlib.Path(args.llvm_dir) if args.llvm_dir else None
        clang_format = checked_tool("clang-format", llvm_dir)
        format_failures = timed(f"clang-format ({len(changed)} files)", lambda: check_format(clang_format, changed))

    tidy_failures = []
    if not args.no_tidy:
        targets = [path for path in changed if path.endswith(".cpp")] if args.all else tidy_targets(changed, sources)
        if targets:
            tidy_failures = run_tidy(args, targets)

    print()
    if not format_failures and not tidy_failures:
        print("check-changed: all checks passed")
        return
    for path in format_failures:
        print(f"clang-format: {path}")
    for path in tidy_failures:
        print(f"clang-tidy:   {path}")
    print(f"check-changed: {len(format_failures)} format and {len(tidy_failures)} clang-tidy failures")
    raise SystemExit(1)


if __name__ == "__main__":
    main()
