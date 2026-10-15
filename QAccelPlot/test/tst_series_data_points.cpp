//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BandSeries.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QtTest/QtTest>

#include <limits>
#include <memory>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {

enum class SeriesType { Line, Points, Rectangles, Bars, Band };

std::unique_ptr<PlotSeries> makeSeries(const SeriesType type)
{
    switch (type) {
    case SeriesType::Line:
        return std::make_unique<LineCurve>();
    case SeriesType::Points:
        return std::make_unique<PointCloud>();
    case SeriesType::Rectangles:
        return std::make_unique<RectangleSeries>();
    case SeriesType::Bars:
        return std::make_unique<BarSeries>();
    case SeriesType::Band:
        return std::make_unique<BandSeries>();
    }
    return {};
}

// Data points of finite, positive values that every series accepts.
template <typename T> std::vector<T> makeValues(const int count, const int stride)
{
    auto values = std::vector<T>(static_cast<std::size_t>(count) * static_cast<std::size_t>(stride));
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        values[i] = static_cast<T>(1 + i);
    }
    return values;
}

} // namespace

class SeriesDataPointsTest : public QObject {
    Q_OBJECT

private slots:
    void invalidArgumentsAreRejected_data();
    void invalidArgumentsAreRejected();
    void nullDataWithoutDataPointsIsAccepted_data();
    void nullDataWithoutDataPointsIsAccepted();
    void rawDataIsCopied_data();
    void rawDataIsCopied();
    void postedDataIsAppliedFromWorkerThread_data();
    void postedDataIsAppliedFromWorkerThread();
    void dataBoundsReplaceTheRangeScan_data();
    void dataBoundsReplaceTheRangeScan();

private:
    void addSeriesRows();
};

void SeriesDataPointsTest::addSeriesRows()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("stride");
    QTest::addColumn<QString>("series");
    QTest::addColumn<QString>("countName");
    QTest::addColumn<QString>("plural");
    QTest::addColumn<QString>("doubleUnit");
    QTest::addColumn<QString>("floatUnit");

    QTest::newRow("LineCurve") << static_cast<int>(SeriesType::Line) << 2 << "LineCurve" << "data point" << "points" << "doubles" << "floats";
    QTest::newRow("PointCloud") << static_cast<int>(SeriesType::Points) << 2 << "PointCloud" << "data point" << "points" << "coordinates" << "coordinates";
    QTest::newRow("RectangleSeries") << static_cast<int>(SeriesType::Rectangles) << 4 << "RectangleSeries" << "data rectangle" << "rectangles" << "coordinates"
                                     << "coordinates";
    QTest::newRow("BarSeries") << static_cast<int>(SeriesType::Bars) << 2 << "BarSeries" << "data bar" << "bars" << "values" << "values";
    QTest::newRow("BandSeries") << static_cast<int>(SeriesType::Band) << 3 << "BandSeries" << "data point" << "data points" << "values" << "values";
}

void SeriesDataPointsTest::invalidArgumentsAreRejected_data()
{
    addSeriesRows();
}

void SeriesDataPointsTest::invalidArgumentsAreRejected()
{
    QFETCH(int, type);
    QFETCH(int, stride);
    QFETCH(QString, series);
    QFETCH(QString, countName);
    QFETCH(QString, plural);
    QFETCH(QString, doubleUnit);
    QFETCH(QString, floatUnit);
    auto plotSeries = makeSeries(static_cast<SeriesType>(type));
    plotSeries->setData(makeValues<double>(1, stride), 1);
    auto revisionSpy = QSignalSpy{plotSeries.get(), &PlotSeries::dataRevisionChanged};
    const auto doubles = makeValues<double>(2, stride);
    const auto floats = makeValues<float>(2, stride);
    const auto negative = QRegularExpression(QStringLiteral("%1 %2 count cannot be negative: -1").arg(series, countName));
    const auto nullData = QRegularExpression(QStringLiteral("%1 received a null data pointer for 2 %2").arg(series, plural));
    const auto expected = QString::number(2 * stride);

    QTest::ignoreMessage(QtWarningMsg, nullData);
    plotSeries->setData(static_cast<const double*>(nullptr), 2);
    QTest::ignoreMessage(QtWarningMsg, nullData);
    plotSeries->setDataF(static_cast<const float*>(nullptr), 2);
    QTest::ignoreMessage(QtWarningMsg, negative);
    plotSeries->setData(doubles.data(), -1);
    QTest::ignoreMessage(QtWarningMsg, negative);
    plotSeries->setDataF(std::vector<float>{}, -1);
    QTest::ignoreMessage(QtWarningMsg,
        QRegularExpression(QStringLiteral("%1 received %2 %3 for 1 %4; expected %5").arg(series, expected, doubleUnit, plural, QString::number(stride))));
    plotSeries->setData(std::vector<double>(doubles), 1);
    QTest::ignoreMessage(QtWarningMsg,
        QRegularExpression(QStringLiteral("%1 received %2 %3 for 3 %4; expected %5").arg(series, expected, floatUnit, plural, QString::number(3 * stride))));
    plotSeries->setDataF(std::vector<float>(floats), 3);
    QTest::ignoreMessage(QtWarningMsg, negative);
    plotSeries->postData(std::vector<double>{}, -1);
    QCoreApplication::sendPostedEvents();

    QCOMPARE(revisionSpy.count(), 0);
}

void SeriesDataPointsTest::nullDataWithoutDataPointsIsAccepted_data()
{
    addSeriesRows();
}

void SeriesDataPointsTest::nullDataWithoutDataPointsIsAccepted()
{
    QFETCH(int, type);
    QFETCH(int, stride);
    auto axis = Axis{};
    auto plotSeries = makeSeries(static_cast<SeriesType>(type));
    plotSeries->setXAxis(&axis);
    plotSeries->setData(makeValues<double>(2, stride), 2);
    QVERIFY(plotSeries->xDataRange());

    plotSeries->setData(static_cast<const double*>(nullptr), 0);
    QVERIFY(!plotSeries->xDataRange());

    plotSeries->setDataF(makeValues<float>(2, stride), 2);
    plotSeries->setDataF(static_cast<const float*>(nullptr), 0);
    QVERIFY(!plotSeries->xDataRange());
}

void SeriesDataPointsTest::rawDataIsCopied_data()
{
    addSeriesRows();
}

void SeriesDataPointsTest::rawDataIsCopied()
{
    QFETCH(int, type);
    QFETCH(int, stride);
    auto plotSeries = makeSeries(static_cast<SeriesType>(type));
    auto doubles = makeValues<double>(3, stride);
    auto floats = makeValues<float>(3, stride);

    // The data range is scanned when it is read, so a change of the caller's array would show in it.
    plotSeries->setData(doubles.data(), 3);
    std::fill(doubles.begin(), doubles.end(), 1000.0);
    const auto doubleRange = plotSeries->xDataRange();
    QVERIFY(doubleRange);
    QVERIFY(doubleRange->max < 1000.0);

    plotSeries->setDataF(floats.data(), 3);
    std::fill(floats.begin(), floats.end(), 1000.0F);
    const auto floatRange = plotSeries->xDataRange();
    QVERIFY(floatRange);
    QVERIFY(floatRange->max < 1000.0);
}

void SeriesDataPointsTest::postedDataIsAppliedFromWorkerThread_data()
{
    addSeriesRows();
}

void SeriesDataPointsTest::postedDataIsAppliedFromWorkerThread()
{
    QFETCH(int, type);
    QFETCH(int, stride);
    auto plotSeries = makeSeries(static_cast<SeriesType>(type));
    auto revisionSpy = QSignalSpy{plotSeries.get(), &PlotSeries::dataRevisionChanged};

    auto worker = std::thread{[&plotSeries, stride]() {
        plotSeries->postData(makeValues<double>(2, stride), 2);
        plotSeries->postData(makeValues<float>(3, stride), 3);
    }};
    worker.join();

    // Queued: nothing is applied until the event loop runs.
    QCOMPARE(revisionSpy.count(), 0);
    QTRY_COMPARE(revisionSpy.count(), 2);
    QVERIFY(plotSeries->xDataRange());
}

void SeriesDataPointsTest::dataBoundsReplaceTheRangeScan_data()
{
    addSeriesRows();
}

void SeriesDataPointsTest::dataBoundsReplaceTheRangeScan()
{
    QFETCH(int, type);
    QFETCH(int, stride);
    QFETCH(QString, series);
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto plotSeries = makeSeries(static_cast<SeriesType>(type));
    plotSeries->setXAxis(&xAxis);
    plotSeries->setYAxis(&yAxis);

    plotSeries->setData(makeValues<double>(2, stride), 2, PlotSeries::DataBounds{-10.0, 10.0, -20.0, 20.0});
    QCOMPARE(xAxis.dataMin(), -10.0);
    QCOMPARE(xAxis.dataMax(), 10.0);
    QCOMPARE(yAxis.dataMin(), -20.0);
    QCOMPARE(yAxis.dataMax(), 20.0);

    // An update without bounds is scanned again.
    const auto raw = makeValues<double>(2, stride);
    plotSeries->setData(raw.data(), 2);
    QVERIFY(xAxis.dataMin() > 0.0);

    plotSeries->setData(raw.data(), 2, PlotSeries::DataBounds{-4.0, 4.0, -2.0, 2.0});
    QCOMPARE(xAxis.dataMin(), -4.0);
    QCOMPARE(yAxis.dataMax(), 2.0);

    const auto floats = makeValues<float>(3, stride);
    plotSeries->setDataF(floats.data(), 3, PlotSeries::DataBounds{-8.0, 8.0, -6.0, 6.0});
    QCOMPARE(xAxis.dataMax(), 8.0);
    QCOMPARE(yAxis.dataMin(), -6.0);

    plotSeries->setDataF(makeValues<float>(3, stride), 3, PlotSeries::DataBounds{-9.0, 9.0, -7.0, 7.0});
    QCOMPARE(xAxis.dataMin(), -9.0);
    QCOMPARE(yAxis.dataMax(), 7.0);

    // A dimension whose bounds are not finite or not ordered reports no extent.
    plotSeries->setDataF(makeValues<float>(3, stride), 3, PlotSeries::DataBounds{std::numeric_limits<qreal>::quiet_NaN(), 9.0, 5.0, -5.0});
    QVERIFY(!plotSeries->xDataRange());
    QVERIFY(!plotSeries->yDataRange());

    // Rejected data does not install its bounds.
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("%1 received a null data pointer.*").arg(series)));
    plotSeries->setData(static_cast<const double*>(nullptr), 1, PlotSeries::DataBounds{0.0, 1.0, 0.0, 1.0});
    QVERIFY(!plotSeries->xDataRange());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::SeriesDataPointsTest)
#include "tst_series_data_points.moc"
