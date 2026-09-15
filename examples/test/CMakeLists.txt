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
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src/DataGenerationWorker.cpp"
)
target_include_directories(tst_data_generation_worker PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src"
)

add_qaccelplot_standalone_test(tst_data_delivery_metrics
    examples
    tst_data_delivery_metrics.cpp
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src/DataDeliveryMetrics.cpp"
)
target_include_directories(tst_data_delivery_metrics PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src"
)

add_qaccelplot_standalone_test(tst_sample_data_generators
    examples
    tst_sample_data_generators.cpp
    "${CMAKE_SOURCE_DIR}/examples/common/src/SampleDataGenerators.cpp"
)
target_include_directories(tst_sample_data_generators PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/common/include"
)

# ── Visual acceptance (smoke + optional AI inspection) ────────────────────────
if(QACCELPLOT_BUILD_VISUAL_TESTS)
    # Scenario names and contracts are described in cmake/QAccelPlotTestHelpers.cmake.
    add_qaccelplot_example_visual_tests(QAccelPlotExampleAnnotations            annotations)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleAxisFormats            axis_formats PAGES dateTime logarithmic categories)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleCustomAxis             custom_axis PAGES dualScale multiRate sharedTime)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleInteractiveTools       interactive_tools)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleQuickstart             quickstart)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleRealtime               realtime)
    add_qaccelplot_example_visual_tests(QAccelPlotExamplePerformanceShowcase    performance_showcase)
    add_qaccelplot_example_visual_tests(QAccelPlotExamplePulsarShowcase         pulsar_showcase NO_AI_INSPECTION)
    add_qaccelplot_example_visual_tests(QAccelPlotExampleStylingAndTransitions  styling_and_transitions PAGES styling fillsGradients transitions gaps)

    add_qaccelplot_example_visual_scenario(QAccelPlotExampleStylingAndTransitions styling_and_transitions styling_light
        PAGE styling
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
endif()
