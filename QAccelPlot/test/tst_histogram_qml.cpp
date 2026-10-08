//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest>

#include <memory>

class TestHistogramQml : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void equalBinsFeedABarSeries();
    void binCountSpansTheSamples();
    void edgesFeedABarSeriesAsDensity();
    void generatedEdgesAreAccepted();
    void fromCountsGivesBars();
    void invalidEdgesGiveAnEmptyHistogram();

private:
    QString call(const char* function) const;

    QQmlEngine engine_;
    std::unique_ptr<QObject> root_;
};

void TestHistogramQml::initTestCase()
{
    auto component = QQmlComponent{&engine_};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Plot {
            id: root
            width: 400; height: 300
            readonly property var samples: [0.5, 1.5, 1.5, 2.5, 2.5, 2.5, 4]
            xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 4 }
            yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 4 }
            QAccelPlot.BarSeries { id: bars; xAxis: root.xAxis; yAxis: root.yAxis }
            function list(values) {
                const items = [];
                for (let i = 0; i < values.length; ++i) {
                    items.push(Number(values[i].toFixed(4)));
                }
                return items.join(",");
            }
            function equalBins() {
                const histogram = QAccelPlot.Histogram.fromSamples(samples, 4, 0, 4);
                bars.setData(histogram.bars());
                const bar = bars.barAt(2);
                return [histogram.binCount, histogram.uniform, histogram.binWidth, histogram.total, list(histogram.edges),
                    list(histogram.values), bars.count, bar.from, bar.to, bar.value].join("|");
            }
            function binCount() {
                const histogram = QAccelPlot.Histogram.fromSamples(samples, 7);
                return [histogram.binCount, histogram.edges[0], histogram.edges[7], list(histogram.values)].join("|");
            }
            function edges() {
                const histogram = QAccelPlot.Histogram.fromSamples(samples, [0, 1, 4]);
                const density = histogram.density();
                bars.setData(density.bars());
                const bar = bars.barAt(1);
                return [histogram.binCount, histogram.uniform, isNaN(histogram.binWidth), list(histogram.values), list(density.values),
                    bars.count, bar.from, bar.to, bar.value.toFixed(4), root.xAxis.dataMin, root.xAxis.dataMax].join("|");
            }
            function generatedEdges() {
                const data = [1, 5, 50, 100];
                const logEdges = QAccelPlot.Histogram.logEdges(1, 100, 2);
                const linearEdges = QAccelPlot.Histogram.linearEdges(0, 100, 4);
                const decadeEdges = QAccelPlot.Histogram.decadeEdges(1, 30);
                return [list(logEdges), list(QAccelPlot.Histogram.fromSamples(data, logEdges).values), list(linearEdges),
                    list(QAccelPlot.Histogram.fromSamples(data, linearEdges).values), list(decadeEdges),
                    list(QAccelPlot.Histogram.fromSamples(data, decadeEdges).values)].join("|");
            }
            function counts() {
                const histogram = QAccelPlot.Histogram.fromCounts([0, 1, 2], [3, 5]);
                const bar = histogram.bars()[1];
                return [histogram.total, list(histogram.values), bar.from, bar.to, bar.value].join("|");
            }
            function invalidEdges() {
                const histogram = QAccelPlot.Histogram.fromSamples(samples, [2, 1]);
                return [histogram.binCount, histogram.bars().length, histogram.density().binCount].join("|");
            }
        }
    )",
        QUrl{});
    root_.reset(component.create());
    QVERIFY2(root_ != nullptr, qPrintable(component.errorString()));
}

void TestHistogramQml::equalBinsFeedABarSeries()
{
    QCOMPARE(call("equalBins"), QStringLiteral("4|true|1|7|0,1,2,3,4|1,2,3,1|4|2|3|3"));
}

void TestHistogramQml::binCountSpansTheSamples()
{
    QCOMPARE(call("binCount"), QStringLiteral("7|0.5|4|1,0,2,0,3,0,1"));
}

void TestHistogramQml::edgesFeedABarSeriesAsDensity()
{
    QCOMPARE(call("edges"), QStringLiteral("2|false|true|1,6|0.1429,0.2857|2|1|4|0.2857|0|4"));
}

void TestHistogramQml::generatedEdgesAreAccepted()
{
    QCOMPARE(call("generatedEdges"), QStringLiteral("1,10,100|2,2|0,25,50,75,100|2,0,1,1|1,2,3,4,5,6,7,8,9,10,20,30|1,0,0,0,1,0,0,0,0,0,0"));
}

void TestHistogramQml::fromCountsGivesBars()
{
    QCOMPARE(call("counts"), QStringLiteral("8|3,5|1|2|5"));
}

void TestHistogramQml::invalidEdgesGiveAnEmptyHistogram()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Histogram needs at least two finite, strictly increasing edges"));
    QCOMPARE(call("invalidEdges"), QStringLiteral("0|0|0"));
}

QString TestHistogramQml::call(const char* function) const
{
    auto result = QVariant{};
    QMetaObject::invokeMethod(root_.get(), function, Q_RETURN_ARG(QVariant, result));
    return result.toString();
}

QTEST_MAIN(TestHistogramQml)
#include "tst_histogram_qml.moc"
