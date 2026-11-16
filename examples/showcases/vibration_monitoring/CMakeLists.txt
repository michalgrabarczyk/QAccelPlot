#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: MIT
#
qaccelplot_add_example(VibrationMonitoring
    SOURCES
        src/AcquisitionClock.cpp
        src/AcquisitionWorker.cpp
        src/LevelHistogram.cpp
        src/MonitorController.cpp
        src/SpectrumAnalyzer.cpp
        src/VibrationModel.cpp
        src/main.cpp
    QML_FILES
        qml/DashboardPanel.qml
        qml/DemoTimeline.qml
        qml/RmsPanel.qml
        qml/SpectrogramPanel.qml
        qml/SpectrumPanel.qml
        qml/StatusStrip.qml
        qml/WaveformPanel.qml
        qml/main.qml
)
