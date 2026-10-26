#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
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
