#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# Shared test-creation helpers used by QAccelPlot/, examples/, and benchmarks/.
#
find_package(Qt6 REQUIRED COMPONENTS Test Quick)

# Qt path for setting the test environment on Windows so executables find Qt DLLs.
get_target_property(_qt_core_location Qt6::Core LOCATION)
get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)

function(add_qaccelplot_test name)
    qt_add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE QAccelPlot Qt6::Test Qt6::Quick)
    # Expose QAccelPlot's private subdirectory headers to in-tree tests.
    set(_qaccelplot_src "${CMAKE_SOURCE_DIR}/QAccelPlot/src")
    target_include_directories(${name} PRIVATE
        "${_qaccelplot_src}/annotations"
        "${_qaccelplot_src}/axis"
        "${_qaccelplot_src}/curves"
        "${_qaccelplot_src}/effects"
        "${_qaccelplot_src}/formatters"
        "${_qaccelplot_src}/grid"
        "${_qaccelplot_src}/linestyles"
        "${_qaccelplot_src}/materials"
        "${_qaccelplot_src}/renderers"
        "${_qaccelplot_src}/series"
        "${_qaccelplot_src}/shapes"
        "${_qaccelplot_src}/transitions"
    )
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
    )
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
    set_tests_properties(smoke_${target} PROPERTIES
        ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH};QT_FORCE_STDERR_LOGGING=1"
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

        if(NOT VISUAL_NO_AI_INSPECTION)
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
