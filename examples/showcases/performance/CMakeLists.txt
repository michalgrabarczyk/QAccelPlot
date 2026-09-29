#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
qaccelplot_add_example(Performance
    SOURCES
        src/DataDeliveryMetrics.cpp
        src/GalaxyGenerator.cpp
        src/PlasmaGenerator.cpp
        src/ShowcaseController.cpp
        src/SineLookup.cpp
        src/SineWaveGenerator.cpp
        src/main.cpp
    QML_FILES
        qml/ControlPanel.qml
        qml/LineCurvePage.qml
        qml/PointCloudPage.qml
        qml/RectangleSeriesPage.qml
        qml/main.qml
)
