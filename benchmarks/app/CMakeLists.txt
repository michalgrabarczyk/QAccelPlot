#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.hpp")

qt_add_executable(QAccelPlotBenchmarkApp
    MANUAL_FINALIZATION
    ${APP_SOURCES}
)

set_source_files_properties(qml/main.qml
    PROPERTIES QT_RESOURCE_ALIAS qml/main.qml
)

qt_add_resources(QAccelPlotBenchmarkApp "app_qml"
    PREFIX "/app"
    FILES
        qml/main.qml
)

target_include_directories(QAccelPlotBenchmarkApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/benchmarks/common
)

target_link_libraries(QAccelPlotBenchmarkApp
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        QAccelPlot::QAccelPlot
        QAccelPlotBenchmarkCommon
)

qt_import_qml_plugins(QAccelPlotBenchmarkApp)
qt_finalize_executable(QAccelPlotBenchmarkApp)

if(WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET QAccelPlotBenchmarkApp POST_BUILD
            COMMAND "${WINDEPLOYQT}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                $<TARGET_FILE:QAccelPlotBenchmarkApp>
            COMMENT "Running windeployqt for QAccelPlotBenchmarkApp..."
        )
    else()
        message(WARNING "windeployqt not found – Qt DLLs will not be deployed automatically.")
    endif()
endif()
