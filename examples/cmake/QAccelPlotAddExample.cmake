#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
include_guard(GLOBAL)

set(QACCELPLOT_EXAMPLE_TYPOGRAPHY_FILE
    "${CMAKE_CURRENT_LIST_DIR}/../common/qml/Typography.qml"
)
if(Qt6_VERSION VERSION_LESS 6.4)
    set(QACCELPLOT_EXAMPLE_FRAME_DRIVER_FILE
        "${CMAKE_CURRENT_LIST_DIR}/../common/qml/FrameDriverLegacy.qml"
    )
else()
    set(QACCELPLOT_EXAMPLE_FRAME_DRIVER_FILE
        "${CMAKE_CURRENT_LIST_DIR}/../common/qml/FrameDriver.qml"
    )
endif()

function(qaccelplot_add_example EXAMPLE_NAME)
    set(TARGET_NAME "QAccelPlotExample${EXAMPLE_NAME}")
    cmake_parse_arguments(ARG "" "" "SOURCES;QML_FILES;LIBRARIES" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "qaccelplot_add_example received unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "qaccelplot_add_example(${EXAMPLE_NAME}) requires SOURCES")
    endif()
    if(NOT ARG_QML_FILES)
        message(FATAL_ERROR "qaccelplot_add_example(${EXAMPLE_NAME}) requires QML_FILES")
    endif()

    qt_add_executable(${TARGET_NAME}
        MANUAL_FINALIZATION
        ${ARG_SOURCES}
    )

    foreach(QML_FILE IN LISTS ARG_QML_FILES)
        set_source_files_properties(${QML_FILE}
            PROPERTIES QT_RESOURCE_ALIAS ${QML_FILE}
        )
    endforeach()
    set_source_files_properties(${QACCELPLOT_EXAMPLE_TYPOGRAPHY_FILE}
        PROPERTIES QT_RESOURCE_ALIAS qml/Typography.qml
    )
    set_source_files_properties(${QACCELPLOT_EXAMPLE_FRAME_DRIVER_FILE}
        PROPERTIES QT_RESOURCE_ALIAS qml/FrameDriver.qml
    )

    qt_add_resources(${TARGET_NAME} "app_qml"
        PREFIX "/app"
        FILES
            ${ARG_QML_FILES}
            ${QACCELPLOT_EXAMPLE_TYPOGRAPHY_FILE}
            ${QACCELPLOT_EXAMPLE_FRAME_DRIVER_FILE}
    )

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Quick
            QAccelPlot::QAccelPlot
            QAccelPlotExampleCommon
            ${ARG_LIBRARIES}
    )

    qt_import_qml_plugins(${TARGET_NAME})
    qt_finalize_executable(${TARGET_NAME})

    if(WIN32 AND QACCELPLOT_DEPLOY_EXAMPLES)
        find_program(QACCELPLOT_WINDEPLOYQT_EXECUTABLE
            NAMES windeployqt
            HINTS "${Qt6_DIR}/../../../bin"
        )
        if(QACCELPLOT_WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                # Every example is emitted into the same configuration directory.
                # Serialize deployment so parallel builds do not race while copying
                # the same Qt runtime files into that directory.
                COMMAND "${CMAKE_COMMAND}"
                    "-DQACCELPLOT_WINDEPLOYQT_EXECUTABLE=${QACCELPLOT_WINDEPLOYQT_EXECUTABLE}"
                    "-DQACCELPLOT_WINDEPLOYQT_QML_DIR=${CMAKE_CURRENT_SOURCE_DIR}/qml"
                    "-DQACCELPLOT_WINDEPLOYQT_TARGET=$<TARGET_FILE:${TARGET_NAME}>"
                    "-DQACCELPLOT_WINDEPLOYQT_LOCK=$<TARGET_FILE_DIR:${TARGET_NAME}>/.qaccelplot-windeployqt.lock"
                    -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/QAccelPlotRunWinDeployQt.cmake"
                COMMENT "Running windeployqt for ${TARGET_NAME}..."
                VERBATIM
            )
        else()
            message(WARNING "windeployqt not found - Qt DLLs will not be deployed for ${TARGET_NAME}.")
        endif()
    endif()

    if(TARGET QAccelPlotExamples)
        add_dependencies(QAccelPlotExamples ${TARGET_NAME})
    endif()
endfunction()
