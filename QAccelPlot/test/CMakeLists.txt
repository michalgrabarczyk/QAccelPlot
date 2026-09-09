#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# QAccelPlot library unit tests.
#
# Visual acceptance tests for example applications live in examples/test/.
#
include("${CMAKE_SOURCE_DIR}/cmake/QAccelPlotTestHelpers.cmake")

add_qaccelplot_test(tst_axis_tick_painter    tst_axis_tick_painter.cpp)
add_qaccelplot_test(tst_spatial_grid         tst_spatial_grid.cpp)
add_qaccelplot_test(tst_formatters           tst_formatters.cpp)
add_qaccelplot_test(tst_transitions          tst_transitions.cpp)
add_qaccelplot_test(tst_axis_mapping         tst_axis_mapping.cpp)
add_qaccelplot_test(tst_plot_appearance      tst_plot_appearance.cpp)
add_qaccelplot_test(tst_math_utils           tst_math_utils.cpp)
add_qaccelplot_test(tst_gradient_coordinates tst_gradient_coordinates.cpp)
add_qaccelplot_test(tst_line_curve_data      tst_line_curve_data.cpp)
add_qaccelplot_test(tst_material_comparison  tst_material_comparison.cpp)
add_qaccelplot_test(tst_rectangle_list_data  tst_rectangle_list_data.cpp)
