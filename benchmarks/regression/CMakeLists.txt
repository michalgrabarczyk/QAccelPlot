#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
qt_add_executable(QAccelPlotBenchDataIngestion
    BenchDataIngestion.cpp
)

target_link_libraries(QAccelPlotBenchDataIngestion
    PRIVATE QAccelPlot::Core QAccelPlotBenchmarkCommon Qt6::Quick
)

qt_add_executable(QAccelPlotBenchmarkRegression
    MANUAL_FINALIZATION
    main.cpp
)

target_link_libraries(QAccelPlotBenchmarkRegression
    PRIVATE QAccelPlot::Core QAccelPlotBenchmarkCommon Qt6::Quick
)

# Automoc is needed for main.cpp since we have a QObject subclass inside it.
set_property(TARGET QAccelPlotBenchmarkRegression PROPERTY AUTOMOC ON)

# We might want to use some QML internally or link to plugins, but for now we just link QAccelPlot directly.
# qt_import_qml_plugins(QAccelPlotBenchmarkRegression)
qt_finalize_executable(QAccelPlotBenchmarkRegression)
