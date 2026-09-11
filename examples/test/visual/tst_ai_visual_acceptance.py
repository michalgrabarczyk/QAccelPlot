#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
import io
import json
import os
import re
import runpy
import sys
import tempfile
import types
from pathlib import Path
import unittest
from unittest import mock


VISUAL_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = VISUAL_DIR.parent.parent.parent
sys.path.insert(0, str(VISUAL_DIR))

from ai_visual_acceptance import (  # noqa: E402
    VisualAcceptanceError,
    build_prompt,
    call_openai,
    configure_console_encoding,
    estimate_cost_usd,
    evaluate_result,
    load_contract,
    load_render_metadata,
    validate_capture_resolution,
    validate_model_response,
)


CONTRACTS_DIR = VISUAL_DIR / "contracts"
EXAMPLES_DIR = PROJECT_ROOT / "examples"
EXAMPLE_TESTS_DIR = VISUAL_DIR.parent
CONTRACT_PATH = CONTRACTS_DIR / "styling_and_transitions.json"
AI_WORKFLOW_PATH = PROJECT_ROOT / ".github" / "workflows" / "ai-regression.yml"


def camel_to_snake(value):
    return re.sub(r"(?<!^)(?=[A-Z])", "_", value).lower()


def example_contract_names():
    example_names = set()
    for cmake_path in EXAMPLES_DIR.glob("*/CMakeLists.txt"):
        match = re.search(r"qaccelplot_add_example\(\s*([A-Za-z0-9_]+)", cmake_path.read_text(encoding="utf-8"))
        if match:
            example_names.add(camel_to_snake(match.group(1)))
    return example_names


class VisualAcceptanceTests(unittest.TestCase):
    def setUp(self):
        self.contract = load_contract(CONTRACT_PATH)

    def model_result(self, *, failed_id=None, confidence=0.95):
        checks = []
        for contract_check in self.contract["checks"]:
            failed = contract_check["id"] == failed_id
            checks.append(
                {
                    "id": contract_check["id"],
                    "status": "fail" if failed else "pass",
                    "confidence": confidence,
                    "evidence": "Visible test evidence.",
                }
            )
        return {"summary": "Test assessment.", "checks": checks}

    def render_metadata(self, *, requested="opengl", actual="opengl"):
        return {
            "version": 3,
            "capture_method": "item_grab_to_image",
            "grab_target_scaled_by_device_pixel_ratio": True,
            "requested_graphics_api": requested,
            "actual_graphics_api": actual,
            "qt_version": "6.7.3",
            "logical_window_width": 880,
            "logical_window_height": 1120,
            "offscreen_target_width": 1760,
            "offscreen_target_height": 2240,
            "effective_device_pixel_ratio": 0.5,
            "captured_pixel_width": 880,
            "captured_pixel_height": 1120,
        }

    def test_every_example_has_a_valid_contract(self):
        contract_paths = list(CONTRACTS_DIR.glob("*.json"))
        contract_names = {path.stem for path in contract_paths}
        self.assertEqual(contract_names, example_contract_names())

        cmake_source = (EXAMPLE_TESTS_DIR / "CMakeLists.txt").read_text(encoding="utf-8")
        registered_names = set(
            re.findall(r"^add_qaccelplot_example_visual_tests\([^\s]+\s+([^\s\)]+)", cmake_source, re.MULTILINE)
        )
        self.assertEqual(registered_names, contract_names)
        self.assertRegex(
            cmake_source,
            r"(?m)^add_qaccelplot_example_visual_tests\(QAccelPlotExamplePulsarShowcase\s+pulsar_showcase NO_AI_INSPECTION\)$",
        )

        for path in contract_paths:
            with self.subTest(contract=path.name):
                contract = load_contract(path)
                self.assertEqual(contract["name"], path.stem)
                raster_quality_checks = [check for check in contract["checks"] if check["id"] == "text_raster_quality"]
                self.assertEqual(len(raster_quality_checks), 1)
                self.assertEqual(raster_quality_checks[0]["severity"], "high")
                self.assertIn("high", contract["policy"]["blocking_severities"])

    def test_workflow_covers_every_hardware_rhi_backend(self):
        workflow = AI_WORKFLOW_PATH.read_text(encoding="utf-8")
        for backend in ("opengl", "vulkan", "d3d11", "d3d12", "metal"):
            with self.subTest(backend=backend):
                self.assertRegex(workflow, rf"(?m)^\s+backend: {backend}$")
        version_axis = re.search(
            r"(?m)^        qt_version:\n(?P<values>(?:          - '[^']+'\n)+)",
            workflow,
        )
        self.assertIsNotNone(version_axis)
        self.assertEqual(
            re.findall(r"'([^']+)'", version_axis.group("values")),
            ["6.2.4", "6.8.3", "6.11.2"],
        )
        self.assertRegex(
            workflow,
            r"(?s)exclude:.*qt_version: '6\.2\.4'.*backend: d3d12",
        )
        self.assertIn("qt_version:", workflow)
        self.assertIn("QACCELPLOT_EXPECTED_QT_VERSION: ${{ matrix.qt_version }}", workflow)
        self.assertIn("version: ${{ matrix.qt_version }}", workflow)
        self.assertIn('cmake -S . -B build -G "Visual Studio 18 2026" -A x64', workflow)
        self.assertIn('cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -T v142', workflow)
        self.assertIn("matrix.qt_version == '6.2.4'", workflow)
        self.assertIn("matrix.qt_version != '6.2.4'", workflow)
        self.assertEqual(workflow.count("-DQACCELPLOT_INSTALL=OFF"), 3)
        self.assertEqual(workflow.count("-DQACCELPLOT_DEPLOY_EXAMPLES=OFF"), 3)
        self.assertIn("libvulkan1 libvulkan-dev mesa-vulkan-drivers vulkan-tools", workflow)
        self.assertIn("QT_SCALE_FACTOR: ${{ matrix.platform.os == 'ubuntu-24.04' && '1' || '0.5' }}", workflow)
        self.assertIn("'.github/scripts/aqt_windows_qt611.py',", workflow)
        self.assertIn("'install-qt', 'windows', 'desktop', '6.11.2'", workflow)
        self.assertIn("uses: actions/cache@v5", workflow)
        self.assertIn("if-no-files-found: error", workflow)
        self.assertIn("overwrite: true", workflow)
        self.assertIn("steps.upload-visual-artifacts.outputs.artifact-url", workflow)
        self.assertNotIn("qt_arch:", workflow)

    def test_ai_visual_inspection_only_runs_for_manual_dispatch(self):
        workflow = AI_WORKFLOW_PATH.read_text(encoding="utf-8")
        self.assertRegex(workflow, r"(?m)^  pull_request:\s*$")
        self.assertRegex(workflow, r"(?m)^  workflow_dispatch:\s*$")
        matrix_job, ai_job = workflow.split("\n  ai-judge:\n", maxsplit=1)
        self.assertNotIn("OPENAI_API_KEY", matrix_job)
        self.assertNotIn("Run OpenAI visual judges", matrix_job)
        self.assertIn("github.event_name == 'workflow_dispatch'", ai_job)
        self.assertIn("OPENAI_API_KEY: ${{ secrets.OPENAI_API_KEY }}", ai_job)
        self.assertIn("python examples/test/visual/ai_visual_matrix.py", ai_job)
        self.assertIn("--max-requests 24", ai_job)
        self.assertIn("pattern: visual-*", ai_job)

    def test_workflow_gates_paid_review_by_changed_files_and_mode(self):
        workflow = AI_WORKFLOW_PATH.read_text(encoding="utf-8")
        self.assertIn("detect-impact:", workflow)
        self.assertIn("ai_visual_impact.py --base", workflow)
        self.assertIn("inputs.ai_mode != 'off'", workflow)
        self.assertIn("needs.detect-impact.outputs.scope != 'none'", workflow)
        self.assertIn("inputs.ai_mode == 'full'", workflow)
        for mode in ("representative", "full", "off"):
            self.assertRegex(workflow, rf"(?m)^          - {mode}$")

    def test_qt_611_windows_repository_workaround_uses_arch_specific_path(self):
        script = PROJECT_ROOT / ".github" / "scripts" / "aqt_windows_qt611.py"
        namespace = runpy.run_path(str(script))
        repository_path = namespace["windows_qt_repository_path"](
            "6.11.2", "win64_msvc2022_64"
        )
        self.assertEqual(repository_path, "qt6_6112/qt6_6112_msvc2022_64")

    def test_high_confidence_blocking_failure_fails(self):
        result = self.model_result(failed_id="overall_layout")
        decision = evaluate_result(result, self.contract)
        self.assertEqual(decision["verdict"], "fail")
        self.assertEqual(decision["blocking_failures"], ["overall_layout"])

    def test_noticed_blurry_plot_text_fails(self):
        result = self.model_result(failed_id="text_raster_quality")
        decision = evaluate_result(result, self.contract)
        self.assertEqual(decision["verdict"], "fail")
        self.assertEqual(decision["blocking_failures"], ["text_raster_quality"])

    def test_low_confidence_failure_is_nonblocking(self):
        result = self.model_result(failed_id="overall_layout", confidence=0.5)
        decision = evaluate_result(result, self.contract)
        self.assertEqual(decision["verdict"], "pass")
        self.assertEqual(decision["blocking_failures"], [])

    def test_missing_check_is_rejected(self):
        result = self.model_result()
        result["checks"].pop()
        with self.assertRaises(VisualAcceptanceError):
            validate_model_response(result, self.contract)

    def test_pass_checks_can_omit_evidence_text(self):
        result = self.model_result()
        for check in result["checks"]:
            check["evidence"] = ""
        validate_model_response(result, self.contract)

    def test_nonpass_checks_still_require_evidence(self):
        result = self.model_result(failed_id="overall_layout")
        next(check for check in result["checks"] if check["status"] == "fail")["evidence"] = ""
        with self.assertRaisesRegex(VisualAcceptanceError, "no evidence"):
            validate_model_response(result, self.contract)

    def test_prompt_distinguishes_unreadable_details_from_visible_defects(self):
        prompt = build_prompt(self.contract)
        self.assertIn("Use `uncertain` when a\ndetail cannot be verified", prompt)
        self.assertIn("a `fail` requires a concrete visible contradiction", prompt)
        self.assertIn("square pixel blocks", prompt)
        self.assertIn("genuinely comparable size, weight, orientation, and contrast", prompt)
        self.assertIn("Set evidence to an empty string for `pass`", prompt)
        self.assertNotIn("blocking_severities", prompt)

    def test_custom_axis_contract_distinguishes_typography_from_rasterization(self):
        contract = load_contract(CONTRACTS_DIR / "custom_axis.json")
        raster_check = next(check for check in contract["checks"] if check["id"] == "text_raster_quality")
        expectation = raster_check["expectation"]
        self.assertIn("intentionally smaller, normal-weight, and light gray", expectation)
        self.assertIn("do not use the title or tabs as a direct sharpness comparison", expectation)
        self.assertIn("square pixel blocks", expectation)
        self.assertIn("no concrete scaling artifact is visible, pass", expectation)

    def test_interactive_tools_contract_describes_both_control_groups(self):
        contract = load_contract(CONTRACTS_DIR / "interactive_tools.json")
        controls_check = next(check for check in contract["checks"] if check["id"] == "tool_controls")
        expectation = controls_check["expectation"]
        self.assertIn("four adjacent controls grouped on the left", expectation)
        self.assertIn("exactly two controls grouped on the right", expectation)
        self.assertIn("Seeing only those two controls on the right is the expected layout", expectation)

    def test_console_output_is_reconfigured_for_unicode(self):
        stdout_bytes = io.BytesIO()
        stderr_bytes = io.BytesIO()
        stdout = io.TextIOWrapper(stdout_bytes, encoding="cp1252")
        stderr = io.TextIOWrapper(stderr_bytes, encoding="cp1252")
        with mock.patch.object(sys, "stdout", stdout), mock.patch.object(sys, "stderr", stderr):
            configure_console_encoding()
            print("distance ≈ 220", file=sys.stdout)
            print("pressure ≈ 6.4", file=sys.stderr)
            sys.stdout.flush()
            sys.stderr.flush()

        self.assertEqual(stdout.encoding, "utf-8")
        self.assertEqual(stderr.encoding, "utf-8")
        self.assertEqual(stdout_bytes.getvalue().decode("utf-8").splitlines(), ["distance ≈ 220"])
        self.assertEqual(stderr_bytes.getvalue().decode("utf-8").splitlines(), ["pressure ≈ 6.4"])

    def test_render_metadata_requires_the_requested_backend(self):
        with tempfile.TemporaryDirectory() as directory:
            metadata_path = Path(directory) / "screenshot.png.rhi.json"
            metadata_path.write_text(json.dumps(self.render_metadata()), encoding="utf-8")
            metadata = load_render_metadata(metadata_path)
            self.assertEqual(metadata["actual_graphics_api"], "opengl")

            with mock.patch.dict(os.environ, {"QACCELPLOT_EXPECTED_QT_VERSION": "6.8.0"}):
                with self.assertRaises(VisualAcceptanceError):
                    load_render_metadata(metadata_path)

            metadata_path.write_text(
                json.dumps(self.render_metadata(requested="vulkan", actual="opengl")),
                encoding="utf-8",
            )
            with self.assertRaises(VisualAcceptanceError):
                load_render_metadata(metadata_path)

    def test_offscreen_capture_can_outresolve_the_scaled_window(self):
        metadata = self.render_metadata()
        validate_capture_resolution(metadata, (880, 1120))

    def test_qt_62_uses_the_grab_target_as_physical_pixels(self):
        metadata = self.render_metadata()
        metadata.update(
            {
                "qt_version": "6.2.4",
                "grab_target_scaled_by_device_pixel_ratio": False,
                "offscreen_target_width": 880,
                "offscreen_target_height": 1120,
            }
        )
        validate_capture_resolution(metadata, (880, 1120))

    def test_resized_low_resolution_capture_is_rejected(self):
        metadata = self.render_metadata()
        metadata["captured_pixel_width"] = 440
        metadata["captured_pixel_height"] = 560
        with self.assertRaisesRegex(VisualAcceptanceError, "not rendered at its logical size"):
            validate_capture_resolution(metadata, (880, 1120))

    def test_png_dimensions_must_match_captured_pixels(self):
        with self.assertRaisesRegex(VisualAcceptanceError, "do not match captured pixels"):
            validate_capture_resolution(self.render_metadata(), (440, 560))

    def test_openai_request_uses_image_and_strict_contract_schema(self):
        request = {}
        expected_result = self.model_result()

        class FakeResponses:
            def create(self, **kwargs):
                request.update(kwargs)
                return types.SimpleNamespace(
                    output_text=json.dumps(expected_result),
                    usage=types.SimpleNamespace(
                        input_tokens=1200,
                        input_tokens_details=types.SimpleNamespace(cached_tokens=200),
                        output_tokens=300,
                        output_tokens_details=types.SimpleNamespace(reasoning_tokens=25),
                        total_tokens=1500,
                    ),
                )

        class FakeOpenAI:
            def __init__(self, *, api_key):
                request["api_key"] = api_key
                self.responses = FakeResponses()

        fake_openai = types.ModuleType("openai")
        fake_openai.OpenAI = FakeOpenAI

        with tempfile.TemporaryDirectory() as directory, mock.patch.dict(sys.modules, {"openai": fake_openai}):
            image_path = Path(directory) / "screenshot.png"
            image_path.write_bytes(b"test image data")
            result, usage = call_openai(
                image_path,
                self.contract,
                "gpt-5.4-nano",
                "test-key",
                self.render_metadata(),
            )

        self.assertEqual(result, expected_result)
        self.assertEqual(
            usage,
            {
                "input_tokens": 1200,
                "cached_input_tokens": 200,
                "output_tokens": 300,
                "reasoning_tokens": 25,
                "total_tokens": 1500,
            },
        )
        self.assertEqual(request["api_key"], "test-key")
        self.assertEqual(request["model"], "gpt-5.4-nano")
        self.assertEqual(request["reasoning"], {"effort": "none"})
        self.assertFalse(request["store"])

        content = request["input"][0]["content"]
        self.assertIn('"graphics_api":"opengl"', content[0]["text"])
        self.assertNotIn("blocking_severities", content[0]["text"])
        self.assertEqual(content[1]["type"], "input_image")
        self.assertTrue(content[1]["image_url"].startswith("data:image/png;base64,"))
        self.assertEqual(content[1]["detail"], "high")
        self.assertEqual(request["max_output_tokens"], 1024)

        response_format = request["text"]["format"]
        self.assertEqual(response_format["type"], "json_schema")
        self.assertTrue(response_format["strict"])
        checks_schema = response_format["schema"]["properties"]["checks"]
        self.assertEqual(checks_schema["minItems"], len(self.contract["checks"]))
        self.assertEqual(checks_schema["maxItems"], len(self.contract["checks"]))

    def test_cost_estimate_separates_cached_input(self):
        usage = {
            "input_tokens": 1000,
            "cached_input_tokens": 250,
            "output_tokens": 100,
        }
        pricing = {"input": 0.20, "cached_input": 0.02, "output": 1.25}
        self.assertEqual(estimate_cost_usd(usage, pricing), 0.00028)


if __name__ == "__main__":
    unittest.main()
