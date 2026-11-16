#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: MIT
#
qaccelplot_add_example(Heatmap
    SOURCES
        src/main.cpp
        src/VisibleHistogram.cpp
        src/VisibleHistogram.hpp
    QML_FILES
        qml/main.qml
        qml/AmplitudeHistogram.qml
        qml/HeatmapColorBar.qml
        qml/InterferencePage.qml
        qml/LogBinsPage.qml
)
