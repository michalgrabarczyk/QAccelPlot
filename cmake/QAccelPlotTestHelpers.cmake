#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# Shared test-creation helpers used by QAccelPlot/, examples/, and benchmarks/.
#
find_package(Qt6 REQUIRED COMPONENTS Test)

# Qt path for setting the test environment on Windows so executables find Qt DLLs.
get_target_property(_qt_core_location Qt6::Core LOCATION)
get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
if(WIN32)
    set(_qaccelplot_windows_path_environment "PATH=${_qt_bin_dir};$ENV{PATH}")
    string(REPLACE ";" "\\;" _qaccelplot_windows_path_environment "${_qaccelplot_windows_path_environment}")
endif()

function(_qaccelplot_register_test name scope)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        LABELS "unit-test;unit-test-${scope}"
    )
    if(WIN32)
        set_tests_properties(${name} PROPERTIES
            ENVIRONMENT "${_qaccelplot_windows_path_environment}"
        )
    endif()

    if(TARGET QAccelPlotTests)
        add_dependencies(QAccelPlotTests ${name})
    endif()
endfunction()

function(add_qaccelplot_test name scope)
    qt_add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE QAccelPlot Qt6::Test)
    _qaccelplot_register_test(${name} ${scope})
endfunction()

# Use for tests whose sources do not depend directly on the plotting library.
function(add_qaccelplot_standalone_test name scope)
    qt_add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE Qt6::Test)
    _qaccelplot_register_test(${name} ${scope})
endfunction()

# Visual acceptance is organized in scenarios: one example captured in one state, such as a
# tab. A scenario named <scenario> of <example> uses these names, which the Python scripts in
# examples/test/visual/ derive from the same convention (see visual_scenarios.py):
#
#   contract:    visual/contracts/<example>/<scenario>.json
#   screenshot:  screenshots/<example>__<scenario>.png
#   tests:       smoke_<example>__<scenario>, validate_visual_<example>__<scenario>,
#                ai_inspection_<example>__<scenario>
#
# add_qaccelplot_example_visual_scenario(<target> <example> <scenario>
#     [PAGE <page>] [ARGS <argument>...] [SMOKE_ONLY] [NO_AI_INSPECTION])
#
#   PAGE              Select the page with this objectName before capturing (--page).
#   ARGS              Extra example arguments, e.g. --light-theme.
#   SMOKE_ONLY        Capture only; the scenario has no contract.
#   NO_AI_INSPECTION  Validate against the contract, but never register paid AI inspection.
function(add_qaccelplot_example_visual_scenario target example scenario)
    cmake_parse_arguments(VISUAL "SMOKE_ONLY;NO_AI_INSPECTION" "PAGE" "ARGS" ${ARGN})

    if(NOT TARGET ${target})
        return()
    endif()

    set(_name "${example}__${scenario}")
    set(_screenshot "${CMAKE_CURRENT_BINARY_DIR}/screenshots/${_name}.png")
    set(_contract "${CMAKE_CURRENT_SOURCE_DIR}/visual/contracts/${example}/${scenario}.json")
    set(_report "${CMAKE_CURRENT_BINARY_DIR}/visual-reports/${_name}.json")
    set(_page_arguments)
    if(VISUAL_PAGE)
        set(_page_arguments "--page=${VISUAL_PAGE}")
    endif()

    add_test(
        NAME smoke_${_name}
        COMMAND $<TARGET_FILE:${target}> ${_page_arguments} ${VISUAL_ARGS} --screenshot "${_screenshot}"
    )
    set(_visual_test_environment
        "QT_FORCE_STDERR_LOGGING=1"
        "QACCELPLOT_HOVER_ENABLED=0"
    )
    if(WIN32)
        list(PREPEND _visual_test_environment "${_qaccelplot_windows_path_environment}")
    endif()
    set_tests_properties(smoke_${_name} PROPERTIES
        ENVIRONMENT "${_visual_test_environment}"
        LABELS "visual-smoke"
        TIMEOUT 15
    )

    if(VISUAL_SMOKE_ONLY)
        return()
    endif()

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME validate_visual_${_name}
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/ai_visual_acceptance.py"
                --image "${_screenshot}"
                --contract "${_contract}"
                "--page=${VISUAL_PAGE}"
                --validate-only
        )
        set_tests_properties(validate_visual_${_name} PROPERTIES
            DEPENDS smoke_${_name}
            LABELS "visual-validation"
        )

        if(QACCELPLOT_BUILD_AI_VISUAL_TESTS AND NOT VISUAL_NO_AI_INSPECTION)
            add_test(
                NAME ai_inspection_${_name}
                COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/ai_visual_acceptance.py"
                    --image "${_screenshot}"
                    --contract "${_contract}"
                    "--page=${VISUAL_PAGE}"
                    --report "${_report}"
            )
            set_tests_properties(ai_inspection_${_name} PROPERTIES
                DEPENDS validate_visual_${_name}
                LABELS "ai-inspection"
                TIMEOUT 120
            )
        endif()
    endif()
endfunction()

# add_qaccelplot_example_visual_tests(<target> <example> [PAGES <page>...] [NO_AI_INSPECTION])
#
# Registers one scenario per page. Pages are the camelCase objectNames of the example's page
# buttons; each scenario is the page name in snake_case (PAGES dateTime registers date_time).
# Without PAGES, registers the single "default" scenario.
function(add_qaccelplot_example_visual_tests target example)
    cmake_parse_arguments(VISUAL "NO_AI_INSPECTION" "" "PAGES" ${ARGN})

    set(_options)
    if(VISUAL_NO_AI_INSPECTION)
        list(APPEND _options NO_AI_INSPECTION)
    endif()

    if(NOT VISUAL_PAGES)
        add_qaccelplot_example_visual_scenario(${target} ${example} default ${_options})
        return()
    endif()
    foreach(_page IN LISTS VISUAL_PAGES)
        string(REGEX REPLACE "([A-Z])" "_\\1" _scenario "${_page}")
        string(TOLOWER "${_scenario}" _scenario)
        add_qaccelplot_example_visual_scenario(${target} ${example} ${_scenario} PAGE ${_page} ${_options})
    endforeach()
endfunction()
