//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QAccelPlot/inspection/PlotInspector.hpp>
#include <QAccelPlot/inspection/SelectionTool.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest>

#include <memory>
#include <vector>

using namespace QAccelPlot;

class TestInspectionQml : public QObject {
    Q_OBJECT
private slots:
    void queriesRowsAndSelection();
};

void TestInspectionQml::queriesRowsAndSelection()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Plot {
            id: root
            width: 800; height: 400
            property alias curve: curve
            property alias inspector: inspector
            property alias tool: tool
            xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }
            yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }
            QAccelPlot.LineCurve { id: curve; name: "A"; xAxis: root.xAxis; yAxis: root.yAxis }
            QAccelPlot.LineCurve { id: other; xAxis: root.xAxis; yAxis: root.yAxis }
            QAccelPlot.PlotInspector {
                id: inspector
                plot: root; includedSeries: [curve]; followPointer: false; cursorX: 3
                QAccelPlot.Crosshair { id: crosshair; axisLabels: true }
                QAccelPlot.InspectionMarkers { id: markers }
                QAccelPlot.InspectionTooltip { id: tooltip }
            }
            QAccelPlot.SelectionTool { id: tool; plot: root }
            // The overlay item is above the series and the legend. Components declared inside the inspector
            // receive it and stack in declaration order, above the selection tool's rectangle.
            function layers() {
                const order = [crosshair, markers, tooltip].map(item => {
                    for (let i = 0; i < root.overlay.children.length; ++i) {
                        if (root.overlay.children[i] === item) {
                            return i;
                        }
                    }
                    return -1;
                });
                return [root.overlay.parent === root, root.overlay.z > curve.z, root.overlay.z > root.legend.z,
                    root.overlay.width === root.width, root.overlay.height === root.height, crosshair.inspector === inspector,
                    tool.rectangleVisible, root.overlay.children.length].concat(order).join("|");
            }
            function queries() {
                const inspection = curve.inspection;
                const sample = inspection.nearestByX(curve.width / 2);
                const bracket = inspection.bracketByX(curve.width / 4);
                const byY = inspection.nearestByY(0);
                const bracketY = inspection.bracketByY(curve.height / 2);
                const summary = inspection.summarizeRange(-Infinity, Infinity);
                const page = inspection.indices(-Infinity, Infinity, -Infinity, Infinity, 1, 10);
                return [inspection.status === QAccelPlot.Inspection.Ready, sample.valid, sample.status === QAccelPlot.Inspection.Ready,
                    sample.index, sample.y, bracket.left.index, bracket.interpolated.y.toFixed(6), summary.count, summary.mean, page.total,
                    page.indices.length, page.indices[0], root.xAxis.formatValue(sample.x, curve.width), byY.index, bracketY.left.index,
                    QAccelPlot.PlotInspector.NearestY].join("|");
            }
            function rows() {
                const row = inspector.model.get(0);
                return [inspector.model.count, row.seriesName, row.valid, row.sampleIndex, row.yText].join("|");
            }
            function selection() {
                tool.select(0, 5, -Infinity, Infinity);
                return [tool.hasSelection, tool.yMax === Infinity, tool.model.get(0).summaryCount, tool.indices(curve).indices.length].join("|");
            }
        }
    )",
        QUrl{});
    const auto object = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    auto* curve = object->property("curve").value<LineCurve*>();
    auto* inspector = object->property("inspector").value<PlotInspector*>();
    QVERIFY(curve && inspector);
    QCOMPARE(inspector->includedSeries(), QList<PlotSeries*>{curve});
    curve->setData(std::vector<double>{2, 2, 4, 6, 8, 4}, 3);
    const auto call = [&](const char* function) {
        auto result = QVariant{};
        QMetaObject::invokeMethod(object.get(), function, Q_RETURN_ARG(QVariant, result));
        return result.toString();
    };
    QTRY_VERIFY(curve->width() > 0);
    QCOMPARE(call("queries"), QStringLiteral("true|true|true|1|6|0|3.000000|3|4|3|2|1|4.000|1|2|1"));
    QTRY_COMPARE(inspector->validCount(), 1);
    QCOMPARE(call("rows"), QStringLiteral("1|A|true|1|6.000"));
    QCOMPARE(call("selection"), QStringLiteral("true|true|2|2"));
    QCOMPARE(call("layers"), QStringLiteral("true|true|true|true|true|true|true|4|1|2|3"));
}

QTEST_MAIN(TestInspectionQml)
#include "tst_inspection_qml.moc"
