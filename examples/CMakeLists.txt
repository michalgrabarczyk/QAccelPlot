#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
include(cmake/QAccelPlotAddExample.cmake)

add_subdirectory(common)

# Convenience aggregate target for building every example in one command:
#   cmake --build <build-dir> --target QAccelPlotExamples
add_custom_target(QAccelPlotExamples)

add_subdirectory(annotations)
add_subdirectory(axis_formats)
add_subdirectory(custom_axis)
add_subdirectory(interactive_tools)
add_subdirectory(quickstart)
add_subdirectory(performance_showcase)
add_subdirectory(pulsar_showcase)
add_subdirectory(realtime)
add_subdirectory(styling_and_transitions)
