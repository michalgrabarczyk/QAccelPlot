#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
add_library(QAccelPlotExampleCommon STATIC
    src/SampleDataGenerators.cpp
    src/ExampleUtils.cpp
)

target_include_directories(QAccelPlotExampleCommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(QAccelPlotExampleCommon
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        QAccelPlot::Core
)
