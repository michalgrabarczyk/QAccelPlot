//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot

PlotView {
    id: root

    property Item legend: Legend {
        series: root.series
    }

    property bool legendVisible: true

    // Reparent legend into this item whenever it changes
    onLegendChanged: internal.setupLegend()
    Component.onCompleted: {
        internal.setupLegend();
    }

    QtObject {
        id: internal
        property Item activeLegend: null

        function setupLegend() {
            if (activeLegend && activeLegend !== root.legend && activeLegend.parent === root) {
                activeLegend.parent = null;
            }
            activeLegend = root.legend;
            if (root.legend) {
                root.legend.parent = root;
                root.legend.z = 1;
            }
        }
    }

    Binding {
        target: root.legend
        property: "visible"
        value: root.legend ? (root.legendVisible && (root.legend.series !== undefined ? root.legend.series.length > 0 : true)) : false
        when: root.legend !== null
    }

    Binding {
        target: root.legend
        property: "x"
        value: root.legend ? root.plotRect.x + root.plotRect.width - root.legend.width - 8 : 0
        when: root.legend !== null
    }

    Binding {
        target: root.legend
        property: "y"
        value: root.plotRect.y + 8
        when: root.legend !== null
    }
}
