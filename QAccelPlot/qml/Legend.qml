//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
// Qt 6.2's implicit import of this module's directory only exposes its QML files. The explicit
// module import is needed for the C++ Colors singleton; without it, the color bindings fail.
import QAccelPlot

Rectangle {
    id: root

    // One row per entry; Plot provides a model over all its series as legendModel.
    required property LegendModel model
    property color textColor: Colors.dark.text
    property font font: Qt.font({
        pixelSize: 12
    })
    property real symbolWidth: 28

    visible: model.count > 0

    width: column.width + 16
    height: column.height + 12

    color: Colors.dark.legendBackground
    radius: 4
    border.color: Colors.dark.legendBorder
    border.width: 1

    Column {
        id: column

        x: 8
        y: 6
        spacing: 4

        Repeater {
            id: entryRepeater

            model: root.model

            Row {
                id: row

                required property string name
                required property var entry

                spacing: 6

                LegendSymbol {
                    entry: row.entry
                    requestedWidth: root.symbolWidth
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: row.name
                    font: root.font
                    color: root.textColor
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
