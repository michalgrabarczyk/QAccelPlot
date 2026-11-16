#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: MIT
#
add_library(QAccelPlotExampleCommon STATIC
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
