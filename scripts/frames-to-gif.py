#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Turns the frames of an example's --record run into an optimized, looping GIF.

    QAccelPlotExampleVibrationMonitoring --record frames/dark --theme dark
    python scripts/frames-to-gif.py frames/dark vibration-dark.gif

Uses gifski when it is on PATH (or given with --gifski), otherwise ffmpeg with a palette
generated from all frames (palettegen) and applied with dithering (paletteuse). ffmpeg is
taken from --ffmpeg, the FFMPEG environment variable, or PATH.
"""

from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import subprocess
import sys

FRAME_PATTERN = "frame_%05d.png"


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("frames", type=pathlib.Path, help="directory with frame_00000.png, frame_00001.png, ...")
    parser.add_argument("output", type=pathlib.Path, help="GIF file to write")
    parser.add_argument("--fps", type=int, default=25, help="frame rate the frames were recorded at (default: 25)")
    parser.add_argument("--width", type=int, help="output width in pixels, keeping the aspect ratio (default: the frame width)")
    parser.add_argument("--colors", type=int, default=256, help="palette size for ffmpeg, 2 to 256 (default: 256)")
    parser.add_argument(
        "--color-bits",
        type=int,
        default=5,
        help="bits kept per color channel before ffmpeg builds the palette, 1 to 8 (default: 5). Fewer bits hide "
        "rendering differences too small to see, which lets unchanged pixels stay transparent between frames.",
    )
    parser.add_argument(
        "--dither",
        default="none",
        help="ffmpeg paletteuse dither: none, bayer, sierra2_4a, or floyd_steinberg (default: none, the smallest for flat UI colors)",
    )
    parser.add_argument("--quality", type=int, default=90, help="gifski quality, 1 to 100 (default: 90)")
    parser.add_argument("--ffmpeg", help="path to ffmpeg")
    parser.add_argument("--gifski", help="path to gifski; preferred over ffmpeg when found")
    parser.add_argument("--max-megabytes", type=float, default=5.0, help="warn when the GIF is larger (default: 5)")
    return parser.parse_args()


def find_tool(explicit: str | None, environment_variable: str | None, name: str) -> str | None:
    if explicit:
        return explicit
    if environment_variable and os.environ.get(environment_variable):
        return os.environ[environment_variable]
    return shutil.which(name)


def gifski_command(gifski: str, frames: list[pathlib.Path], arguments: argparse.Namespace) -> list[str]:
    size = ["--width", str(arguments.width)] if arguments.width else []
    return [
        gifski,
        "--fps",
        str(arguments.fps),
        *size,
        "--quality",
        str(arguments.quality),
        "--output",
        str(arguments.output),
        *[str(frame) for frame in frames],
    ]


def ffmpeg_command(ffmpeg: str, arguments: argparse.Namespace) -> list[str]:
    # One palette for the whole animation keeps colors from flickering between frames.
    # diff_mode=rectangle re-encodes only the changed area of each frame.
    dither = arguments.dither if arguments.dither != "bayer" else "bayer:bayer_scale=3"
    filters = []
    if arguments.width:
        filters.append(f"scale={arguments.width}:-1:flags=lanczos")
    if arguments.color_bits < 8:
        mask = (0xFF << (8 - arguments.color_bits)) & 0xFF
        filters.append(f"lutrgb=r='bitand(val,{mask})':g='bitand(val,{mask})':b='bitand(val,{mask})'")
    filters.append("split[frames][copy]")
    graph = (
        ",".join(filters) + ";"
        f"[copy]palettegen=max_colors={arguments.colors}:stats_mode=full[palette];"
        f"[frames][palette]paletteuse=dither={dither}:diff_mode=rectangle"
    )
    return [
        ffmpeg,
        "-hide_banner",
        "-loglevel",
        "error",
        "-y",
        "-framerate",
        str(arguments.fps),
        "-i",
        str(arguments.frames / FRAME_PATTERN),
        "-filter_complex",
        graph,
        "-loop",
        "0",
        str(arguments.output),
    ]


def main() -> int:
    arguments = parse_arguments()
    frames = sorted(arguments.frames.glob("frame_*.png"))
    if not frames:
        print(f"No frame_*.png files in {arguments.frames}", file=sys.stderr)
        return 1

    gifski = find_tool(arguments.gifski, None, "gifski")
    ffmpeg = find_tool(arguments.ffmpeg, "FFMPEG", "ffmpeg")
    if gifski:
        command = gifski_command(gifski, frames, arguments)
    elif ffmpeg:
        command = ffmpeg_command(ffmpeg, arguments)
    else:
        print("Neither gifski nor ffmpeg was found. Install one, or pass --ffmpeg or --gifski.", file=sys.stderr)
        return 1

    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(command, check=True)

    megabytes = arguments.output.stat().st_size / 1e6
    print(f"{arguments.output}: {len(frames)} frames, {megabytes:.2f} MB ({pathlib.Path(command[0]).stem})")
    if megabytes > arguments.max_megabytes:
        print(f"Warning: larger than {arguments.max_megabytes:g} MB. Try a smaller --width, fewer --colors or --color-bits, or a lower --fps recording.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
