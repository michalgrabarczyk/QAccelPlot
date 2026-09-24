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

add_subdirectory(quickstart)
add_subdirectory(axes)
add_subdirectory(data)
add_subdirectory(interaction)
add_subdirectory(plot_types)
add_subdirectory(showcases)
add_subdirectory(styling)
