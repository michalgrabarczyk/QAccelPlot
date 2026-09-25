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
add_qaccelplot_test(tst_spatial_grid_memory  core tst_spatial_grid_memory.cpp)
add_qaccelplot_test(tst_formatters           core tst_formatters.cpp)
add_qaccelplot_test(tst_transitions          core tst_transitions.cpp)
add_qaccelplot_test(tst_axis_mapping         core tst_axis_mapping.cpp)
add_qaccelplot_test(tst_axis_ticker          core tst_axis_ticker.cpp)
add_qaccelplot_test(tst_plot_appearance      core tst_plot_appearance.cpp)
add_qaccelplot_test(tst_plot_interaction     core tst_plot_interaction.cpp)
add_qaccelplot_standalone_test(tst_math_utils core tst_math_utils.cpp)
target_include_directories(tst_math_utils PRIVATE "${CMAKE_SOURCE_DIR}/QAccelPlot/src")
add_qaccelplot_test(tst_gradient_coordinates core tst_gradient_coordinates.cpp)
add_qaccelplot_test(tst_gradient_value_range core tst_gradient_value_range.cpp)
add_qaccelplot_test(tst_line_curve_data      core tst_line_curve_data.cpp)
add_qaccelplot_test(tst_line_curve_gaps      core tst_line_curve_gaps.cpp)
add_qaccelplot_test(tst_line_curve_rendering core tst_line_curve_rendering.cpp)
add_qaccelplot_test(tst_material_comparison  core tst_material_comparison.cpp)
add_qaccelplot_test(tst_line_curve_point_renderer core tst_line_curve_point_renderer.cpp)
add_qaccelplot_test(tst_line_curve_line_renderer core tst_line_curve_line_renderer.cpp)
add_qaccelplot_test(tst_marker_shapes        core tst_marker_shapes.cpp)
add_qaccelplot_test(tst_rectangle_list_data  core tst_rectangle_list_data.cpp)
add_qaccelplot_test(tst_rectangle_hover      core tst_rectangle_hover.cpp)
add_qaccelplot_test(tst_point_spatial_index  core tst_point_spatial_index.cpp)
add_qaccelplot_test(tst_point_cloud_data     core tst_point_cloud_data.cpp)
add_qaccelplot_test(tst_line_styles          core tst_line_styles.cpp)
add_qaccelplot_test(tst_data_anchor          core tst_data_anchor.cpp)
add_qaccelplot_test(tst_axis_rescale         core tst_axis_rescale.cpp)
add_qaccelplot_test(tst_plot_series_data_ranges core tst_plot_series_data_ranges.cpp)
add_qaccelplot_test(tst_color_palette        core tst_color_palette.cpp)
add_qaccelplot_test(tst_colormap             core tst_colormap.cpp)
add_qaccelplot_test(tst_color_bar            core tst_color_bar.cpp)
add_qaccelplot_test(tst_property_notifications core tst_property_notifications.cpp)
add_qaccelplot_test(tst_plot_rendering       core tst_plot_rendering.cpp)
# Instantiates Colormap from QML, so it needs the QML module plugin.
target_link_libraries(tst_colormap PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_colormap)
# Renders ColorBar and loads it inside Plot.qml, so it needs the QML module plugin.
target_link_libraries(tst_color_bar PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_color_bar)
# Sets marker groups from QML and loads Legend.qml, so it needs the QML module plugin.
target_link_libraries(tst_marker_shapes PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_marker_shapes)
# Loads Legend.qml to check QML-side palette defaults, so it needs the QML module plugin.
target_link_libraries(tst_color_palette PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_color_palette)
# Renders a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_plot_rendering PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_plot_rendering)
