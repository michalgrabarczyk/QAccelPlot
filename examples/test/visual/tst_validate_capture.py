#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
import json
import os
import re
import runpy
import struct
import sys
import tempfile
from pathlib import Path
import unittest
from unittest import mock


VISUAL_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = VISUAL_DIR.parent.parent.parent
sys.path.insert(0, str(VISUAL_DIR))

from validate_capture import (  # noqa: E402
    PNG_SIGNATURE,
    CaptureValidationError,
    load_render_metadata,
    validate_capture_page,
    validate_capture_resolution,
    validate_image,
)


EXAMPLES_DIR = PROJECT_ROOT / "examples"
EXAMPLE_TESTS_DIR = VISUAL_DIR.parent
WORKFLOW_PATH = PROJECT_ROOT / ".github" / "workflows" / "platform-matrix.yml"
TEST_HELPERS_PATH = PROJECT_ROOT / "cmake" / "QAccelPlotTestHelpers.cmake"


def camel_to_snake(value):
    return re.sub(r"(?<!^)(?=[A-Z])", "_", value).lower()


def example_dirs():
    """Maps each example name, the snake_case form of its target name, to its directory."""
    dirs = {}
    for cmake_path in EXAMPLES_DIR.rglob("CMakeLists.txt"):
        match = re.search(r"qaccelplot_add_example\(\s*([A-Za-z0-9_]+)", cmake_path.read_text(encoding="utf-8"))
        if match:
            dirs[camel_to_snake(match.group(1))] = cmake_path.parent
    return dirs


def example_names():
    return set(example_dirs())


def registered_visual_scenarios():
    """Parses the ``<example>/<scenario>`` registrations in examples/test/CMakeLists.txt."""
    cmake_source = (EXAMPLE_TESTS_DIR / "CMakeLists.txt").read_text(encoding="utf-8")
    scenarios = set()
    for command, arguments in re.findall(
        r"^\s*(add_qaccelplot_example_visual_tests|add_qaccelplot_example_visual_scenario)\(([^)]*)\)",
        cmake_source,
        re.MULTILINE,
    ):
        tokens = arguments.split()
        if command == "add_qaccelplot_example_visual_scenario":
            scenarios.add(f"{tokens[1]}/{tokens[2]}")
            continue
        pages = []
        if "PAGES" in tokens:
            pages = [token for token in tokens[tokens.index("PAGES") + 1 :] if not token.isupper()]
        for page in pages or ["default"]:
            scenarios.add(f"{tokens[1]}/{camel_to_snake(page)}")
    return scenarios


class CaptureValidationTests(unittest.TestCase):
    def setUp(self):
        # The CI matrix sets these for its real captures; the tests supply their own expectations.
        environment = mock.patch.dict(os.environ)
        environment.start()
        self.addCleanup(environment.stop)
        for name in ("QACCELPLOT_EXPECTED_GRAPHICS_API", "QACCELPLOT_EXPECTED_QT_VERSION", "QACCELPLOT_OPENGL_ES_VERSION"):
            os.environ.pop(name, None)

    def render_metadata(self, *, requested="opengl", actual="opengl", opengl_es=False, opengl_version=(4, 5)):
        return {
            "version": 5,
            "capture_method": "item_grab_to_image",
            "grab_target_scaled_by_device_pixel_ratio": True,
            "requested_graphics_api": requested,
            "actual_graphics_api": actual,
            "opengl_context_observed": actual == "opengl",
            "opengl_es": opengl_es,
            "opengl_major_version": opengl_version[0] if actual == "opengl" else 0,
            "opengl_minor_version": opengl_version[1] if actual == "opengl" else 0,
            "qt_version": "6.7.3",
            "logical_window_width": 880,
            "logical_window_height": 1120,
            "offscreen_target_width": 1760,
            "offscreen_target_height": 2240,
            "effective_device_pixel_ratio": 0.5,
            "captured_pixel_width": 880,
            "captured_pixel_height": 1120,
            "page": "styling",
        }

    def test_every_example_registers_a_visual_scenario(self):
        for name, directory in example_dirs().items():
            self.assertEqual(directory.name, name, "An example directory is named after its target")
        self.assertEqual({identity.split("/")[0] for identity in registered_visual_scenarios()}, example_names())

    def test_workflow_covers_every_hardware_rhi_backend(self):
        workflow = WORKFLOW_PATH.read_text(encoding="utf-8")
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
        self.assertEqual(workflow.count("-DQACCELPLOT_BUILD_VISUAL_TESTS=ON"), 3)
        self.assertIn("libvulkan1 libvulkan-dev mesa-vulkan-drivers vulkan-tools", workflow)
        self.assertIn("QT_SCALE_FACTOR: ${{ startsWith(matrix.platform.os, 'ubuntu-') && '1' || '0.5' }}", workflow)
        self.assertIn("'.github/scripts/aqt_windows_qt611.py',", workflow)
        self.assertIn("'install-qt', 'windows', 'desktop', '6.11.2'", workflow)
        self.assertIn("uses: actions/cache@v5", workflow)
        self.assertIn("if-no-files-found: error", workflow)
        self.assertIn("overwrite: true", workflow)
        self.assertIn("steps.upload-visual-artifacts.outputs.artifact-url", workflow)
        self.assertNotIn("qt_arch:", workflow)

    def test_workflow_captures_opengl_es_on_x86_64_and_arm64(self):
        workflow = WORKFLOW_PATH.read_text(encoding="utf-8")
        include = re.search(r"(?ms)^        include:\n(?P<entries>.*?)^    env:", workflow)
        self.assertIsNotNone(include)
        entries = include.group("entries")
        for os_name in ("ubuntu-24.04", "ubuntu-24.04-arm"):
            with self.subTest(os=os_name):
                self.assertRegex(
                    entries,
                    rf"qt_version: '6\.8\.3'\n\s+platform:\n\s+os: {re.escape(os_name)}\n"
                    r"\s+backend: opengl\n\s+label: opengles3\n\s+opengl_es: '3\.0'\n",
                )
        self.assertIn("if: matrix.platform.opengl_es", workflow)
        self.assertIn('echo "QACCELPLOT_OPENGL_ES_VERSION=${{ matrix.platform.opengl_es }}"', workflow)
        self.assertIn('echo "QT_XCB_GL_INTEGRATION=xcb_egl"', workflow)
        self.assertIn('echo "MESA_GLES_VERSION_OVERRIDE=${{ matrix.platform.opengl_es }}"', workflow)
        # Artifact and job names must keep ES captures apart from desktop OpenGL on the same runner.
        self.assertIn(
            "name: visual-${{ matrix.qt_version }}-${{ matrix.platform.label || matrix.platform.backend }}-${{ matrix.platform.os }}",
            workflow,
        )

    def test_screenshot_tests_disable_hover(self):
        test_helpers = TEST_HELPERS_PATH.read_text(encoding="utf-8")
        example_tests = (EXAMPLE_TESTS_DIR / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("QACCELPLOT_HOVER_ENABLED=0", test_helpers)
        # Screenshot tests are registered through the scenario helpers, which set the environment.
        self.assertNotRegex(example_tests, r"NAME smoke_")

    def test_workflow_runs_unit_tests_in_the_matrix(self):
        workflow = WORKFLOW_PATH.read_text(encoding="utf-8")
        self.assertRegex(workflow, r"(?m)^  pull_request:\s*$")
        self.assertIn("--target QAccelPlotExamples ${{ !matrix.platform.opengl_es && 'QAccelPlotTests' || '' }}", workflow)
        self.assertEqual(workflow.count("-DQACCELPLOT_BUILD_TESTS=ON"), 3)
        self.assertEqual(workflow.count("--output-on-failure -L '^unit-test$'"), 2)
        self.assertIn("xvfb-run -a ctest --test-dir build -C Release --output-on-failure -L '^unit-test$'", workflow)
        windows_and_macos = workflow.split("      - name: Run unit tests (Windows and macOS)\n", maxsplit=1)[1]
        unit_test_step = windows_and_macos.split("\n\n", maxsplit=1)[0]
        # The job-wide 0.5 scale factor would halve the pixels the rendering tests inspect.
        self.assertIn("QT_SCALE_FACTOR: '1'", unit_test_step)

    def test_workflow_provides_one_combined_screenshot_download(self):
        workflow = WORKFLOW_PATH.read_text(encoding="utf-8")
        bundle_job = workflow.split("\n  bundle-screenshots:\n", maxsplit=1)[1]
        self.assertIn("name: Bundle all visual screenshots", bundle_job)
        self.assertIn("pattern: visual-*", bundle_job)
        self.assertIn("name: all-visual-screenshots", bundle_job)
        self.assertIn('destination="all-visual-screenshots/$artifact_name/screenshots"', bundle_job)
        self.assertIn("find \"$artifact_dir\" -type f -path '*/screenshots/*'", bundle_job)
        self.assertIn("steps.upload-combined-screenshots.outputs.artifact-url", bundle_job)

    def test_qt_611_windows_repository_workaround_uses_arch_specific_path(self):
        script = PROJECT_ROOT / ".github" / "scripts" / "aqt_windows_qt611.py"
        namespace = runpy.run_path(str(script))
        repository_path = namespace["windows_qt_repository_path"](
            "6.11.2", "win64_msvc2022_64"
        )
        self.assertEqual(repository_path, "qt6_6112/qt6_6112_msvc2022_64")

    def test_render_metadata_requires_the_requested_backend(self):
        with tempfile.TemporaryDirectory() as directory:
            metadata_path = Path(directory) / "screenshot.png.rhi.json"
            metadata_path.write_text(json.dumps(self.render_metadata()), encoding="utf-8")
            metadata = load_render_metadata(metadata_path)
            self.assertEqual(metadata["actual_graphics_api"], "opengl")

            with mock.patch.dict(os.environ, {"QACCELPLOT_EXPECTED_QT_VERSION": "6.8.0"}):
                with self.assertRaises(CaptureValidationError):
                    load_render_metadata(metadata_path)

            metadata_path.write_text(
                json.dumps(self.render_metadata(requested="vulkan", actual="opengl")),
                encoding="utf-8",
            )
            with self.assertRaises(CaptureValidationError):
                load_render_metadata(metadata_path)

    def test_render_metadata_requires_the_requested_opengl_es_context(self):
        with tempfile.TemporaryDirectory() as directory:
            metadata_path = Path(directory) / "screenshot.png.rhi.json"

            def write(**kwargs):
                metadata_path.write_text(json.dumps(self.render_metadata(**kwargs)), encoding="utf-8")

            with mock.patch.dict(os.environ, {"QACCELPLOT_OPENGL_ES_VERSION": "3.0"}):
                write(opengl_es=True, opengl_version=(3, 2))
                self.assertTrue(load_render_metadata(metadata_path)["opengl_es"])

                write(opengl_es=False, opengl_version=(4, 5))
                with self.assertRaisesRegex(CaptureValidationError, "expected OpenGL ES 3.0 or newer"):
                    load_render_metadata(metadata_path)

                write(opengl_es=True, opengl_version=(2, 0))
                with self.assertRaisesRegex(CaptureValidationError, "expected OpenGL ES 3.0 or newer"):
                    load_render_metadata(metadata_path)

                write(requested="vulkan", actual="vulkan")
                with self.assertRaisesRegex(CaptureValidationError, "expected OpenGL ES 3.0 or newer"):
                    load_render_metadata(metadata_path)

            with mock.patch.dict(os.environ, {"QACCELPLOT_OPENGL_ES_VERSION": "three"}):
                write(opengl_es=True, opengl_version=(3, 0))
                with self.assertRaisesRegex(CaptureValidationError, "must look like"):
                    load_render_metadata(metadata_path)

            metadata = self.render_metadata()
            metadata["opengl_context_observed"] = False
            metadata_path.write_text(json.dumps(metadata), encoding="utf-8")
            with self.assertRaisesRegex(CaptureValidationError, "did not record a valid current context"):
                load_render_metadata(metadata_path)

    def test_capture_page_must_match_the_scenario(self):
        metadata = self.render_metadata()
        validate_capture_page(metadata, "styling")
        with self.assertRaisesRegex(CaptureValidationError, "shows page 'styling'; expected page 'transitions'"):
            validate_capture_page(metadata, "transitions")
        with self.assertRaisesRegex(CaptureValidationError, "expected an example without pages"):
            validate_capture_page(metadata, "")

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
        with self.assertRaisesRegex(CaptureValidationError, "not rendered at its logical size"):
            validate_capture_resolution(metadata, (880, 1120))

    def test_png_dimensions_must_match_captured_pixels(self):
        with self.assertRaisesRegex(CaptureValidationError, "do not match captured pixels"):
            validate_capture_resolution(self.render_metadata(), (440, 560))


    def test_image_must_be_a_png_of_plausible_size(self):
        with tempfile.TemporaryDirectory() as directory:
            image_path = Path(directory) / "screenshot.png"
            with self.assertRaisesRegex(CaptureValidationError, "does not exist"):
                validate_image(image_path)

            header = PNG_SIGNATURE + struct.pack(">I", 13) + b"IHDR" + struct.pack(">II", 880, 1120)
            image_path.write_bytes(header)
            with self.assertRaisesRegex(CaptureValidationError, "suspiciously small"):
                validate_image(image_path)
            self.assertEqual(validate_image(image_path, minimum_file_size_bytes=len(header)), (880, 1120))

            image_path.write_bytes(b"not a png".ljust(64))
            with self.assertRaisesRegex(CaptureValidationError, "not a valid PNG"):
                validate_image(image_path, minimum_file_size_bytes=1)


if __name__ == "__main__":
    unittest.main()
