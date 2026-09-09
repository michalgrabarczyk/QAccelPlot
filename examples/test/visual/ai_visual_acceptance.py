#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Reference-free visual acceptance testing for QAccelPlot example screenshots."""

from __future__ import annotations

import argparse
import base64
import copy
import json
import os
from pathlib import Path
import struct
import sys
from typing import Any


PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
VALID_STATUSES = {"pass", "fail", "uncertain"}
VALID_SEVERITIES = {"low", "medium", "high"}
VALID_GRAPHICS_APIS = {"opengl", "vulkan", "d3d11", "d3d12", "metal", "software", "null"}

MODEL_RESPONSE_SCHEMA: dict[str, Any] = {
    "type": "object",
    "additionalProperties": False,
    "required": ["summary", "checks"],
    "properties": {
        "summary": {
            "type": "string",
            "description": "A concise overall visual assessment grounded only in visible evidence.",
        },
        "checks": {
            "type": "array",
            "description": "Exactly one assessment for every check ID supplied in the visual contract.",
            "items": {
                "type": "object",
                "additionalProperties": False,
                "required": ["id", "status", "confidence", "evidence"],
                "properties": {
                    "id": {"type": "string"},
                    "status": {"type": "string", "enum": ["pass", "fail", "uncertain"]},
                    "confidence": {"type": "number", "minimum": 0.0, "maximum": 1.0},
                    "evidence": {
                        "type": "string",
                        "description": "What is visibly present or wrong, including its approximate location.",
                    },
                },
            },
        },
    },
}


class VisualAcceptanceError(RuntimeError):
    """Configuration, input, API, or response-validation failure."""


def configure_console_encoding() -> None:
    """Make model-generated Unicode safe to print in Windows CI logs."""
    for stream in (sys.stdout, sys.stderr):
        reconfigure = getattr(stream, "reconfigure", None)
        if reconfigure is not None:
            reconfigure(encoding="utf-8", errors="backslashreplace")


def load_contract(path: Path) -> dict[str, Any]:
    try:
        contract = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise VisualAcceptanceError(f"Unable to read visual contract {path}: {error}") from error

    required_fields = {"version", "name", "description", "expected_image", "policy", "checks"}
    missing_fields = required_fields.difference(contract)
    if missing_fields:
        raise VisualAcceptanceError(f"Visual contract is missing fields: {sorted(missing_fields)}")
    if contract["version"] != 1:
        raise VisualAcceptanceError(f"Unsupported visual contract version: {contract['version']}")
    if not isinstance(contract["checks"], list) or not contract["checks"]:
        raise VisualAcceptanceError("Visual contract must contain at least one check.")

    check_ids: list[str] = []
    for check in contract["checks"]:
        if not isinstance(check, dict) or not {"id", "severity", "expectation"}.issubset(check):
            raise VisualAcceptanceError("Every contract check requires id, severity, and expectation.")
        if check["severity"] not in VALID_SEVERITIES:
            raise VisualAcceptanceError(f"Invalid severity for check {check['id']}: {check['severity']}")
        check_ids.append(check["id"])
    if len(check_ids) != len(set(check_ids)):
        raise VisualAcceptanceError("Visual contract check IDs must be unique.")

    policy = contract["policy"]
    if not isinstance(policy, dict) or not {"blocking_severities", "minimum_confidence"}.issubset(policy):
        raise VisualAcceptanceError("Visual contract policy requires blocking_severities and minimum_confidence.")
    if not set(policy["blocking_severities"]).issubset(VALID_SEVERITIES):
        raise VisualAcceptanceError("Visual contract contains an invalid blocking severity.")
    if not 0.0 <= float(policy["minimum_confidence"]) <= 1.0:
        raise VisualAcceptanceError("Visual contract minimum_confidence must be between zero and one.")

    return contract


def read_png_dimensions(path: Path) -> tuple[int, int]:
    try:
        with path.open("rb") as image_file:
            header = image_file.read(24)
    except OSError as error:
        raise VisualAcceptanceError(f"Unable to read screenshot {path}: {error}") from error

    if len(header) != 24 or header[:8] != PNG_SIGNATURE or header[12:16] != b"IHDR":
        raise VisualAcceptanceError(f"Screenshot is not a valid PNG: {path}")
    return struct.unpack(">II", header[16:24])


def validate_image(path: Path, contract: dict[str, Any]) -> tuple[int, int]:
    if not path.is_file():
        raise VisualAcceptanceError(f"Screenshot does not exist: {path}")

    expected = contract["expected_image"]
    minimum_size = int(expected.get("minimum_file_size_bytes", 1))
    if path.stat().st_size < minimum_size:
        raise VisualAcceptanceError(
            f"Screenshot is suspiciously small: {path.stat().st_size} bytes; expected at least {minimum_size}."
        )

    dimensions = read_png_dimensions(path)
    expected_dimensions = (int(expected["width"]), int(expected["height"]))
    if dimensions != expected_dimensions:
        raise VisualAcceptanceError(f"Unexpected screenshot dimensions: {dimensions}; expected {expected_dimensions}.")
    return dimensions


def load_render_metadata(path: Path) -> dict[str, Any]:
    try:
        metadata = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise VisualAcceptanceError(f"Unable to read graphics metadata {path}: {error}") from error

    required_fields = {
        "version",
        "capture_method",
        "grab_target_scaled_by_device_pixel_ratio",
        "requested_graphics_api",
        "actual_graphics_api",
        "qt_version",
        "logical_window_width",
        "logical_window_height",
        "offscreen_target_width",
        "offscreen_target_height",
        "effective_device_pixel_ratio",
        "captured_pixel_width",
        "captured_pixel_height",
    }
    missing_fields = required_fields.difference(metadata) if isinstance(metadata, dict) else required_fields
    if missing_fields:
        raise VisualAcceptanceError(f"Graphics metadata is missing fields: {sorted(missing_fields)}")
    if metadata["version"] != 3:
        raise VisualAcceptanceError(f"Unsupported graphics metadata version: {metadata['version']}")
    if metadata["capture_method"] != "item_grab_to_image":
        raise VisualAcceptanceError(f"Unsupported screenshot capture method: {metadata['capture_method']}")

    requested_api = metadata["requested_graphics_api"]
    actual_api = metadata["actual_graphics_api"]
    if requested_api not in VALID_GRAPHICS_APIS or actual_api not in VALID_GRAPHICS_APIS:
        raise VisualAcceptanceError(f"Graphics metadata contains an unsupported API: requested={requested_api}, actual={actual_api}")
    if requested_api != actual_api:
        raise VisualAcceptanceError(f"Requested graphics API {requested_api}, but the screenshot used {actual_api}.")

    expected_api = os.environ.get("QACCELPLOT_EXPECTED_GRAPHICS_API", "").strip().lower()
    if expected_api and actual_api != expected_api:
        raise VisualAcceptanceError(f"CI expected graphics API {expected_api}, but the screenshot used {actual_api}.")
    if not isinstance(metadata["qt_version"], str) or not metadata["qt_version"].strip():
        raise VisualAcceptanceError("Graphics metadata contains no Qt version.")
    expected_qt_version = os.environ.get("QACCELPLOT_EXPECTED_QT_VERSION", "").strip()
    if expected_qt_version and metadata["qt_version"] != expected_qt_version:
        raise VisualAcceptanceError(
            f"CI expected Qt {expected_qt_version}, but the screenshot used Qt {metadata['qt_version']}."
        )
    return metadata


def validate_capture_resolution(metadata: dict[str, Any], image_dimensions: tuple[int, int]) -> None:
    grab_target_scaled = metadata["grab_target_scaled_by_device_pixel_ratio"]
    if not isinstance(grab_target_scaled, bool):
        raise VisualAcceptanceError(
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
            raise VisualAcceptanceError(f"Graphics metadata contains an invalid {field}: {value}")

    effective_device_pixel_ratio = metadata["effective_device_pixel_ratio"]
    if (
        not isinstance(effective_device_pixel_ratio, (int, float))
        or isinstance(effective_device_pixel_ratio, bool)
        or effective_device_pixel_ratio <= 0
    ):
        raise VisualAcceptanceError(
            f"Graphics metadata contains an invalid effective_device_pixel_ratio: {effective_device_pixel_ratio}"
        )

    logical_size = (metadata["logical_window_width"], metadata["logical_window_height"])
    captured_size = (metadata["captured_pixel_width"], metadata["captured_pixel_height"])
    if captured_size != logical_size:
        raise VisualAcceptanceError(
            f"Screenshot was not rendered at its logical size: captured {captured_size}; logical window {logical_size}."
        )
    grab_scale = effective_device_pixel_ratio if grab_target_scaled else 1.0
    expected_capture_size = (
        round(metadata["offscreen_target_width"] * grab_scale),
        round(metadata["offscreen_target_height"] * grab_scale),
    )
    if captured_size != expected_capture_size:
        raise VisualAcceptanceError(
            f"Offscreen target predicts {expected_capture_size} pixels; captured {captured_size}."
        )
    if image_dimensions != captured_size:
        raise VisualAcceptanceError(f"PNG dimensions {image_dimensions} do not match captured pixels {captured_size}.")


def build_prompt(contract: dict[str, Any], render_metadata: dict[str, Any] | None = None) -> str:
    render_context = ""
    if render_metadata:
        render_context = "\n\nRender context:\n" + json.dumps(render_metadata, indent=2, ensure_ascii=False)
    return """
You are a strict visual QA judge for a plotting-library example. Judge the supplied
screenshot on its own against the visual contract below. There is no baseline image.

Assess only properties that are visibly observable. Do not claim that exact numerical
data is correct from appearance alone. Use `uncertain` when the screenshot does not
provide enough visual evidence. Minor sub-pixel antialiasing and small platform-specific
spacing differences are acceptable. A low-resolution text-rasterization defect requires
concrete visible evidence such as square pixel blocks, doubled or jagged stair-step glyph
edges, or uniformly upscaled glyphs. Such a defect must fail the text_raster_quality
check. Do not infer a rasterization defect merely from a smaller font size, lighter font
weight, lower contrast, gray coloring, rotation, or normal antialiasing. Compare text
sharpness only between text with genuinely comparable size, weight, orientation, and
contrast. Do not penalize variations explicitly allowed by the contract.

Return exactly one result for every contract check ID. Evidence must identify the visible
feature and its approximate panel or screen location. A `fail` requires a concrete visible
defect, not merely a possible concern. If a label, tick ordering, or small detail is too
small to verify and there is no sharper nearby UI text for comparison, report `uncertain`
rather than `fail`. When plot labels show concrete low-resolution artifacts, or remain
materially blurrier than nearby text with genuinely comparable typography, report `fail`
for text_raster_quality rather than `uncertain`. Do not treat inability to verify another
requirement as evidence that it is wrong. Mark other checks as `fail` only when the
screenshot visibly contradicts the expectation (for example, raw numeric labels are
clearly shown where formatted labels are required, or axes actually overlap).

Visual contract:
""".strip() + "\n" + json.dumps(contract, indent=2, ensure_ascii=False) + render_context


def validate_model_response(result: Any, contract: dict[str, Any]) -> dict[str, Any]:
    if not isinstance(result, dict) or not isinstance(result.get("summary"), str) or not isinstance(result.get("checks"), list):
        raise VisualAcceptanceError("AI response does not contain a summary and checks array.")

    expected_ids = {check["id"] for check in contract["checks"]}
    seen_ids: set[str] = set()
    for check in result["checks"]:
        if not isinstance(check, dict):
            raise VisualAcceptanceError("AI response contains a malformed check.")
        check_id = check.get("id")
        if check_id not in expected_ids:
            raise VisualAcceptanceError(f"AI response contains an unknown check ID: {check_id}")
        if check_id in seen_ids:
            raise VisualAcceptanceError(f"AI response contains duplicate check ID: {check_id}")
        seen_ids.add(check_id)
        if check.get("status") not in VALID_STATUSES:
            raise VisualAcceptanceError(f"AI response contains an invalid status for {check_id}.")
        confidence = check.get("confidence")
        if not isinstance(confidence, (int, float)) or isinstance(confidence, bool) or not 0.0 <= confidence <= 1.0:
            raise VisualAcceptanceError(f"AI response contains invalid confidence for {check_id}.")
        if not isinstance(check.get("evidence"), str) or not check["evidence"].strip():
            raise VisualAcceptanceError(f"AI response contains no evidence for {check_id}.")

    missing_ids = expected_ids.difference(seen_ids)
    if missing_ids:
        raise VisualAcceptanceError(f"AI response omitted contract checks: {sorted(missing_ids)}")
    return result


def evaluate_result(result: dict[str, Any], contract: dict[str, Any]) -> dict[str, Any]:
    contract_checks = {check["id"]: check for check in contract["checks"]}
    policy = contract["policy"]
    threshold = float(policy["minimum_confidence"])
    blocking_severities = set(policy["blocking_severities"])

    evaluated_checks: list[dict[str, Any]] = []
    blocking_failures: list[str] = []
    for model_check in result["checks"]:
        contract_check = contract_checks[model_check["id"]]
        evaluated = {
            **model_check,
            "severity": contract_check["severity"],
            "blocking": model_check["status"] == "fail"
            and contract_check["severity"] in blocking_severities
            and float(model_check["confidence"]) >= threshold,
        }
        evaluated_checks.append(evaluated)
        if evaluated["blocking"]:
            blocking_failures.append(model_check["id"])

    return {
        "verdict": "fail" if blocking_failures else "pass",
        "summary": result["summary"],
        "blocking_failures": blocking_failures,
        "checks": evaluated_checks,
    }


def build_response_schema(contract: dict[str, Any]) -> dict[str, Any]:
    schema = copy.deepcopy(MODEL_RESPONSE_SCHEMA)
    expected_ids = [check["id"] for check in contract["checks"]]
    checks_schema = schema["properties"]["checks"]
    checks_schema["minItems"] = len(expected_ids)
    checks_schema["maxItems"] = len(expected_ids)
    checks_schema["items"]["properties"]["id"]["enum"] = expected_ids
    return schema


def call_openai(
    image_path: Path,
    contract: dict[str, Any],
    model: str,
    api_key: str,
    render_metadata: dict[str, Any] | None = None,
) -> dict[str, Any]:
    try:
        from openai import OpenAI
    except ImportError as error:
        raise VisualAcceptanceError("openai is not installed. Install it with `pip install openai`.") from error

    try:
        image_data = base64.b64encode(image_path.read_bytes()).decode("ascii")
        client = OpenAI(api_key=api_key)
        response = client.responses.create(
            model=model,
            input=[
                {
                    "role": "user",
                    "content": [
                        {"type": "input_text", "text": build_prompt(contract, render_metadata)},
                        {
                            "type": "input_image",
                            "image_url": f"data:image/png;base64,{image_data}",
                            "detail": "high",
                        },
                    ],
                }
            ],
            reasoning={"effort": "none"},
            text={
                "format": {
                    "type": "json_schema",
                    "name": "qaccelplot_visual_acceptance",
                    "strict": True,
                    "schema": build_response_schema(contract),
                }
            },
            max_output_tokens=4096,
            store=False,
        )
    except Exception as error:
        raise VisualAcceptanceError(f"OpenAI request failed: {error}") from error

    raw_response = getattr(response, "output_text", "")
    try:
        parsed = json.loads(raw_response)
    except (TypeError, json.JSONDecodeError) as error:
        raise VisualAcceptanceError(f"Unable to parse OpenAI response: {raw_response or '<no text response>'}") from error
    return validate_model_response(parsed, contract)


def write_report(path: Path, report: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Judge a screenshot against a reference-free visual contract.")
    parser.add_argument("--image", required=True, type=Path, help="PNG screenshot to inspect")
    parser.add_argument("--contract", required=True, type=Path, help="Visual contract JSON")
    parser.add_argument("--metadata", type=Path, help="Graphics metadata JSON; defaults to <image>.rhi.json")
    parser.add_argument("--report", type=Path, help="Where to write the JSON report")
    parser.add_argument("--model", default=os.environ.get("AI_VISUAL_MODEL", "gpt-5.4-nano"))
    parser.add_argument("--validate-only", action="store_true", help="Validate local inputs without calling OpenAI")
    return parser.parse_args()


def main() -> int:
    configure_console_encoding()
    args = parse_arguments()
    try:
        contract = load_contract(args.contract)
        dimensions = validate_image(args.image, contract)
        metadata_path = args.metadata or Path(f"{args.image}.rhi.json")
        render_metadata = load_render_metadata(metadata_path)
        validate_capture_resolution(render_metadata, dimensions)
        if args.validate_only:
            print(
                f"PASS: {render_metadata['actual_graphics_api']} / Qt {render_metadata['qt_version']} "
                f"contract and {dimensions[0]}x{dimensions[1]} screenshot are valid."
            )
            return 0

        api_key = os.environ.get("OPENAI_API_KEY")
        if not api_key:
            raise VisualAcceptanceError("OPENAI_API_KEY environment variable is not set.")

        model_result = call_openai(args.image, contract, args.model, api_key, render_metadata)
        decision = evaluate_result(model_result, contract)
        report = {
            "contract": contract["name"],
            "image": str(args.image),
            "image_dimensions": {"width": dimensions[0], "height": dimensions[1]},
            "rendering": render_metadata,
            "model": args.model,
            **decision,
        }
        if args.report:
            write_report(args.report, report)

        print(f"{decision['verdict'].upper()}: {decision['summary']}")
        for check in decision["checks"]:
            print(
                f"- {check['id']}: {check['status']} "
                f"(confidence={check['confidence']:.2f}, severity={check['severity']}) — {check['evidence']}"
            )
        return 1 if decision["verdict"] == "fail" else 0
    except VisualAcceptanceError as error:
        if args.report:
            write_report(args.report, {"verdict": "error", "error": str(error)})
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
