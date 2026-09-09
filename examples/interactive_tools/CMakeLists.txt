#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
qaccelplot_add_example(InteractiveTools
    SOURCES
        src/main.cpp
    QML_FILES
        qml/main.qml
        qml/tools/AngleTool.qml
        qml/tools/PointMarkerTool.qml
        qml/tools/RectangleTool.qml
        qml/tools/RulerTool.qml
)
