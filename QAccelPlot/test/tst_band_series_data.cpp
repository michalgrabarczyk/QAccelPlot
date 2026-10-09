//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HoverEvents.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/linestyles/DashLine.hpp"
#include "QAccelPlot/materials/BandEdgeMaterial.hpp"
#include "QAccelPlot/materials/BandMaterial.hpp"
#include "QAccelPlot/materials/LineMaterial.hpp"
#include "QAccelPlot/series/BandSeries.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<double>::infinity();

class RenderableBand final : public BandSeries {
public:
    using BandSeries::updatePaintNode;
};

// A band bound to axes spanning 0..10 in a 100 × 100 px item, so one data unit is 10 px.
struct BandFixture {
    BandFixture()
    {
        for (auto* axis : {&xAxis, &yAxis}) {
            axis->setViewportMin(0.0);
            axis->setViewportMax(10.0);
        }
        yAxis.setOrientation(Axis::Vertical);
        band.setParentItem(window.contentItem());
        band.setSize({100.0, 100.0});
        band.setPlotRect({0.0, 0.0, 100.0, 100.0});
        band.setXAxis(&xAxis);
        band.setYAxis(&yAxis);
    }

    QSGNode* paint()
    {
        node.reset(band.updatePaintNode(node.release(), nullptr));
        return node.get();
    }

    BandMaterial* fillMaterial() const
    {
        return static_cast<BandMaterial*>(static_cast<QSGGeometryNode*>(node->firstChild())->material());
    }

    QQuickWindow window;
    Axis xAxis;
    Axis yAxis;
    RenderableBand band;
    std::unique_ptr<QSGNode> node;
};

// Returns the line material of the edge line painted as child \a index of the band's root node.
LineMaterial* edgeMaterial(QSGNode* root, const int index)
{
    return static_cast<LineMaterial*>(static_cast<QSGGeometryNode*>(root->childAtIndex(index))->material());
}

bool hasSpan(const QVariantMap& value, const double low, const double high)
{
    return !value.isEmpty() && qFuzzyCompare(value.value(QStringLiteral("low")).toDouble(), low)
        && qFuzzyCompare(value.value(QStringLiteral("high")).toDouble(), high);
}

} // namespace

class BandSeriesDataTest : public QObject {
    Q_OBJECT

private slots:
    void invalidArgumentsAreRejected();
    void listsUseTheShortestLength();
    void rangesCoverLowAndHighValues();
    void invalidValuesAreSkippedInRanges();
    void dataBoundsReplaceTheRangeScan();
    void floatDataIsApplied();
    void rawDoubleDataPreservesModernEpochPrecision();
    void appendDataExtendsSamplesAndRanges();
    void appendToFloatDataKeepsSamples();
    void postedDataIsAppliedFromWorkerThread();
    void clearDataRemovesSamples();
    void valueAtInterpolatesBetweenSamples();
    void valueAtInterpolatesOnLogScales();
    void valueAtNeedsValidAscendingSamples();
    void containsTestsTheBandUnderTheCursor();
    void hoveredFollowsChangesUnderARestingCursor();
    void edgeSettingsClampAndNotify();
    void edgeLinesReadTheBandSamples();
    void dashLengthsFollowZoomButNotPan();
    void appendingKeepsTheVertexBuffers();
    void renderOriginFollowsTheViewport();
    void fewerThanTwoSamplesDrawNothing();
};

void BandSeriesDataTest::invalidArgumentsAreRejected()
{
    auto band = BandSeries{};
    auto countSpy = QSignalSpy{&band, &BandSeries::countChanged};
    band.setData(std::vector<double>{0.0, 1.0, 2.0}, 1);
    QCOMPARE(band.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BandSeries received a null data pointer for 1 samples"));
    band.setData(static_cast<const double*>(nullptr), 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BandSeries sample count cannot be negative: -1"));
    band.setDataF(static_cast<const float*>(nullptr), -1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BandSeries received 4 values for 1 samples; expected 3"));
    band.setData(std::vector<double>{0.0, 1.0, 2.0, 3.0}, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BandSeries received 2 values for 1 samples; expected 3"));
    band.setDataF(std::vector<float>{0.0f, 1.0f}, 1);

    QCOMPARE(band.count(), 1);
    QCOMPARE(countSpy.count(), 1);
}

void BandSeriesDataTest::listsUseTheShortestLength()
{
    auto band = BandSeries{};
    band.setData(QList<qreal>{0.0, 1.0, 2.0}, QList<qreal>{1.0, 2.0}, QList<qreal>{3.0, 4.0, 5.0});
    QCOMPARE(band.count(), 2);
    QVERIFY(hasSpan(band.valueAt(1.0), 2.0, 4.0));
    QVERIFY(band.valueAt(2.0).isEmpty());

    band.setData(std::vector<double>{0.0, 1.0}, std::vector<double>{1.0, 2.0}, std::vector<double>{3.0});
    QCOMPARE(band.count(), 1);
    QVERIFY(hasSpan(band.valueAt(0.0), 1.0, 3.0));
}

void BandSeriesDataTest::rangesCoverLowAndHighValues()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);

    band.setData(std::vector<double>{0.0, 1.0, 4.0, 1.0, -2.0, 5.0, 2.0, 3.0, -6.0}, 3);

    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 2.0);
    QCOMPARE(yAxis.dataMin(), -6.0);
    QCOMPARE(yAxis.dataMax(), 5.0);
}

void BandSeriesDataTest::invalidValuesAreSkippedInRanges()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);

    // A NaN low still counts the sample's x and high; infinite values never count.
    band.setData(std::vector<double>{-1.0, kNaN, 8.0, 0.0, 0.5, 2.0, kInf, 1.0, 3.0, 3.0, 4.0, -kInf}, 4);
    QCOMPARE(xAxis.dataMin(), -1.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
    QCOMPARE(yAxis.dataMin(), 0.5);
    QCOMPARE(yAxis.dataMax(), 8.0);

    xAxis.setLogScale(true);
    QCOMPARE(xAxis.dataMin(), 3.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
}

void BandSeriesDataTest::dataBoundsReplaceTheRangeScan()
{
    auto xAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setData(std::vector<double>{1.0, 0.0, 1.0, 3.0, 0.0, 1.0}, 2);
    QCOMPARE(xAxis.dataMax(), 3.0);

    band.setData(std::vector<double>{10.0, 0.0, 1.0, 30.0, 0.0, 1.0}, 2, PlotSeries::DataBounds{0.0, 100.0, 0.0, 1.0});
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 100.0);
    const auto raw = std::array<double, 3>{50.0, 0.0, 1.0};
    band.setData(raw.data(), 1, PlotSeries::DataBounds{0.0, 200.0, 0.0, 1.0});
    QCOMPARE(xAxis.dataMax(), 200.0);
    band.setDataF(std::vector<float>{60.0f, 0.0f, 1.0f}, 1, PlotSeries::DataBounds{0.0, 300.0, 0.0, 1.0});
    QCOMPARE(xAxis.dataMax(), 300.0);
    const auto rawFloat = std::array<float, 3>{70.0f, 0.0f, 1.0f};
    band.setDataF(rawFloat.data(), 1, PlotSeries::DataBounds{0.0, 75.0, 0.0, 1.0});
    QCOMPARE(band.count(), 1);
    QVERIFY(hasSpan(band.valueAt(70.0), 0.0, 1.0));
    QCOMPARE(xAxis.dataMax(), 75.0);

    // An append widens the given bounds.
    band.appendData(80.0, 0.0, 1.0);
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 80.0);

    // An update without bounds is scanned again.
    band.setDataF(rawFloat.data(), 1);
    QCOMPARE(xAxis.dataMin(), 70.0);
    QCOMPARE(xAxis.dataMax(), 70.0);
}

void BandSeriesDataTest::floatDataIsApplied()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);

    band.setDataF(std::vector<float>{0.0f, 1.0f, 3.0f, 2.0f, 2.0f, 5.0f}, 2);
    QCOMPARE(band.count(), 2);
    QVERIFY(hasSpan(band.valueAt(1.0), 1.5, 4.0));
    QCOMPARE(yAxis.dataMin(), 1.0);
    QCOMPARE(yAxis.dataMax(), 5.0);

    const auto raw = std::array<float, 3>{7.0f, -1.0f, 1.0f};
    band.setDataF(raw.data(), 1);
    QCOMPARE(band.count(), 1);
    QCOMPARE(xAxis.dataMin(), 7.0);
    QVERIFY(hasSpan(band.valueAt(7.0), -1.0, 1.0));
}

void BandSeriesDataTest::rawDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);

    const auto data = std::array<double, 6>{epochMilliseconds, 0.0, 1.0, epochMilliseconds + 3.0, 0.0, 1.0};
    band.setData(data.data(), 2);

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 3.0);
    QVERIFY(hasSpan(band.valueAt(epochMilliseconds + 1.0), 0.0, 1.0));
}

void BandSeriesDataTest::appendDataExtendsSamplesAndRanges()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);
    auto countSpy = QSignalSpy{&band, &BandSeries::countChanged};

    band.appendData(0.0, 1.0, 2.0);
    band.appendData(1.0, -1.0, 3.0);
    band.appendData(2.0, kNaN, 9.0);

    QCOMPARE(band.count(), 3);
    QCOMPARE(countSpy.count(), 3);
    QCOMPARE(xAxis.dataMax(), 2.0);
    QCOMPARE(yAxis.dataMin(), -1.0);
    QCOMPARE(yAxis.dataMax(), 9.0);
    QVERIFY(hasSpan(band.valueAt(0.5), 0.0, 2.5));
}

void BandSeriesDataTest::appendToFloatDataKeepsSamples()
{
    auto band = BandSeries{};
    band.setDataF(std::vector<float>{0.0f, 1.0f, 2.0f}, 1);
    band.appendData(2.0, 3.0, 4.0);

    QCOMPARE(band.count(), 2);
    QVERIFY(hasSpan(band.valueAt(1.0), 2.0, 3.0));

    // After clearing float data, appending starts from the new sample only.
    band.setDataF(std::vector<float>{5.0f, 6.0f, 7.0f}, 1);
    band.clearData();
    band.appendData(10.0, 0.0, 1.0);
    QCOMPARE(band.count(), 1);
    QVERIFY(hasSpan(band.valueAt(10.0), 0.0, 1.0));
    QVERIFY(band.valueAt(5.0).isEmpty());
}

void BandSeriesDataTest::postedDataIsAppliedFromWorkerThread()
{
    auto band = BandSeries{};
    auto countSpy = QSignalSpy{&band, &BandSeries::countChanged};

    auto worker = std::thread{[&band] { band.postData(std::vector<double>{0.0, 1.0, 2.0, 1.0, 1.0, 2.0}, 2); }};
    worker.join();
    QTRY_COMPARE(band.count(), 2);

    worker = std::thread{[&band] { band.postData(std::vector<float>{5.0f, 1.0f, 2.0f}, 1); }};
    worker.join();
    QTRY_COMPARE(band.count(), 1);
    QVERIFY(hasSpan(band.valueAt(5.0), 1.0, 2.0));
    QCOMPARE(countSpy.count(), 2);
}

void BandSeriesDataTest::clearDataRemovesSamples()
{
    auto band = BandSeries{};
    band.setData(std::vector<double>{0.0, 1.0, 2.0, 1.0, 1.0, 2.0}, 2);
    auto countSpy = QSignalSpy{&band, &BandSeries::countChanged};

    band.clearData();
    band.clearData();

    QCOMPARE(band.count(), 0);
    QCOMPARE(countSpy.count(), 1);
    QVERIFY(band.valueAt(0.5).isEmpty());
}

void BandSeriesDataTest::valueAtInterpolatesBetweenSamples()
{
    auto band = BandSeries{};
    // The second sample has low and high swapped; the band spans from the smaller to the larger.
    band.setData(QList<qreal>{0.0, 2.0, 4.0}, QList<qreal>{0.0, 6.0, 2.0}, QList<qreal>{4.0, 2.0, 2.0});

    QVERIFY(hasSpan(band.valueAt(0.0), 0.0, 4.0));
    QVERIFY(hasSpan(band.valueAt(1.0), 1.0, 5.0));
    QVERIFY(hasSpan(band.valueAt(2.0), 2.0, 6.0));
    QVERIFY(hasSpan(band.valueAt(3.0), 2.0, 4.0));
    QCOMPARE(band.valueAt(3.0).value(QStringLiteral("x")).toDouble(), 3.0);
    QVERIFY(band.valueAt(-0.1).isEmpty());
    QVERIFY(band.valueAt(4.1).isEmpty());
    QVERIFY(band.valueAt(kNaN).isEmpty());
}

void BandSeriesDataTest::valueAtInterpolatesOnLogScales()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setLogScale(true);
    yAxis.setLogScale(true);
    auto band = BandSeries{};
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);
    band.setData(QList<qreal>{1.0, 100.0}, QList<qreal>{1.0, 100.0}, QList<qreal>{10.0, 1000.0});

    QVERIFY(hasSpan(band.valueAt(10.0), 10.0, 100.0));
    QVERIFY(band.valueAt(0.0).isEmpty());
}

void BandSeriesDataTest::valueAtNeedsValidAscendingSamples()
{
    auto band = BandSeries{};
    band.setData(QList<qreal>{0.0, 1.0, 2.0, 3.0}, QList<qreal>{0.0, kNaN, 0.0, 0.0}, QList<qreal>{1.0, 1.0, 1.0, 1.0});
    QVERIFY(band.valueAt(0.0).size() == 3);
    QVERIFY(band.valueAt(0.5).isEmpty());
    QVERIFY(band.valueAt(1.0).isEmpty());
    QVERIFY(band.valueAt(1.5).isEmpty());
    QVERIFY(hasSpan(band.valueAt(2.5), 0.0, 1.0));

    // Equal X values keep the samples ascending, and the vertical step covers both samples.
    band.setData(QList<qreal>{0.0, 1.0, 1.0, 2.0}, QList<qreal>{0.0, 0.0, 2.0, 2.0}, QList<qreal>{1.0, 1.0, 3.0, 3.0});
    QVERIFY(hasSpan(band.valueAt(1.5), 2.0, 3.0));
    QVERIFY(hasSpan(band.valueAt(1.0), 0.0, 3.0));

    band.appendData(1.5, 0.0, 1.0);
    QVERIFY(band.valueAt(1.5).isEmpty());

    band.setData(QList<qreal>{0.0, kNaN, 2.0}, QList<qreal>{0.0, 0.0, 0.0}, QList<qreal>{1.0, 1.0, 1.0});
    QVERIFY(band.valueAt(0.0).isEmpty());
}

void BandSeriesDataTest::containsTestsTheBandUnderTheCursor()
{
    auto fixture = BandFixture{};
    auto& band = fixture.band;
    band.setData(QList<qreal>{0.0, 10.0}, QList<qreal>{2.0, 4.0}, QList<qreal>{6.0, 8.0});

    // At x = 5 the band spans y 3..7, which is pixels 30..70 from the top.
    QVERIFY(band.contains({50.0, 50.0}));
    QVERIFY(band.contains({50.0, 31.0}));
    QVERIFY(band.contains({50.0, 69.0}));
    QVERIFY(!band.contains({50.0, 28.0}));
    QVERIFY(!band.contains({50.0, 72.0}));
    QVERIFY(!band.contains({150.0, 50.0}));

    band.edges()->setWidth(8.0);
    QVERIFY(band.contains({50.0, 28.0}));
    QVERIFY(band.contains({50.0, 72.0}));
    QVERIFY(!band.contains({50.0, 25.0}));

    band.setData(QList<qreal>{10.0, 0.0}, QList<qreal>{2.0, 4.0}, QList<qreal>{6.0, 8.0});
    QVERIFY(!band.contains({50.0, 50.0}));
}

void BandSeriesDataTest::hoveredFollowsChangesUnderARestingCursor()
{
    auto fixture = BandFixture{};
    auto& band = fixture.band;
    band.setData(QList<qreal>{0.0, 10.0}, QList<qreal>{4.0, 4.0}, QList<qreal>{6.0, 6.0});
    const auto hover = [&band](const QEvent::Type type) {
        const auto position = QPointF{50.0, 50.0};
        auto event = QAccelPlotTest::hoverEvent(type, position, position);
        QCoreApplication::sendEvent(&band, &event);
    };
    auto hoveredSpy = QSignalSpy{&band, &BandSeries::hoveredChanged};

    hover(QEvent::HoverEnter);
    QVERIFY(band.hovered());

    // The cursor rests at y = 5 while the band moves away and back.
    band.clearData();
    QVERIFY(!band.hovered());
    band.setData(QList<qreal>{0.0, 10.0}, QList<qreal>{4.0, 4.0}, QList<qreal>{6.0, 6.0});
    QVERIFY(band.hovered());
    fixture.yAxis.setViewportMin(20.0);
    fixture.yAxis.setViewportMax(30.0);
    QVERIFY(!band.hovered());
    fixture.yAxis.setViewportMin(0.0);
    fixture.yAxis.setViewportMax(10.0);
    QVERIFY(band.hovered());

    hover(QEvent::HoverLeave);
    QVERIFY(!band.hovered());
    band.setData(QList<qreal>{0.0, 10.0}, QList<qreal>{0.0, 0.0}, QList<qreal>{10.0, 10.0});
    QVERIFY(!band.hovered());
    QCOMPARE(hoveredSpy.count(), 6);
}

void BandSeriesDataTest::edgeSettingsClampAndNotify()
{
    auto band = BandSeries{};
    auto* edges = band.edges();
    QCOMPARE(edges->width(), 0.0);
    QVERIFY(!edges->color().isValid());
    QVERIFY(edges->lineStyle());

    auto widthSpy = QSignalSpy{edges, &BandEdges::widthChanged};
    edges->setWidth(-3.0);
    QCOMPARE(edges->width(), 0.0);
    QCOMPARE(widthSpy.count(), 0);

    auto styleSpy = QSignalSpy{edges, &BandEdges::lineStyleChanged};
    auto dash = std::make_unique<DashLine>();
    edges->setLineStyle(dash.get());
    QCOMPARE(styleSpy.count(), 1);
    // Editing the style leaves the lineStyle property unchanged.
    dash->setPattern({4.0, 2.0});
    QCOMPARE(styleSpy.count(), 1);
    dash.reset();
    QCOMPARE(styleSpy.count(), 2);
    QVERIFY(!edges->lineStyle());
}

void BandSeriesDataTest::edgeLinesReadTheBandSamples()
{
    auto fixture = BandFixture{};
    auto& band = fixture.band;
    band.setData(QList<qreal>{0.0, 5.0, 10.0}, QList<qreal>{1.0, 2.0, 3.0}, QList<qreal>{4.0, 5.0, 6.0});

    auto* root = fixture.paint();
    QVERIFY(root);
    QCOMPARE(root->childCount(), 1);
    QCOMPARE(fixture.fillMaterial()->sampleCount, 3.0f);
    // Room for 256 samples is reserved, two vertices each.
    QCOMPARE(static_cast<QSGGeometryNode*>(root->firstChild())->geometry()->vertexCount(), 512);

    // The edges sample the fill's data texture instead of uploading their own.
    band.edges()->setWidth(2.0);
    root = fixture.paint();
    QCOMPARE(root->childCount(), 3);
    QVERIFY(fixture.fillMaterial()->dataTexture);
    for (const auto [child, edge] : {std::pair{1, BandEdgeMaterial::Edge::Lower}, std::pair{2, BandEdgeMaterial::Edge::Upper}}) {
        const auto* material = dynamic_cast<const BandEdgeMaterial*>(edgeMaterial(root, child));
        QVERIFY(material);
        QCOMPARE(material->dataTexture, fixture.fillMaterial()->dataTexture);
        QCOMPARE(material->edge(), edge);
        QCOMPARE(material->lineWidth, 2.0f);
    }
    // Without an edge color, edges draw the fill color at full opacity.
    QCOMPARE(edgeMaterial(root, 1)->color, QColor(band.color().red(), band.color().green(), band.color().blue()));

    band.edges()->setWidth(0.0);
    root = fixture.paint();
    QCOMPARE(root->childCount(), 1);
}

void BandSeriesDataTest::dashLengthsFollowZoomButNotPan()
{
    auto fixture = BandFixture{};
    auto dash = DashLine{};
    dash.setPattern({4.0, 2.0});
    fixture.band.edges()->setWidth(1.0);
    fixture.band.edges()->setLineStyle(&dash);
    fixture.band.setData(QList<qreal>{0.0, 5.0, 10.0}, QList<qreal>{1.0, 1.0, 1.0}, QList<qreal>{4.0, 4.0, 4.0});

    // The upper edge is a horizontal line across the 100 px wide item.
    const auto upperEdgeVertices
        = [&fixture]() { return static_cast<LineVertex*>(static_cast<QSGGeometryNode*>(fixture.node->childAtIndex(2))->geometry()->vertexData()); };
    fixture.paint();
    QCOMPARE(upperEdgeVertices()[5].arcLength, 100.0f);

    // Panning keeps the lengths, so the vertex buffer is not rewritten: the marker survives.
    upperEdgeVertices()[0].arcLength = -1.0f;
    fixture.xAxis.setViewportMin(1.0);
    fixture.xAxis.setViewportMax(11.0);
    fixture.paint();
    QCOMPARE(upperEdgeVertices()[0].arcLength, -1.0f);

    // Zooming out halves the pixel lengths.
    fixture.xAxis.setViewportMax(21.0);
    fixture.paint();
    QCOMPARE(upperEdgeVertices()[0].arcLength, 0.0f);
    QCOMPARE(upperEdgeVertices()[5].arcLength, 50.0f);
}

void BandSeriesDataTest::appendingKeepsTheVertexBuffers()
{
    auto fixture = BandFixture{};
    fixture.band.edges()->setWidth(1.0);
    fixture.band.setData(QList<qreal>{0.0, 1.0}, QList<qreal>{1.0, 1.0}, QList<qreal>{2.0, 2.0});
    const auto vertexData = [&fixture](const int child) { return static_cast<QSGGeometryNode*>(fixture.node->childAtIndex(child))->geometry()->vertexData(); };
    fixture.paint();

    // Markers in the fill and edge vertices survive appends within the reserved room.
    static_cast<BandMaterial::Vertex*>(vertexData(0))[0].edge = -1.0f;
    static_cast<LineVertex*>(vertexData(1))[0].side = -2.0f;
    for (auto x = 2; x < 256; ++x) {
        fixture.band.appendData(x, 1.0, 2.0);
        fixture.paint();
    }
    QCOMPARE(fixture.fillMaterial()->sampleCount, 256.0f);
    QCOMPARE(static_cast<BandMaterial::Vertex*>(vertexData(0))[0].edge, -1.0f);
    QCOMPARE(static_cast<LineVertex*>(vertexData(1))[0].side, -2.0f);

    // Crossing the reserved room doubles it and rebuilds the vertices.
    fixture.band.appendData(256.0, 1.0, 2.0);
    fixture.paint();
    QCOMPARE(static_cast<QSGGeometryNode*>(fixture.node->firstChild())->geometry()->vertexCount(), 1024);
    QCOMPARE(static_cast<BandMaterial::Vertex*>(vertexData(0))[0].edge, 0.0f);
}

void BandSeriesDataTest::renderOriginFollowsTheViewport()
{
    constexpr auto epoch = double{1'789'032'600'000.0};
    auto fixture = BandFixture{};
    fixture.xAxis.setViewportMin(epoch);
    fixture.xAxis.setViewportMax(epoch + 10.0);
    fixture.band.setData(QList<qreal>{epoch, epoch + 10.0}, QList<qreal>{0.0, 0.0}, QList<qreal>{1.0, 1.0});

    fixture.paint();
    QCOMPARE(fixture.fillMaterial()->domainMin.x(), 0.0f);
    QCOMPARE(fixture.fillMaterial()->domainMax.x(), 10.0f);

    // Far from the data, the origin moves to the viewport so float coordinates stay precise.
    fixture.xAxis.setViewportMin(epoch + 1.0e6);
    fixture.xAxis.setViewportMax(epoch + 1.0e6 + 10.0);
    fixture.paint();
    QCOMPARE(fixture.fillMaterial()->domainMin.x(), -5.0f);
    QCOMPARE(fixture.fillMaterial()->domainMax.x(), 5.0f);
}

void BandSeriesDataTest::fewerThanTwoSamplesDrawNothing()
{
    auto fixture = BandFixture{};
    fixture.band.setData(QList<qreal>{1.0}, QList<qreal>{0.0}, QList<qreal>{1.0});
    QVERIFY(!fixture.paint());

    fixture.band.appendData(2.0, 0.0, 1.0);
    QVERIFY(fixture.paint());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::BandSeriesDataTest)
#include "tst_band_series_data.moc"
