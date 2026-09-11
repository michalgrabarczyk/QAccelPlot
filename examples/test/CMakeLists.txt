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
    tst_data_generation_worker.cpp
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src/DataGenerationWorker.cpp"
)
target_include_directories(tst_data_generation_worker PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src"
)

add_qaccelplot_test(tst_data_delivery_metrics
    tst_data_delivery_metrics.cpp
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src/DataDeliveryMetrics.cpp"
)
target_include_directories(tst_data_delivery_metrics PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/performance_showcase/src"
)

add_qaccelplot_test(tst_sample_data_generators
    tst_sample_data_generators.cpp
    "${CMAKE_SOURCE_DIR}/examples/common/src/SampleDataGenerators.cpp"
)
target_include_directories(tst_sample_data_generators PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/common/include"
)

# ── Visual acceptance (smoke + AI inspection) ─────────────────────────────────
add_qaccelplot_example_visual_tests(QAccelPlotExampleAnnotations            annotations)
add_qaccelplot_example_visual_tests(QAccelPlotExampleAxisFormats            axis_formats)
add_qaccelplot_example_visual_tests(QAccelPlotExampleCustomAxis             custom_axis)
add_qaccelplot_example_visual_tests(QAccelPlotExampleInteractiveTools       interactive_tools)
add_qaccelplot_example_visual_tests(QAccelPlotExampleQuickstart             quickstart)
add_qaccelplot_example_visual_tests(QAccelPlotExampleRealtime               realtime)
add_qaccelplot_example_visual_tests(QAccelPlotExamplePerformanceShowcase    performance_showcase)
add_qaccelplot_example_visual_tests(QAccelPlotExamplePulsarShowcase         pulsar_showcase NO_AI_INSPECTION)
add_qaccelplot_example_visual_tests(QAccelPlotExampleStylingAndTransitions  styling_and_transitions)

if(TARGET QAccelPlotExampleStylingAndTransitions)
    set(_light_theme_screenshot "${CMAKE_CURRENT_BINARY_DIR}/screenshots/QAccelPlotExampleStylingAndTransitionsLight.png")
    add_test(
        NAME smoke_QAccelPlotExampleStylingAndTransitionsLight
        COMMAND $<TARGET_FILE:QAccelPlotExampleStylingAndTransitions>
            --light-theme
            --screenshot "${_light_theme_screenshot}"
    )
    set_tests_properties(smoke_QAccelPlotExampleStylingAndTransitionsLight PROPERTIES
        ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH};QT_FORCE_STDERR_LOGGING=1;QACCELPLOT_HOVER_ENABLED=0"
        TIMEOUT 15
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
