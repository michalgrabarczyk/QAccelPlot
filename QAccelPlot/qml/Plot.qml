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

    // Legend entries of every visible series that has one.
    readonly property LegendModel legendModel: LegendModel {
        series: root.series
    }

    property Item legend: Legend {
        model: root.legendModel
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

    // While legendVisible is set, the legend decides its own visibility, as Legend does from its model.
    Binding {
        target: root.legend
        property: "visible"
        value: false
        when: root.legend !== null && !root.legendVisible
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
