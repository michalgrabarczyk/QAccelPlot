#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "*.cpp" "*.hpp")

add_library(QAccelPlotBenchmarkCommon STATIC
    ${SRC_FILES}
)

target_include_directories(QAccelPlotBenchmarkCommon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(QAccelPlotBenchmarkCommon
    PUBLIC Qt6::Core Qt6::Gui QAccelPlot::Core
)
