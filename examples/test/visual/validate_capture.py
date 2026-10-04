#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Validates an example screenshot and the graphics metadata captured with it."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import struct
import sys
from typing import Any


PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
VALID_GRAPHICS_APIS = {"opengl", "vulkan", "d3d11", "d3d12", "metal", "software", "null"}
# Smaller than any rendered example page; a blank or truncated capture falls below it.
DEFAULT_MINIMUM_FILE_SIZE_BYTES = 15000


class CaptureValidationError(RuntimeError):
    """Input or metadata validation failure."""


def read_png_dimensions(path: Path) -> tuple[int, int]:
    try:
        with path.open("rb") as image_file:
            header = image_file.read(24)
    except OSError as error:
        raise CaptureValidationError(f"Unable to read screenshot {path}: {error}") from error

    if len(header) != 24 or header[:8] != PNG_SIGNATURE or header[12:16] != b"IHDR":
        raise CaptureValidationError(f"Screenshot is not a valid PNG: {path}")
    return struct.unpack(">II", header[16:24])


def validate_image(path: Path, minimum_file_size_bytes: int = DEFAULT_MINIMUM_FILE_SIZE_BYTES) -> tuple[int, int]:
    if not path.is_file():
        raise CaptureValidationError(f"Screenshot does not exist: {path}")

    if path.stat().st_size < minimum_file_size_bytes:
        raise CaptureValidationError(
            f"Screenshot is suspiciously small: {path.stat().st_size} bytes; "
            f"expected at least {minimum_file_size_bytes}."
        )
    return read_png_dimensions(path)


def load_render_metadata(path: Path) -> dict[str, Any]:
    try:
        metadata = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise CaptureValidationError(f"Unable to read graphics metadata {path}: {error}") from error

    required_fields = {
        "version",
        "capture_method",
        "grab_target_scaled_by_device_pixel_ratio",
        "requested_graphics_api",
        "actual_graphics_api",
        "opengl_context_observed",
        "opengl_es",
        "opengl_major_version",
        "opengl_minor_version",
        "qt_version",
        "logical_window_width",
        "logical_window_height",
        "offscreen_target_width",
        "offscreen_target_height",
        "effective_device_pixel_ratio",
        "captured_pixel_width",
        "captured_pixel_height",
        "page",
    }
    missing_fields = required_fields.difference(metadata) if isinstance(metadata, dict) else required_fields
    if missing_fields:
        raise CaptureValidationError(f"Graphics metadata is missing fields: {sorted(missing_fields)}")
    if metadata["version"] != 5:
        raise CaptureValidationError(f"Unsupported graphics metadata version: {metadata['version']}")
    if metadata["capture_method"] != "item_grab_to_image":
        raise CaptureValidationError(f"Unsupported screenshot capture method: {metadata['capture_method']}")

    requested_api = metadata["requested_graphics_api"]
    actual_api = metadata["actual_graphics_api"]
    if requested_api not in VALID_GRAPHICS_APIS or actual_api not in VALID_GRAPHICS_APIS:
        raise CaptureValidationError(f"Graphics metadata contains an unsupported API: requested={requested_api}, actual={actual_api}")
    if requested_api != actual_api:
        raise CaptureValidationError(f"Requested graphics API {requested_api}, but the screenshot used {actual_api}.")

    expected_api = os.environ.get("QACCELPLOT_EXPECTED_GRAPHICS_API", "").strip().lower()
    if expected_api and actual_api != expected_api:
        raise CaptureValidationError(f"CI expected graphics API {expected_api}, but the screenshot used {actual_api}.")
    validate_opengl_context(metadata)
    if not isinstance(metadata["page"], str):
        raise CaptureValidationError(f"Graphics metadata contains an invalid page: {metadata['page']}")
    if not isinstance(metadata["qt_version"], str) or not metadata["qt_version"].strip():
        raise CaptureValidationError("Graphics metadata contains no Qt version.")
    expected_qt_version = os.environ.get("QACCELPLOT_EXPECTED_QT_VERSION", "").strip()
    if expected_qt_version and metadata["qt_version"] != expected_qt_version:
        raise CaptureValidationError(
            f"CI expected Qt {expected_qt_version}, but the screenshot used Qt {metadata['qt_version']}."
        )
    return metadata


def parse_opengl_es_version(text: str) -> tuple[int, int]:
    match = re.fullmatch(r"(\d+)(?:\.(\d+))?", text)
    if not match or int(match.group(1)) <= 0:
        raise CaptureValidationError(f'QACCELPLOT_OPENGL_ES_VERSION must look like "3.0", got "{text}".')
    return int(match.group(1)), int(match.group(2) or 0)


def validate_opengl_context(metadata: dict[str, Any]) -> None:
    observed = metadata["opengl_context_observed"]
    is_opengl_es = metadata["opengl_es"]
    if not isinstance(observed, bool) or not isinstance(is_opengl_es, bool):
        raise CaptureValidationError("Graphics metadata contains invalid OpenGL context flags.")
    version = (metadata["opengl_major_version"], metadata["opengl_minor_version"])
    for value in version:
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise CaptureValidationError(f"Graphics metadata contains an invalid OpenGL version: {value}")
    if metadata["actual_graphics_api"] == "opengl" and (not observed or version[0] <= 0):
        raise CaptureValidationError("The OpenGL capture did not record a valid current context.")

    # A desktop Qt build falls back to desktop OpenGL without an error, so an ES run must prove its context.
    requested_text = os.environ.get("QACCELPLOT_OPENGL_ES_VERSION", "").strip()
    if not requested_text:
        return
    requested = parse_opengl_es_version(requested_text)
    if metadata["actual_graphics_api"] != "opengl" or not is_opengl_es or version < requested:
        raise CaptureValidationError(
            f"CI expected OpenGL ES {requested[0]}.{requested[1]} or newer, but the screenshot used "
            f"API={metadata['actual_graphics_api']}, ES={is_opengl_es}, version={version[0]}.{version[1]}."
        )


def validate_capture_resolution(metadata: dict[str, Any], image_dimensions: tuple[int, int]) -> None:
    grab_target_scaled = metadata["grab_target_scaled_by_device_pixel_ratio"]
    if not isinstance(grab_target_scaled, bool):
        raise CaptureValidationError(
            "Graphics metadata contains an invalid grab_target_scaled_by_device_pixel_ratio: "
            f"{grab_target_scaled}"
        )

    dimension_fields = (
        "logical_window_width",
        "logical_window_height",
        "offscreen_target_width",
        "offscreen_target_height",
        "captured_pixel_width",
        "captured_pixel_height",
    )
    for field in dimension_fields:
        value = metadata[field]
        if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
            raise CaptureValidationError(f"Graphics metadata contains an invalid {field}: {value}")

    effective_device_pixel_ratio = metadata["effective_device_pixel_ratio"]
    if (
        not isinstance(effective_device_pixel_ratio, (int, float))
        or isinstance(effective_device_pixel_ratio, bool)
        or effective_device_pixel_ratio <= 0
    ):
        raise CaptureValidationError(
            f"Graphics metadata contains an invalid effective_device_pixel_ratio: {effective_device_pixel_ratio}"
        )

    logical_size = (metadata["logical_window_width"], metadata["logical_window_height"])
    captured_size = (metadata["captured_pixel_width"], metadata["captured_pixel_height"])
    if captured_size != logical_size:
        raise CaptureValidationError(
            f"Screenshot was not rendered at its logical size: captured {captured_size}; logical window {logical_size}."
        )
    grab_scale = effective_device_pixel_ratio if grab_target_scaled else 1.0
    expected_capture_size = (
        round(metadata["offscreen_target_width"] * grab_scale),
        round(metadata["offscreen_target_height"] * grab_scale),
    )
    if captured_size != expected_capture_size:
        raise CaptureValidationError(
            f"Offscreen target predicts {expected_capture_size} pixels; captured {captured_size}."
        )
    if image_dimensions != captured_size:
        raise CaptureValidationError(f"PNG dimensions {image_dimensions} do not match captured pixels {captured_size}.")


def validate_capture_page(metadata: dict[str, Any], expected_page: str) -> None:
    """Rejects a screenshot of a different page, e.g. after a tab was renamed or reordered."""
    if metadata["page"] != expected_page:
        expected = f"page '{expected_page}'" if expected_page else "an example without pages"
        raise CaptureValidationError(f"Screenshot shows page '{metadata['page']}'; expected {expected}.")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate an example screenshot and its graphics metadata.")
    parser.add_argument("--image", required=True, type=Path, help="PNG screenshot to validate")
    parser.add_argument("--metadata", type=Path, help="Graphics metadata JSON; defaults to <image>.rhi.json")
    parser.add_argument("--page", default="", help="Page the screenshot must show; empty for examples without pages")
    parser.add_argument("--minimum-file-size-bytes", type=int, default=DEFAULT_MINIMUM_FILE_SIZE_BYTES)
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    try:
        dimensions = validate_image(args.image, args.minimum_file_size_bytes)
        render_metadata = load_render_metadata(args.metadata or Path(f"{args.image}.rhi.json"))
        validate_capture_resolution(render_metadata, dimensions)
        validate_capture_page(render_metadata, args.page)
    except CaptureValidationError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2

    print(
        f"PASS: {render_metadata['actual_graphics_api']} / Qt {render_metadata['qt_version']} "
        f"{dimensions[0]}x{dimensions[1]} screenshot is valid."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
