#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# A standalone application that consumes QAccelPlot the way a user project does.
# It is not part of the QAccelPlot build: configure it on its own, either against
# an installed package (-DCMAKE_PREFIX_PATH=<install prefix>) or against this
# checkout (-DQACCELPLOT_CONSUMER_ADD_SUBDIRECTORY=ON).
#
cmake_minimum_required(VERSION 3.16)

project(QAccelPlotConsumer LANGUAGES CXX)

option(QACCELPLOT_CONSUMER_ADD_SUBDIRECTORY
    "Consume this checkout with add_subdirectory instead of an installed package"
    OFF
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Quick)

if(QACCELPLOT_CONSUMER_ADD_SUBDIRECTORY)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../.." QAccelPlot)
else()
    find_package(QAccelPlot REQUIRED)
endif()

qt_add_executable(QAccelPlotConsumer MANUAL_FINALIZATION
    main.cpp
)

qt_add_resources(QAccelPlotConsumer "app_qml"
    PREFIX "/app"
    FILES Main.qml
)

target_link_libraries(QAccelPlotConsumer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    QAccelPlot::QAccelPlot
)

qt_import_qml_plugins(QAccelPlotConsumer)
qt_finalize_executable(QAccelPlotConsumer)

enable_testing()
add_test(NAME consumer_renders_plot COMMAND QAccelPlotConsumer)
set_tests_properties(consumer_renders_plot PROPERTIES
    ENVIRONMENT "QT_FORCE_STDERR_LOGGING=1"
)
