//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot
import "tools"

Window {
    id: window

    property int pointCount: 1001
    property var tools: []
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1200
    height: 800
    visible: true
    title: "Terrain Measurements"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    Typography {
        id: typography
    }

    function addTool(component, properties) {
        const tool = component.createObject(toolLayer, properties);
        if (tool) {
            tools = tools.concat([tool]);
            activateTool(tool);
        }
        return tool;
    }

    function activateTool(selectedTool) {
        for (const tool of tools) {
            if (tool !== selectedTool) {
                tool.active = false;
            }
        }
        if (selectedTool) {
            selectedTool.active = true;
        }
    }

    function removeSelectedTool() {
        for (let i = tools.length - 1; i >= 0; --i) {
            if (tools[i].active) {
                tools[i].destroy();
                tools.splice(i, 1);
                tools = tools;
                return;
            }
        }
    }

    function clearTools() {
        for (const tool of tools) {
            tool.destroy();
        }
        tools = [];
    }

    function addRuler(x, y) {
        if (x === undefined || y === undefined) {
            x = 80 + Math.random() * 760;
            y = 18 + Math.random() * 42;
        }
        return addTool(rulerComponent, {
            x1Data: x,
            y1Data: y,
            x2Data: x + 220,
            y2Data: y + 6
        });
    }

    function addAngle(x, y) {
        if (x === undefined || y === undefined) {
            x = 120 + Math.random() * 700;
            y = 20 + Math.random() * 42;
        }
        return addTool(angleComponent, {
            vertexXData: x,
            vertexYData: y,
            arm1XData: x + 100,
            arm1YData: y,
            arm2XData: x,
            arm2YData: y + 16
        });
    }

    function addMarker(x, y) {
        if (x === undefined || y === undefined) {
            x = 80 + Math.random() * 840;
            y = 12 + Math.random() * 58;
        }
        return addTool(markerComponent, {
            dataX: x,
            dataY: y
        });
    }

    function addRegion(x, y) {
        if (x === undefined || y === undefined) {
            x = 120 + Math.random() * 760;
            y = 18 + Math.random() * 44;
        }
        return addTool(regionComponent, {
            x1Data: x - 60,
            y1Data: y - 10,
            x2Data: x + 60,
            y2Data: y + 10
        });
    }

    Component.onCompleted: addRuler(120, 40)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Terrain cross-section measurements"
            color: colorPalette.text
            font.bold: true
            font.pixelSize: 18
        }

        Label {
            text: "Drag a measurement or its handles. All axes use metres, so angle values are measured in data space."
            color: colorPalette.textSecondary
            font.pixelSize: 12
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Add ruler"
                onClicked: window.addRuler()
            }
            Button {
                text: "Add angle"
                onClicked: window.addAngle()
            }
            Button {
                text: "Add point"
                onClicked: window.addMarker()
            }
            Button {
                text: "Add region"
                onClicked: window.addRegion()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: "Delete selected"
                enabled: window.tools.length > 0
                onClicked: window.removeSelectedTool()
            }
            Button {
                text: "Clear"
                enabled: window.tools.length > 0
                onClicked: window.clearTools()
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            border.color: colorPalette.plotBorder
            border.width: 2
            grid.gridVisible: false
            grid.subGridVisible: false

            // Tools use plot-local coordinates, while this layer provides a
            // rectangular clip exactly matching the drawable plot area.
            Item {
                id: toolLayer
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                property var xAxis: plot.xAxis
                property var yAxis: plot.yAxis
                signal plotRectChanged

                function dataToPixelX(value) {
                    return plot.dataToPixelX(value) - plot.plotRect.x;
                }

                function dataToPixelY(value) {
                    return plot.dataToPixelY(value) - plot.plotRect.y;
                }

                function pixelToDataX(value) {
                    return plot.pixelToDataX(value + plot.plotRect.x);
                }

                function pixelToDataY(value) {
                    return plot.pixelToDataY(value + plot.plotRect.y);
                }

                function isInsidePlotArea(x, y) {
                    return x >= 0 && x <= width && y >= 0 && y <= height;
                }

                Connections {
                    target: plot
                    function onPlotRectChanged() {
                        toolLayer.plotRectChanged();
                    }
                }
            }

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 1000
                dataMin: 0
                dataMax: 1000
                side: QAccelPlot.Axis.Bottom
                label: "Distance (m)"
                baselineColor: colorPalette.axisLine
                baselineWidth: 2
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 80
                dataMin: 0
                dataMax: 80
                side: QAccelPlot.Axis.Left
                axisTitlePadding: 40
                layoutSize: 60
                label: "Elevation (m)"
                baselineColor: colorPalette.axisLine
                baselineWidth: 2
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            onMousePressed: event => {
                if (event.button === Qt.RightButton) {
                    toolMenu.spawnDataX = pixelToDataX(event.x);
                    toolMenu.spawnDataY = pixelToDataY(event.y);
                    toolMenu.popup();
                }
            }

            Component {
                id: rulerComponent
                RulerTool {
                    palette: colorPalette
                }
            }
            Component {
                id: angleComponent
                AngleTool {
                    palette: colorPalette
                }
            }
            Component {
                id: markerComponent
                PointMarkerTool {
                    palette: colorPalette
                }
            }
            Component {
                id: regionComponent
                RectangleTool {
                    palette: colorPalette
                }
            }

            QAccelPlot.LineCurve {
                objectName: "terrainProfile"
                name: "Elevation profile"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: hovered ? 4 : 3
                antialiasingEnabled: true
            }
        }
    }
}
