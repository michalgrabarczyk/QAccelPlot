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

add_qaccelplot_test(tst_axis_tick_painter    core tst_axis_tick_painter.cpp)
add_qaccelplot_test(tst_grid_node            core tst_grid_node.cpp)
add_qaccelplot_test(tst_spatial_grid         core tst_spatial_grid.cpp)
add_qaccelplot_test(tst_formatters           core tst_formatters.cpp)
add_qaccelplot_test(tst_transitions          core tst_transitions.cpp)
add_qaccelplot_test(tst_axis_mapping         core tst_axis_mapping.cpp)
add_qaccelplot_test(tst_plot_appearance      core tst_plot_appearance.cpp)
add_qaccelplot_standalone_test(tst_math_utils core tst_math_utils.cpp)
target_include_directories(tst_math_utils PRIVATE "${CMAKE_SOURCE_DIR}/QAccelPlot/src")
add_qaccelplot_test(tst_gradient_coordinates core tst_gradient_coordinates.cpp)
add_qaccelplot_test(tst_line_curve_data      core tst_line_curve_data.cpp)
add_qaccelplot_test(tst_line_curve_gaps      core tst_line_curve_gaps.cpp)
add_qaccelplot_test(tst_material_comparison  core tst_material_comparison.cpp)
add_qaccelplot_test(tst_line_curve_point_renderer core tst_line_curve_point_renderer.cpp)
add_qaccelplot_test(tst_line_curve_line_renderer core tst_line_curve_line_renderer.cpp)
add_qaccelplot_test(tst_marker_shapes        core tst_marker_shapes.cpp)
add_qaccelplot_test(tst_rectangle_list_data  core tst_rectangle_list_data.cpp)
add_qaccelplot_test(tst_line_styles          core tst_line_styles.cpp)
add_qaccelplot_test(tst_data_anchor          core tst_data_anchor.cpp)
add_qaccelplot_test(tst_axis_rescale         core tst_axis_rescale.cpp)
add_qaccelplot_test(tst_plot_series_data_ranges core tst_plot_series_data_ranges.cpp)
add_qaccelplot_test(tst_color_palette        core tst_color_palette.cpp)
# Loads Legend.qml to check QML-side palette defaults, so it needs the QML module plugin.
target_link_libraries(tst_color_palette PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_color_palette)
