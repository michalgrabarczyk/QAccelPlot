#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Aggregate matrix screenshots and pay for only representative AI judgments."""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
import math
import os
from pathlib import Path
import re
from typing import Callable

from ai_visual_acceptance import VisualAcceptanceError, judge_image, load_pricing, write_report


TARGET_TO_CONTRACT = {
    "QAccelPlotExampleAnnotations": "annotations",
    "QAccelPlotExampleAxisFormats": "axis_formats",
    "QAccelPlotExampleCustomAxis": "custom_axis",
    "QAccelPlotExampleInteractiveTools": "interactive_tools",
    "QAccelPlotExamplePerformanceShowcase": "performance_showcase",
    "QAccelPlotExampleQuickstart": "quickstart",
    "QAccelPlotExampleRealtime": "realtime",
    "QAccelPlotExampleStylingAndTransitions": "styling_and_transitions",
}

ARTIFACT_PATTERN = re.compile(r"^visual-(?P<qt>[^-]+)-(?P<backend>[^-]+)-(?P<os>.+)$")


@dataclass(frozen=True)
class Capture:
    contract: str
    image: Path
    metadata: Path
    qt_version: str
    backend: str
    os_name: str

    @property
    def identity(self) -> str:
        return f"{self.contract}:{self.qt_version}:{self.backend}:{self.os_name}"


def discover_captures(artifacts_dir: Path) -> list[Capture]:
    captures: list[Capture] = []
    for artifact_dir in sorted(artifacts_dir.glob("visual-*")):
        match = ARTIFACT_PATTERN.fullmatch(artifact_dir.name)
        if not match:
            continue
        for image_path in sorted(artifact_dir.rglob("*.png")):
            if image_path.parent.name != "screenshots":
                continue
            contract = TARGET_TO_CONTRACT.get(image_path.stem)
            if not contract:
                continue
            metadata_path = Path(f"{image_path}.rhi.json")
            if not metadata_path.is_file():
                raise VisualAcceptanceError(f"Missing render metadata for {image_path}")
            captures.append(
                Capture(
                    contract=contract,
                    image=image_path,
                    metadata=metadata_path,
                    qt_version=match.group("qt"),
                    backend=match.group("backend"),
                    os_name=match.group("os"),
                )
            )
    if not captures:
        raise VisualAcceptanceError(f"No AI-eligible screenshots found below {artifacts_dir}")
    return captures


def canonical_order(capture: Capture) -> tuple[int, int, int, str]:
    return (
        0 if capture.qt_version == "6.8.3" else 1,
        0 if capture.backend == "opengl" else 1,
        0 if capture.os_name == "ubuntu-24.04" else 1,
        capture.identity,
    )


def image_difference(reference_path: Path, candidate_path: Path) -> float:
    try:
        from PIL import Image, ImageChops, ImageFilter, ImageStat
    except ImportError as error:
        raise VisualAcceptanceError("Pillow is required for matrix image comparison.") from error

    try:
        with Image.open(reference_path) as reference_source, Image.open(candidate_path) as candidate_source:
            reference = reference_source.convert("RGB").resize((256, 256))
            candidate = candidate_source.convert("RGB").resize((256, 256))
    except OSError as error:
        raise VisualAcceptanceError(f"Unable to compare matrix screenshots: {error}") from error
    pixel_rms = sum(ImageStat.Stat(ImageChops.difference(reference, candidate)).rms) / (3 * 255)

    reference_edges = ImageStat.Stat(reference.convert("L").filter(ImageFilter.FIND_EDGES)).rms[0]
    candidate_edges = ImageStat.Stat(candidate.convert("L").filter(ImageFilter.FIND_EDGES)).rms[0]
    sharpness_delta = min(abs(math.log((candidate_edges + 1) / (reference_edges + 1))) / 2.5, 1.0)
    return max(pixel_rms, sharpness_delta)


def parse_examples(value: str) -> set[str] | None:
    if value.strip().lower() == "all":
        return None
    examples = {item.strip() for item in value.split(",") if item.strip()}
    unknown = examples.difference(TARGET_TO_CONTRACT.values())
    if unknown:
        raise VisualAcceptanceError(f"Unknown AI visual examples: {sorted(unknown)}")
    return examples


def select_captures(
    captures: list[Capture],
    mode: str,
    eligible_examples: set[str] | None,
    threshold: float,
    compare: Callable[[Path, Path], float] = image_difference,
) -> tuple[list[Capture], list[dict[str, object]]]:
    eligible = [capture for capture in captures if eligible_examples is None or capture.contract in eligible_examples]
    if mode == "full":
        return eligible, [
            {"capture": capture.identity, "selected": True, "reason": "full mode"}
            for capture in eligible
        ]

    selected: list[Capture] = []
    manifest: list[dict[str, object]] = []
    contracts = sorted({capture.contract for capture in eligible})
    for contract in contracts:
        contract_captures = sorted(
            (capture for capture in eligible if capture.contract == contract), key=canonical_order
        )
        canonical = contract_captures[0]
        selected.append(canonical)
        manifest.append(
            {"capture": canonical.identity, "selected": True, "reason": "canonical"}
        )
        for candidate in contract_captures[1:]:
            score = compare(canonical.image, candidate.image)
            is_outlier = score > threshold
            if is_outlier:
                selected.append(candidate)
            manifest.append(
                {
                    "capture": candidate.identity,
                    "selected": is_outlier,
                    "reason": "visual outlier" if is_outlier else "covered by canonical",
                    "difference_score": round(score, 6),
                    "canonical": canonical.identity,
                }
            )
    return selected, manifest


def markdown_summary(
    reports: list[dict[str, object]],
    manifest: list[dict[str, object]],
    model: str,
) -> str:
    totals = {
        key: sum(int(report["usage"][key]) for report in reports)
        for key in ("input_tokens", "cached_input_tokens", "output_tokens", "reasoning_tokens", "total_tokens")
    }
    total_cost = sum(float(report["estimated_cost_usd"]) for report in reports)
    failed_render_count = sum(report["verdict"] == "fail" for report in reports)
    lines = [
        "## AI visual acceptance",
        "",
        f"- Model: `{model}`",
        f"- Paid requests: {len(reports)} of {len(manifest)} captured candidates",
        f"- Failed renders: {failed_render_count}",
        f"- Tokens: {totals['input_tokens']} input ({totals['cached_input_tokens']} cached), "
        f"{totals['output_tokens']} output, {totals['reasoning_tokens']} reasoning",
        f"- Estimated API cost: ${total_cost:.6f}",
        "",
        "| Capture | Verdict | Input | Cached | Output | Estimated cost |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for report in reports:
        usage = report["usage"]
        lines.append(
            f"| {capture_label(report)} | {report['verdict']} | {usage['input_tokens']} | "
            f"{usage['cached_input_tokens']} | {usage['output_tokens']} | "
            f"${float(report['estimated_cost_usd']):.6f} |"
        )
    lines.extend(failed_checks_table(reports))
    return "\n".join(lines)


def capture_label(report: dict[str, object]) -> str:
    rendering = report["rendering"]
    matrix = report.get("matrix", {})
    os_name = matrix.get("os_name", "unknown OS")
    return (
        f"{report['contract']} / Qt {rendering['qt_version']} / "
        f"{rendering['actual_graphics_api']} / {os_name}"
    )


def markdown_cell(value: object) -> str:
    return " ".join(str(value).splitlines()).replace("|", "\\|")


def failed_checks_table(reports: list[dict[str, object]]) -> list[str]:
    failed_checks = [
        (report, check)
        for report in reports
        for check in report["checks"]
        if check["status"] == "fail"
    ]
    lines = ["", "## Failed AI checks", ""]
    if not failed_checks:
        return lines + ["No AI checks failed.", ""]

    lines.extend(
        [
            "| Capture | Failed check | Severity | Confidence | Blocking | Reason |",
            "|---|---|---:|---:|---:|---|",
        ]
    )
    for report, check in sorted(
        failed_checks,
        key=lambda item: (not item[1]["blocking"], capture_label(item[0]), item[1]["id"]),
    ):
        lines.append(
            f"| {capture_label(report)} | `{check['id']}` | {check['severity']} | "
            f"{float(check['confidence']):.2f} | {'yes' if check['blocking'] else 'no'} | "
            f"{markdown_cell(check['evidence'])} |"
        )
    lines.append("")
    return lines


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Judge representative screenshots from the RHI matrix.")
    parser.add_argument("--artifacts", required=True, type=Path)
    parser.add_argument("--contracts", required=True, type=Path)
    parser.add_argument("--reports", required=True, type=Path)
    parser.add_argument("--summary", required=True, type=Path)
    parser.add_argument("--mode", choices=("representative", "full"), default="representative")
    parser.add_argument("--examples", default="all", help="Comma-separated contract names or all")
    parser.add_argument("--model", default=os.environ.get("AI_VISUAL_MODEL", "gpt-5.4-nano"))
    parser.add_argument("--outlier-threshold", type=float, default=0.08)
    parser.add_argument("--max-requests", type=int, default=24)
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    args.reports.mkdir(parents=True, exist_ok=True)
    try:
        captures = discover_captures(args.artifacts)
        selected, manifest = select_captures(
            captures,
            args.mode,
            parse_examples(args.examples),
            args.outlier_threshold,
        )
        write_report(args.reports / "selection.json", {"mode": args.mode, "captures": manifest})
        if not selected:
            args.summary.write_text("## AI visual acceptance\n\nNo changed examples require paid judgment.\n", encoding="utf-8")
            return 0
        if args.mode != "full" and args.max_requests > 0 and len(selected) > args.max_requests:
            raise VisualAcceptanceError(
                f"Representative selection requires {len(selected)} requests, above the {args.max_requests}-request "
                "budget. Inspect the artifacts or dispatch full mode."
            )

        api_key = os.environ.get("OPENAI_API_KEY")
        if not api_key:
            raise VisualAcceptanceError("OPENAI_API_KEY environment variable is not set.")

        pricing = load_pricing()
        reports: list[dict[str, object]] = []
        had_failure = False
        for capture in selected:
            contract_path = args.contracts / f"{capture.contract}.json"
            report = judge_image(
                capture.image,
                contract_path,
                capture.metadata,
                args.model,
                api_key,
                pricing,
            )
            report["matrix"] = asdict(capture) | {"image": str(capture.image), "metadata": str(capture.metadata)}
            report_path = args.reports / f"{capture.contract}-{capture.qt_version}-{capture.backend}-{capture.os_name}.json"
            write_report(report_path, report)
            reports.append(report)
            had_failure = had_failure or report["verdict"] == "fail"

        summary = markdown_summary(reports, manifest, args.model)
        args.summary.write_text(summary, encoding="utf-8")
        print(summary)
        return 1 if had_failure else 0
    except VisualAcceptanceError as error:
        message = f"## AI visual acceptance\n\nERROR: {error}\n"
        args.summary.write_text(message, encoding="utf-8")
        print(message)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
