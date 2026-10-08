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

add_qaccelplot_test(tst_graphics_context     core tst_graphics_context.cpp)
add_qaccelplot_test(tst_axis_tick_painter    core tst_axis_tick_painter.cpp)
add_qaccelplot_test(tst_grid_node            core tst_grid_node.cpp)
add_qaccelplot_test(tst_spatial_grid         core tst_spatial_grid.cpp)
add_qaccelplot_test(tst_spatial_grid_memory  core tst_spatial_grid_memory.cpp)
add_qaccelplot_test(tst_hover_index_budget   core tst_hover_index_budget.cpp)
add_qaccelplot_test(tst_formatters           core tst_formatters.cpp)
add_qaccelplot_test(tst_transitions          core tst_transitions.cpp)
add_qaccelplot_test(tst_axis_mapping         core tst_axis_mapping.cpp)
add_qaccelplot_test(tst_axis_ticker          core tst_axis_ticker.cpp)
add_qaccelplot_test(tst_plot_appearance      core tst_plot_appearance.cpp)
add_qaccelplot_test(tst_plot_interaction     core tst_plot_interaction.cpp)
add_qaccelplot_test(tst_rectangle_zoom       core tst_rectangle_zoom.cpp)
add_qaccelplot_test(tst_rectangle_zoom_rendering core tst_rectangle_zoom_rendering.cpp)
target_link_libraries(tst_rectangle_zoom_rendering PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_rectangle_zoom_rendering)
add_qaccelplot_standalone_test(tst_math_utils core tst_math_utils.cpp)
target_include_directories(tst_math_utils PRIVATE "${CMAKE_SOURCE_DIR}/QAccelPlot/src")
add_qaccelplot_test(tst_gradient_coordinates core tst_gradient_coordinates.cpp)
add_qaccelplot_test(tst_gradient_value_range core tst_gradient_value_range.cpp)
add_qaccelplot_test(tst_gradient_effect_threads core tst_gradient_effect_threads.cpp)
add_qaccelplot_test(tst_line_curve_data      core tst_line_curve_data.cpp)
add_qaccelplot_test(tst_line_curve_gaps      core tst_line_curve_gaps.cpp)
add_qaccelplot_test(tst_line_curve_rendering core tst_line_curve_rendering.cpp)
add_qaccelplot_test(tst_line_curve_hover     core tst_line_curve_hover.cpp)
add_qaccelplot_test(tst_render_origin_precision core tst_render_origin_precision.cpp)
add_qaccelplot_test(tst_line_curve_transitions core tst_line_curve_transitions.cpp)
add_qaccelplot_test(tst_material_comparison  core tst_material_comparison.cpp)
add_qaccelplot_test(tst_line_curve_point_renderer core tst_line_curve_point_renderer.cpp)
add_qaccelplot_test(tst_line_curve_line_renderer core tst_line_curve_line_renderer.cpp)
add_qaccelplot_test(tst_marker_shapes        core tst_marker_shapes.cpp)
add_qaccelplot_test(tst_rectangle_series_data  core tst_rectangle_series_data.cpp)
add_qaccelplot_test(tst_rectangle_hover      core tst_rectangle_hover.cpp)
add_qaccelplot_test(tst_rectangle_series_rendering core tst_rectangle_series_rendering.cpp)
add_qaccelplot_test(tst_rect_vertex_cache    core tst_rect_vertex_cache.cpp)
add_qaccelplot_test(tst_bar_series_data      core tst_bar_series_data.cpp)
add_qaccelplot_test(tst_bar_hover            core tst_bar_hover.cpp)
add_qaccelplot_test(tst_bar_series_rendering core tst_bar_series_rendering.cpp)
add_qaccelplot_test(tst_band_series_data     core tst_band_series_data.cpp)
add_qaccelplot_test(tst_band_series_rendering core tst_band_series_rendering.cpp)
add_qaccelplot_test(tst_band_series_capacity core tst_band_series_capacity.cpp)
add_qaccelplot_test(tst_point_spatial_index  core tst_point_spatial_index.cpp)
add_qaccelplot_test(tst_point_cloud_data     core tst_point_cloud_data.cpp)
add_qaccelplot_test(tst_point_cloud_capacity core tst_point_cloud_capacity.cpp)
add_qaccelplot_test(tst_line_styles          core tst_line_styles.cpp)
add_qaccelplot_test(tst_data_anchor          core tst_data_anchor.cpp)
add_qaccelplot_test(tst_axis_rescale         core tst_axis_rescale.cpp)
add_qaccelplot_test(tst_plot_series_data_ranges core tst_plot_series_data_ranges.cpp)
add_qaccelplot_test(tst_plot_series_registration core tst_plot_series_registration.cpp)
add_qaccelplot_test(tst_color_palette        core tst_color_palette.cpp)
add_qaccelplot_test(tst_colormap             core tst_colormap.cpp)
add_qaccelplot_test(tst_color_bar            core tst_color_bar.cpp)
add_qaccelplot_test(tst_property_notifications core tst_property_notifications.cpp)
add_qaccelplot_test(tst_plot_rendering       core tst_plot_rendering.cpp)
add_qaccelplot_test(tst_series_opacity       core tst_series_opacity.cpp)
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
# Renders a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_series_opacity PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_series_opacity)
# Renders a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_rectangle_series_rendering PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_rectangle_series_rendering)
# Renders a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_bar_series_rendering PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_bar_series_rendering)
# Loads a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_line_curve_hover PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_line_curve_hover)
# Renders a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_band_series_rendering PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_band_series_rendering)

add_qaccelplot_test(tst_series_inspection  core tst_series_inspection.cpp)
add_qaccelplot_test(tst_inspection_row_model core tst_inspection_row_model.cpp)
add_qaccelplot_test(tst_plot_inspector     core tst_plot_inspector.cpp InspectionPlotRig.cpp)
# Loads the tooltip component, so it needs the QML module plugin.
target_link_libraries(tst_plot_inspector PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_plot_inspector)
add_qaccelplot_test(tst_selection_tool     core tst_selection_tool.cpp InspectionPlotRig.cpp)
add_qaccelplot_test(tst_inspection_qml     core tst_inspection_qml.cpp)
# Loads a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_inspection_qml PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_inspection_qml)

add_qaccelplot_test(tst_histogram          core tst_histogram.cpp)
add_qaccelplot_test(tst_histogram_qml      core tst_histogram_qml.cpp)
# Loads a QML scene, so it needs the QML module plugin.
target_link_libraries(tst_histogram_qml PRIVATE QAccelPlot::QAccelPlot)
qt_import_qml_plugins(tst_histogram_qml)
