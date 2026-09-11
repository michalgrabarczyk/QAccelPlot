#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


VISUAL_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = VISUAL_DIR.parent.parent.parent
sys.path.insert(0, str(VISUAL_DIR))

from ai_visual_acceptance import VisualAcceptanceError  # noqa: E402
from ai_visual_matrix import Capture, discover_captures, parse_examples, select_captures  # noqa: E402


def load_impact_module():
    path = PROJECT_ROOT / ".github" / "scripts" / "ai_visual_impact.py"
    spec = importlib.util.spec_from_file_location("ai_visual_impact", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class ImpactRoutingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.impact = load_impact_module()

    def test_nonvisual_changes_skip_paid_review(self):
        result = self.impact.classify_paths(["docs/guide/index.md", "README.md"])
        self.assertEqual(result["scope"], "none")
        self.assertEqual(result["examples"], [])

    def test_example_and_contract_changes_select_only_affected_examples(self):
        result = self.impact.classify_paths(
            [
                "examples/quickstart/main.qml",
                "examples/test/visual/contracts/realtime.json",
            ]
        )
        self.assertEqual(result["scope"], "selected")
        self.assertEqual(result["examples"], ["quickstart", "realtime"])

    def test_shared_rendering_change_selects_all_examples(self):
        result = self.impact.classify_paths(["QAccelPlot/src/QAccelPlot.cpp"])
        self.assertEqual(result["scope"], "all")
        self.assertEqual(set(result["examples"]), self.impact.EXAMPLES)


class MatrixSelectionTests(unittest.TestCase):
    @staticmethod
    def capture(qt_version, backend, os_name, suffix):
        return Capture(
            contract="quickstart",
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
            {"quickstart"},
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
        selected, _manifest = select_captures(captures, "full", {"quickstart"}, 0.08)
        self.assertEqual(selected, captures)

    def test_unknown_example_is_rejected(self):
        with self.assertRaisesRegex(VisualAcceptanceError, "Unknown AI visual examples"):
            parse_examples("quickstart,not_an_example")

    def test_nested_artifact_layout_is_discovered(self):
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "visual-6.8.3-opengl-ubuntu-24.04"
            screenshots = artifact / "examples" / "test" / "screenshots"
            screenshots.mkdir(parents=True)
            image = screenshots / "QAccelPlotExampleQuickstart.png"
            image.write_bytes(b"not parsed during discovery")
            Path(f"{image}.rhi.json").write_text("{}", encoding="utf-8")

            captures = discover_captures(Path(directory))

        self.assertEqual(len(captures), 1)
        self.assertEqual(captures[0].contract, "quickstart")
        self.assertEqual(captures[0].backend, "opengl")


if __name__ == "__main__":
    unittest.main()
