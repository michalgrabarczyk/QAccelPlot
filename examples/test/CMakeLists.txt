#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# Example component unit tests and visual acceptance tests.
#
include("${CMAKE_SOURCE_DIR}/cmake/QAccelPlotTestHelpers.cmake")

# ── Component unit tests ──────────────────────────────────────────────────────
add_qaccelplot_test(tst_data_generation_worker
    examples
    tst_data_generation_worker.cpp
    "${CMAKE_SOURCE_DIR}/examples/showcases/performance/src/DataGenerationWorker.cpp"
)
target_include_directories(tst_data_generation_worker PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/showcases/performance/src"
)

add_qaccelplot_standalone_test(tst_data_delivery_metrics
    examples
    tst_data_delivery_metrics.cpp
    "${CMAKE_SOURCE_DIR}/examples/showcases/performance/src/DataDeliveryMetrics.cpp"
)
target_include_directories(tst_data_delivery_metrics PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/showcases/performance/src"
)

# ── Visual acceptance (smoke + optional AI inspection) ────────────────────────
if(QACCELPLOT_BUILD_VISUAL_TESTS)
    # Scenario names and contracts are described in cmake/QAccelPlotTestHelpers.cmake.
    add_qaccelplot_example_visual_tests(QAccelPlotExampleQuickstart             quickstart)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleMultipleAxes           multiple_axes PAGES dualScale multiRate sharedTime)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleTickFormats            tick_formats PAGES dateTime logarithmic categories)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleRealtime               realtime)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleMissingData            missing_data)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleAnnotations            annotations)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleMeasurementTools       measurement_tools)
    add_qaccelplot_example_visual_tests(QAccelPlotExamplePointCloud             point_cloud)
    add_qaccelplot_example_visual_tests(QAccelPlotExamplePerformance            performance)
    add_qaccelplot_example_visual_tests(QAccelPlotExamplePulsar                 pulsar NO_AI_INSPECTION)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleLineStyles             line_styles)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleMarkers                markers)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleGradients              gradients)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleTransitions            transitions)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleTheming                theming)

    add_qaccelplot_example_visual_scenario(QAccelPlotExampleTheming theming light
        ARGS --light-theme
        SMOKE_ONLY
    )
endif()

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME tst_ai_visual_acceptance
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/tst_ai_visual_acceptance.py"
    )
    add_test(
        NAME tst_ai_visual_cost_controls
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/tst_ai_visual_cost_controls.py"
    )
    # Offline checks of the visual tooling: no screenshots, no API calls.
    set_tests_properties(tst_ai_visual_acceptance tst_ai_visual_cost_controls PROPERTIES
        LABELS "unit-test;unit-test-examples"
    )
endif()
