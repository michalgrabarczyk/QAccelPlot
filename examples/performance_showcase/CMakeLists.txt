#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
qaccelplot_add_example(PerformanceShowcase
    SOURCES
        src/DataDeliveryMetrics.cpp
        src/DataGenerationWorker.cpp
        src/main.cpp
    QML_FILES
        qml/ControlPanel.qml
        qml/main.qml
)
