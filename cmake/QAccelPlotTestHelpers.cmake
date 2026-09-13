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

function(add_qaccelplot_example_visual_tests target contract_name)
    cmake_parse_arguments(VISUAL "NO_AI_INSPECTION" "" "" ${ARGN})

    if(NOT TARGET ${target})
        return()
    endif()

    set(_screenshot "${CMAKE_CURRENT_BINARY_DIR}/screenshots/${target}.png")
    set(_contract "${CMAKE_CURRENT_SOURCE_DIR}/visual/contracts/${contract_name}.json")
    set(_report "${CMAKE_CURRENT_BINARY_DIR}/visual-reports/${contract_name}.json")
    add_test(
        NAME smoke_${target}
        COMMAND $<TARGET_FILE:${target}> --screenshot "${_screenshot}"
    )
    set(_visual_test_environment
        "QT_FORCE_STDERR_LOGGING=1"
        "QACCELPLOT_HOVER_ENABLED=0"
    )
    if(WIN32)
        list(PREPEND _visual_test_environment "${_qaccelplot_windows_path_environment}")
    endif()
    set_tests_properties(smoke_${target} PROPERTIES
        ENVIRONMENT "${_visual_test_environment}"
        LABELS "visual-smoke"
        TIMEOUT 15
    )

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME validate_visual_${contract_name}
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/ai_visual_acceptance.py"
                --image "${_screenshot}"
                --contract "${_contract}"
                --validate-only
        )
        set_tests_properties(validate_visual_${contract_name} PROPERTIES
            DEPENDS smoke_${target}
            LABELS "visual-validation"
        )

        if(QACCELPLOT_BUILD_AI_VISUAL_TESTS AND NOT VISUAL_NO_AI_INSPECTION)
            add_test(
                NAME ai_inspection_${contract_name}
                COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/visual/ai_visual_acceptance.py"
                    --image "${_screenshot}"
                    --contract "${_contract}"
                    --report "${_report}"
            )
            set_tests_properties(ai_inspection_${contract_name} PROPERTIES
                DEPENDS validate_visual_${contract_name}
                LABELS "ai-inspection"
                TIMEOUT 120
            )
        endif()
    endif()
endfunction()
