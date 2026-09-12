#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
cmake_minimum_required(VERSION 3.16)

project(QAccelPlotPackageConsumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# This project must remain independent of the QAccelPlot source tree. Its
# purpose is to verify the public package exactly as a downstream application
# consumes it.
find_package(QAccelPlot CONFIG REQUIRED)

if(Qt6_VERSION VERSION_LESS 6.3)
    set(CMAKE_AUTOMOC ON)
else()
    qt_standard_project_setup(REQUIRES 6.2)
endif()

qt_add_executable(QAccelPlotPackageConsumer MANUAL_FINALIZATION
    main.cpp
)
set_target_properties(QAccelPlotPackageConsumer PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

set_source_files_properties(Main.qml
    PROPERTIES QT_RESOURCE_ALIAS Main.qml
)

qt_add_resources(QAccelPlotPackageConsumer "package_consumer_qml"
    PREFIX "/package-consumer"
    FILES Main.qml
)

target_link_libraries(QAccelPlotPackageConsumer PRIVATE
    QAccelPlot::QAccelPlot
)

qt_import_qml_plugins(QAccelPlotPackageConsumer)
qt_finalize_executable(QAccelPlotPackageConsumer)
