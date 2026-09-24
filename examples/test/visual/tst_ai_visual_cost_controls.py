#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest


VISUAL_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = VISUAL_DIR.parent.parent.parent
sys.path.insert(0, str(VISUAL_DIR))

from ai_visual_acceptance import VisualAcceptanceError  # noqa: E402
from ai_visual_matrix import (  # noqa: E402
    Capture,
    capture_label,
    discover_captures,
    markdown_summary,
    parse_scenarios,
    select_captures,
)
from visual_scenarios import discover_scenarios  # noqa: E402


def load_impact_module():
    path = PROJECT_ROOT / ".github" / "scripts" / "ai_visual_impact.py"
    spec = importlib.util.spec_from_file_location("ai_visual_impact", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def write_contract(contracts_dir, example, scenario, **fields):
    path = contracts_dir / example / f"{scenario}.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({"name": f"{example}/{scenario}", **fields}), encoding="utf-8")


class ImpactRoutingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.impact = load_impact_module()

    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.contracts = Path(directory.name)
        write_contract(self.contracts, "quickstart", "default")
        write_contract(self.contracts, "styling", "lines", sources=["qml/LinesPage.qml"])
        write_contract(self.contracts, "styling", "gaps", sources=["qml/GapsPage.qml"])
        write_contract(self.contracts, "pulsar", "default", ai_inspection=False)

    def classify(self, paths):
        return self.impact.classify_paths(paths, self.contracts)

    def test_nonvisual_changes_skip_paid_review(self):
        result = self.classify(["docs/guide/index.md", "README.md"])
        self.assertEqual(result["scope"], "none")
        self.assertEqual(result["scenarios"], [])

    def test_example_and_contract_changes_select_only_affected_scenarios(self):
        result = self.classify(
            [
                "examples/quickstart/main.qml",
                "examples/test/visual/contracts/styling/gaps.json",
            ]
        )
        self.assertEqual(result["scope"], "selected")
        self.assertEqual(result["scenarios"], ["quickstart/default", "styling/gaps"])

    def test_page_source_change_selects_only_its_scenario(self):
        result = self.classify(["examples/styling/qml/GapsPage.qml"])
        self.assertEqual(result["scenarios"], ["styling/gaps"])

    def test_unclaimed_example_file_selects_every_scenario_of_the_example(self):
        result = self.classify(["examples/styling/qml/main.qml", "examples/styling/src/main.cpp"])
        self.assertEqual(result["scenarios"], ["styling/gaps", "styling/lines"])

    def test_shared_rendering_change_selects_every_ai_scenario(self):
        result = self.classify(["QAccelPlot/src/QAccelPlot/QAccelPlot.cpp"])
        self.assertEqual(result["scope"], "all")
        self.assertEqual(result["scenarios"], ["quickstart/default", "styling/gaps", "styling/lines"])

    def test_scenarios_without_ai_inspection_are_never_selected(self):
        result = self.classify(["examples/pulsar/src/main.cpp", "examples/test/visual/contracts/pulsar/default.json"])
        self.assertEqual(result["scope"], "none")

    def test_examples_inside_category_folders_are_routable(self):
        result = self.classify(["examples/showcases/pulsar/src/main.cpp", "examples/category/styling/qml/GapsPage.qml"])
        self.assertEqual(result["scenarios"], ["styling/gaps"])

    def test_repository_contracts_are_routable(self):
        result = self.impact.classify_paths(["examples/styling/styling_and_transitions/qml/StylingPage.qml"])
        self.assertEqual(result["scenarios"], ["styling_and_transitions/styling"])


class MatrixSelectionTests(unittest.TestCase):
    @staticmethod
    def capture(qt_version, backend, os_name, suffix):
        return Capture(
            scenario="quickstart/default",
            image=Path(f"{suffix}.png"),
            metadata=Path(f"{suffix}.png.rhi.json"),
            qt_version=qt_version,
            backend=backend,
            os_name=os_name,
        )

    def test_representative_mode_selects_canonical_and_outliers(self):
        canonical = self.capture("6.8.3", "opengl", "ubuntu-24.04", "canonical")
        covered = self.capture("6.11.2", "metal", "macos-15", "covered")
        outlier = self.capture("6.2.4", "d3d11", "windows-2025", "outlier")
        scores = {covered.image: 0.02, outlier.image: 0.20}

        selected, manifest = select_captures(
            [covered, outlier, canonical],
            "representative",
            {"quickstart/default"},
            0.08,
            compare=lambda _reference, candidate: scores[candidate],
        )

        self.assertEqual(selected, [canonical, outlier])
        self.assertEqual(len(manifest), 3)
        self.assertEqual([entry["reason"] for entry in manifest], ["canonical", "covered by canonical", "visual outlier"])

    def test_full_mode_preserves_every_eligible_capture(self):
        captures = [
            self.capture("6.8.3", "opengl", "ubuntu-24.04", "one"),
            self.capture("6.11.2", "metal", "macos-15", "two"),
        ]
        selected, _manifest = select_captures(captures, "full", {"quickstart/default"}, 0.08)
        self.assertEqual(selected, captures)

    def test_scenario_filter_accepts_examples_and_scenarios(self):
        with tempfile.TemporaryDirectory() as directory:
            contracts = Path(directory)
            write_contract(contracts, "quickstart", "default")
            write_contract(contracts, "styling", "lines")
            write_contract(contracts, "styling", "gaps")
            scenarios = discover_scenarios(contracts)

            self.assertIsNone(parse_scenarios("all", scenarios))
            self.assertEqual(parse_scenarios("styling", scenarios), {"styling/lines", "styling/gaps"})
            self.assertEqual(parse_scenarios("quickstart/default,styling/gaps", scenarios), {"quickstart/default", "styling/gaps"})
            with self.assertRaisesRegex(VisualAcceptanceError, "Unknown AI visual scenarios"):
                parse_scenarios("quickstart,styling/not_a_page", scenarios)

    def test_nested_artifact_layout_is_discovered(self):
        with tempfile.TemporaryDirectory() as directory:
            contracts = Path(directory) / "contracts"
            write_contract(contracts, "quickstart", "default")
            write_contract(contracts, "pulsar", "default", ai_inspection=False)
            artifact = Path(directory) / "visual-6.8.3-opengl-ubuntu-24.04"
            screenshots = artifact / "examples" / "test" / "screenshots"
            screenshots.mkdir(parents=True)
            # Smoke-only scenarios have no contract and opted-out scenarios are never judged.
            for stem in ("quickstart__default", "pulsar__default", "quickstart__smoke_only", "LegacyName"):
                image = screenshots / f"{stem}.png"
                image.write_bytes(b"not parsed during discovery")
                Path(f"{image}.rhi.json").write_text("{}", encoding="utf-8")

            captures = discover_captures(artifact.parent, discover_scenarios(contracts))

        self.assertEqual(len(captures), 1)
        self.assertEqual(captures[0].scenario, "quickstart/default")
        self.assertEqual(captures[0].backend, "opengl")

    def test_opengl_es_arm_captures_are_compared_against_the_canonical_render(self):
        captures = [
            self.capture("6.8.3", "opengles3", "ubuntu-24.04-arm", "arm"),
            self.capture("6.8.3", "opengl", "ubuntu-24.04", "canonical"),
        ]
        selected, manifest = select_captures(captures, "representative", None, 0.08, compare=lambda _a, _b: 0.01)

        self.assertEqual([capture.os_name for capture in selected], ["ubuntu-24.04"])
        self.assertEqual(manifest[1]["capture"], "quickstart/default:6.8.3:opengles3:ubuntu-24.04-arm")
        self.assertEqual(manifest[1]["reason"], "covered by canonical")

    def test_summary_labels_opengl_es_captures(self):
        report = {
            "contract": "quickstart",
            "rendering": {"qt_version": "6.8.3", "actual_graphics_api": "opengl", "opengl_es": True, "opengl_major_version": 3},
            "matrix": {"os_name": "ubuntu-24.04-arm"},
        }
        self.assertEqual(capture_label(report), "quickstart / Qt 6.8.3 / opengles3 / ubuntu-24.04-arm")

    def test_summary_explains_failed_checks(self):
        report = {
            "contract": "measurement_tools",
            "verdict": "fail",
            "rendering": {"qt_version": "6.2.4", "actual_graphics_api": "opengl"},
            "matrix": {"os_name": "ubuntu-24.04"},
            "usage": {
                "input_tokens": 2000,
                "cached_input_tokens": 0,
                "output_tokens": 200,
                "reasoning_tokens": 0,
                "total_tokens": 2200,
            },
            "estimated_cost_usd": 0.00065,
            "checks": [
                {
                    "id": "terrain_plot",
                    "status": "fail",
                    "severity": "high",
                    "confidence": 0.93,
                    "blocking": True,
                    "evidence": "Terrain curve is missing | lower plot.",
                }
            ],
        }

        summary = markdown_summary([report], [{"capture": "candidate"}], "gpt-5.6-luna")

        self.assertIn("- Failed renders: 1", summary)
        self.assertIn("## Failed AI checks", summary)
        self.assertIn("measurement_tools / Qt 6.2.4 / opengl / ubuntu-24.04", summary)
        self.assertIn("`terrain_plot`", summary)
        self.assertIn("| high | 0.93 | yes |", summary)
        self.assertIn("Terrain curve is missing \\| lower plot.", summary)


if __name__ == "__main__":
    unittest.main()
