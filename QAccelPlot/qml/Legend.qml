//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

Rectangle {
    id: root

    required property var series
    property color textColor: "#333333"
    property font font: Qt.font({
        family: "Arial",
        pixelSize: 12
    })
    property real symbolWidth: 28

    visible: series.length > 0

    width: column.width + 16
    height: column.height + 12

    color: Qt.rgba(1, 1, 1, 0.7)
    radius: 4
    border.color: Qt.rgba(0, 0, 0, 0.15)
    border.width: 1

    Column {
        id: column

        x: 8
        y: 6
        spacing: 4

        Repeater {
            id: curveRepeater

            model: root.series

            Row {
                id: row

                required property var modelData
                required property int index

                spacing: 6

                LegendSymbol {
                    sourceSeries: row.modelData
                    requestedWidth: root.symbolWidth
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: row.modelData.name || row.modelData.objectName || ("Curve " + (row.index + 1))
                    font: root.font
                    color: root.textColor
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
