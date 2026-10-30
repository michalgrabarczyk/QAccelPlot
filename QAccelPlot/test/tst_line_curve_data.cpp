//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HoverEvents.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/linestyles/SolidLine.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/LineCurveVertexCache.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QHoverEvent>
#include <QPointer>
#include <QtTest/QtTest>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace QAccelPlot {

class LineCurveDataTest : public QObject {
    Q_OBJECT

public:
    enum class TransitionStopMode { Detach, Cancel, Destroy };
    Q_ENUM(TransitionStopMode)

private slots:
    void hoverEnvironmentControlsAcceptance();
    void cacheUpdateWritesOnlyAddedPoints();
    void cacheUpdateRewritesChangedLayout();
    void cacheIsReusableOnlyForItsLayoutAndCount();
    void emptyCacheUpdateInvalidates();
    void rawUpdateHonorsTransition();
    void destroyedTransitionClearsReference();
    void replacedTransitionDestructionDoesNotNotify();
    void stoppedTransitionBeforeFirstFrame_data();
    void stoppedTransitionBeforeFirstFrame();
    void disablingTransitionMidRunCancelsIt();
    void appendDataCancelsRunningTransition();
    void axisScaleChangeDuringTransitionReportsTargetRange();
    void axisAggregatesCurrentSeriesRanges();
    void pointListDataPreservesModernEpochPrecision();
    void separateDoubleDataPreservesModernEpochPrecision();
    void interleavedDoubleDataPreservesModernEpochPrecision();
    void postedDoubleDataPreservesModernEpochPrecision();
    void hoverEnterAndLeaveToggleHovered();
    void lineAndMarkerCurveContainsLineAndMarkers();
    void hoverRadiusLimitsLineHitTest();
    void hitTestFollowsDataUpdates_data();
    void hitTestFollowsDataUpdates();
    void reassignedEffectsSurviveListClear();
    void destroyedEffectIsRemovedFromList();
    void defaultLineStyleIsDestroyedWithCurve();
    void replacedDefaultLineStyleIsDestroyedWithCurve();
    void assignedLineStyleOutlivesReplacement();
    void destroyedAssignedLineStyleClearsReference();
    void destroyedAssignedLineStyleNotifiesCurve();
    void destroyedReplacedLineStyleDoesNotNotifyCurve();
    void appendedPointsReportSameRangeAsBulkAssignment();
    void appendDataAfterClearRebuildsRangeFromScratch();
    void appendDataIgnoresNonFiniteCoordinates();
    void countTracksStoredPoints();
};

namespace {

std::vector<float> makeData(const int pointCount, const float offset = 0.0f)
{
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; ++i) {
        data[static_cast<std::size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = static_cast<float>(i) + offset;
    }
    return data;
}

class HoverableCurve final : public LineCurve {
public:
    using LineCurve::hoverEnterEvent;
    using LineCurve::hoverLeaveEvent;
};

// Records the first point of every write and fills the written points with \a mark.
struct RecordingWriter {
    std::size_t bytesPerPoint;
    char mark;
    std::vector<int> firstPoints;

    LineCurveVertexCache::Writer writer()
    {
        return [this](std::vector<char>& bytes, const int firstPoint) {
            firstPoints.push_back(firstPoint);
            std::fill(bytes.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(firstPoint) * bytesPerPoint), bytes.end(), mark);
        };
    }
};

} // namespace

void LineCurveDataTest::hoverEnvironmentControlsAcceptance()
{
    constexpr auto variableName = "QACCELPLOT_HOVER_ENABLED";
    const auto wasSet = qEnvironmentVariableIsSet(variableName);
    const auto previousValue = qgetenv(variableName);

    qunsetenv(variableName);
    const auto defaultCurve = LineCurve{};
    qputenv(variableName, "0");
    const auto hoverDisabledCurve = LineCurve{};

    if (wasSet) {
        qputenv(variableName, previousValue);
    } else {
        qunsetenv(variableName);
    }

    QVERIFY(defaultCurve.acceptHoverEvents());
    QVERIFY(!hoverDisabledCurve.acceptHoverEvents());
}

void LineCurveDataTest::cacheUpdateWritesOnlyAddedPoints()
{
    constexpr auto kBytesPerPoint = std::size_t{4};
    auto cache = LineCurveVertexCache{};
    auto first = RecordingWriter{kBytesPerPoint, 'a', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 3, kBytesPerPoint, first.writer());
    QCOMPARE(first.firstPoints, std::vector<int>{0});
    QVERIFY(cache.bytes() == std::vector<char>(12, 'a'));

    auto grown = RecordingWriter{kBytesPerPoint, 'b', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 5, kBytesPerPoint, grown.writer());
    QCOMPARE(grown.firstPoints, std::vector<int>{3});
    auto expected = std::vector<char>(12, 'a');
    expected.resize(20, 'b');
    QVERIFY(cache.bytes() == expected);

    auto same = RecordingWriter{kBytesPerPoint, 'c', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 5, kBytesPerPoint, same.writer());
    QVERIFY(same.firstPoints.empty());

    auto shrunk = RecordingWriter{kBytesPerPoint, 'd', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 2, kBytesPerPoint, shrunk.writer());
    QVERIFY(shrunk.firstPoints.empty());
    QVERIFY(cache.bytes() == std::vector<char>(8, 'a'));
    QVERIFY(cache.valid());
    QCOMPARE(cache.pointCount(), 2);
}

void LineCurveDataTest::cacheUpdateRewritesChangedLayout()
{
    auto cache = LineCurveVertexCache{};
    auto line = RecordingWriter{4, 'a', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 3, 4, line.writer());

    // A different vertex size, such as another step corner count, rewrites every point.
    auto steps = RecordingWriter{8, 'b', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 4, 8, steps.writer());
    QCOMPARE(steps.firstPoints, std::vector<int>{0});
    QVERIFY(cache.bytes() == std::vector<char>(32, 'b'));

    auto points = RecordingWriter{8, 'c', {}};
    cache.update(LineCurveVertexCache::Layout::Points, 4, 8, points.writer());
    QCOMPARE(points.firstPoints, std::vector<int>{0});
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::Points);

    cache.invalidate();
    auto afterInvalidate = RecordingWriter{8, 'd', {}};
    cache.update(LineCurveVertexCache::Layout::Points, 5, 8, afterInvalidate.writer());
    QCOMPARE(afterInvalidate.firstPoints, std::vector<int>{0});
}

void LineCurveDataTest::cacheIsReusableOnlyForItsLayoutAndCount()
{
    auto cache = LineCurveVertexCache{};
    auto writer = RecordingWriter{4, 'a', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 6, 4, writer.writer());

    QVERIFY(cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 6));
    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 3));
    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Points, 6));
    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::None, 6));
}

void LineCurveDataTest::emptyCacheUpdateInvalidates()
{
    auto cache = LineCurveVertexCache{};
    auto writer = RecordingWriter{4, 'a', {}};
    cache.update(LineCurveVertexCache::Layout::Line, 4, 4, writer.writer());
    QVERIFY(cache.valid());

    cache.update(LineCurveVertexCache::Layout::Line, 0, 4, writer.writer());
    QVERIFY(!cache.valid());
    QVERIFY(!cache.data());
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::None);
    QCOMPARE(cache.pointCount(), 0);
    QCOMPARE(writer.firstPoints, std::vector<int>{0});

    cache.update(LineCurveVertexCache::Layout::None, 4, 4, writer.writer());
    QVERIFY(!cache.valid());
    cache.update(LineCurveVertexCache::Layout::Line, 4, 0, writer.writer());
    QVERIFY(!cache.valid());

    cache.clear();
    QVERIFY(cache.bytes().empty());
}

void LineCurveDataTest::rawUpdateHonorsTransition()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    transition.setEnabled(false);
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataF(first.data(), 2);
    transition.setEnabled(true);

    auto second = makeData(3, 8.0f);
    curve.setDataF(second.data(), 3);

    QVERIFY(transition.running());
}

void LineCurveDataTest::destroyedTransitionClearsReference()
{
    auto curve = LineCurve{};
    auto* transition = new MorphTransition;
    curve.setTransition(transition);
    auto changed = QSignalSpy{&curve, &LineCurve::transitionChanged};

    delete transition;

    QCOMPARE(curve.transition(), nullptr);
    QCOMPARE(changed.count(), 1);
    curve.setDataF(makeData(2), 2);
    curve.clearData();
}

void LineCurveDataTest::replacedTransitionDestructionDoesNotNotify()
{
    auto curve = LineCurve{};
    auto* first = new MorphTransition;
    auto second = MorphTransition{};
    curve.setTransition(first);
    curve.setTransition(&second);
    auto changed = QSignalSpy{&curve, &LineCurve::transitionChanged};

    delete first;

    QCOMPARE(curve.transition(), &second);
    QCOMPARE(changed.count(), 0);
}

void LineCurveDataTest::stoppedTransitionBeforeFirstFrame_data()
{
    QTest::addColumn<TransitionStopMode>("stopMode");
    QTest::newRow("detach") << TransitionStopMode::Detach;
    QTest::newRow("cancel") << TransitionStopMode::Cancel;
    QTest::newRow("destroy") << TransitionStopMode::Destroy;
}

void LineCurveDataTest::stoppedTransitionBeforeFirstFrame()
{
    QFETCH(TransitionStopMode, stopMode);
    auto curve = LineCurve{};
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(MarkerShape::Circle);
    auto transition = std::make_unique<MorphTransition>();
    curve.setTransition(transition.get());
    curve.setDataF(makeData(2), 2);

    switch (stopMode) {
    case TransitionStopMode::Detach:
        curve.setTransition(nullptr);
        break;
    case TransitionStopMode::Cancel:
        transition->cancel();
        break;
    case TransitionStopMode::Destroy:
        transition.reset();
        break;
    }

    // Rebuilding marker geometry must never read a pending animation's absent buffer.
    curve.gaps()->setNanMode(NanGapMode::Connect);
    curve.appendData(5.0, 6.0);
    curve.clearData();
}

void LineCurveDataTest::disablingTransitionMidRunCancelsIt()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataF(first.data(), 2);
    QVERIFY(transition.running());

    transition.setEnabled(false);
    auto second = makeData(3, 8.0f);
    curve.setDataF(second.data(), 3);

    QVERIFY(!transition.running());
}

void LineCurveDataTest::appendDataCancelsRunningTransition()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataF(first.data(), 2);
    QVERIFY(transition.running());

    curve.appendData(5.0, 6.0);

    QVERIFY(!transition.running());
}

void LineCurveDataTest::axisScaleChangeDuringTransitionReportsTargetRange()
{
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    auto transition = MorphTransition{};
    curve.setTransition(&transition);

    // Without a window no frame advances the transition, so the curve still shows its start data.
    curve.setData(QList<QPointF>{{10.0, 0.0}, {20.0, 1.0}});
    QVERIFY(transition.running());
    xAxis.setLogScale(true);

    QCOMPARE(xAxis.dataMin(), 10.0);
    QCOMPARE(xAxis.dataMax(), 20.0);
    curve.setTransition(nullptr);
}

void LineCurveDataTest::axisAggregatesCurrentSeriesRanges()
{
    auto axis = Axis{};
    auto first = LineCurve{};
    auto second = LineCurve{};
    first.setXAxis(&axis);
    second.setXAxis(&axis);

    first.setDataF(std::vector<float>{0.0f, 0.0f, 10.0f, 1.0f}, 2);
    second.setDataF(std::vector<float>{20.0f, 0.0f, 30.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 0.0);
    QCOMPARE(axis.dataMax(), 30.0);

    first.setDataF(std::vector<float>{5.0f, 0.0f, 8.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 30.0);

    second.clearData();
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    second.setDataF(std::vector<float>{20.0f, 0.0f, 30.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 30.0);
    second.setXAxis(nullptr);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    {
        auto temporary = LineCurve{};
        temporary.setXAxis(&axis);
        temporary.setDataF(std::vector<float>{-10.0f, 0.0f, -5.0f, 1.0f}, 2);
        QCOMPARE(axis.dataMin(), -10.0);
        QCOMPARE(axis.dataMax(), 8.0);
    }
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    first.clearData();
    QCOMPARE(axis.dataMin(), 0.0);
    QCOMPARE(axis.dataMax(), 1.0);
}

void LineCurveDataTest::pointListDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = qreal{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.setData(QList<QPointF>{
        {epochMilliseconds, 1.0},
        {epochMilliseconds + 1.0, 2.0},
        {epochMilliseconds + 2.0, 3.0},
    });

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 2.0);
}

void LineCurveDataTest::separateDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    const auto xs = std::vector<double>{epochMilliseconds, epochMilliseconds + 0.5, epochMilliseconds + 1.0};
    const auto ys = std::vector<double>{10.0, 20.0, 30.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.setData(xs, ys);

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

void LineCurveDataTest::interleavedDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.setData(std::vector<double>{epochMilliseconds, 10.0, epochMilliseconds + 0.5, 20.0, epochMilliseconds + 1.0, 30.0}, 3);

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

void LineCurveDataTest::postedDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    auto dataSpy = QSignalSpy{&curve, &PlotSeries::dataRevisionChanged};

    curve.postData(std::vector<double>{epochMilliseconds, 10.0, epochMilliseconds + 0.5, 20.0, epochMilliseconds + 1.0, 30.0}, 3);
    QCOMPARE(dataSpy.count(), 0);

    QTRY_COMPARE(dataSpy.count(), 1);
    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

void LineCurveDataTest::hoverEnterAndLeaveToggleHovered()
{
    auto curve = HoverableCurve{};
    auto hoveredSpy = QSignalSpy{&curve, &LineCurve::hoveredChanged};
    const auto position = QPointF{5.0, 5.0};

    auto enter = QAccelPlotTest::hoverEvent(QEvent::HoverEnter, position, position);
    curve.hoverEnterEvent(&enter);
    QVERIFY(curve.hovered());

    auto leave = QAccelPlotTest::hoverEvent(QEvent::HoverLeave, position, position);
    curve.hoverLeaveEvent(&leave);
    QVERIFY(!curve.hovered());
    QCOMPARE(hoveredSpy.count(), 2);
}

void LineCurveDataTest::lineAndMarkerCurveContainsLineAndMarkers()
{
    auto xAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Qt::Vertical);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.marker()->setShape(MarkerShape::Circle);
    curve.setData(QList<QPointF>{{0.1, 0.5}, {0.9, 0.5}});

    const auto& item = static_cast<const QQuickItem&>(curve);
    QVERIFY(item.contains({10, 50}));
    QVERIFY(item.contains({50, 50}));
    QVERIFY(!item.contains({50, 20}));
}

void LineCurveDataTest::hoverRadiusLimitsLineHitTest()
{
    auto xAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Qt::Vertical);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.setData(QList<QPointF>{{0.1, 0.5}, {0.9, 0.5}});
    const auto& item = static_cast<const QQuickItem&>(curve);

    QCOMPARE(curve.hoverRadius(), 10.0);
    QVERIFY(item.contains({50, 42}));
    QVERIFY(!item.contains({50, 38}));

    curve.setHoverRadius(4.0);
    QVERIFY(item.contains({50, 47}));
    QVERIFY(!item.contains({50, 45}));

    curve.setHoverRadius(-1.0);
    QCOMPARE(curve.hoverRadius(), 0.0);
    QVERIFY(!item.contains({50, 50}));
    QVERIFY(!item.contains({10, 50}));

    curve.marker()->setShape(MarkerShape::Circle);
    QVERIFY(item.contains({10, 50}));
    QVERIFY(!item.contains({50, 50}));
}

void LineCurveDataTest::hitTestFollowsDataUpdates_data()
{
    QTest::addColumn<bool>("hoverEnabled");
    QTest::newRow("hover enabled") << true;
    QTest::newRow("hover disabled") << false;
}

void LineCurveDataTest::hitTestFollowsDataUpdates()
{
    QFETCH(bool, hoverEnabled);
    auto xAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Qt::Vertical);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.setHoverEnabled(hoverEnabled);
    const auto& item = static_cast<const QQuickItem&>(curve);

    curve.setDataF(std::vector<float>{0.1f, 0.5f, 0.9f, 0.5f}, 2);
    QVERIFY(item.contains({50, 50}));
    QVERIFY(!item.contains({50, 80}));

    curve.setDataF(std::vector<float>{0.1f, 0.2f, 0.9f, 0.2f}, 2);
    QVERIFY(item.contains({50, 80}));
    QVERIFY(!item.contains({50, 50}));

    curve.setHoverEnabled(!hoverEnabled);
    curve.setDataF(std::vector<float>{0.1f, 0.5f, 0.9f, 0.5f}, 2);
    QVERIFY(item.contains({50, 50}));
    QVERIFY(!item.contains({50, 80}));
}

void LineCurveDataTest::reassignedEffectsSurviveListClear()
{
    auto owner = QObject{};
    auto curve = LineCurve{};
    auto effect = QPointer<GradientFill>{new GradientFill{&owner}};
    auto effects = curve.effects();

    // QML assigns a list property by clearing it and appending each element again.
    effects.append(&effects, effect.data());
    effects.clear(&effects);
    effects.append(&effects, effect.data());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QVERIFY(!effect.isNull());
    QCOMPARE(effect->parent(), &owner);
    QCOMPARE(effects.count(&effects), 1);
    QCOMPARE(effects.at(&effects, 0), effect.data());
}

void LineCurveDataTest::destroyedEffectIsRemovedFromList()
{
    auto curve = LineCurve{};
    auto* effect = new GradientFill{};
    auto effects = curve.effects();
    effects.append(&effects, effect);
    QCOMPARE(effects.count(&effects), 1);

    delete effect;

    QCOMPARE(effects.count(&effects), 0);
}

void LineCurveDataTest::defaultLineStyleIsDestroyedWithCurve()
{
    auto* curve = new LineCurve{};
    auto defaultStyle = QPointer<LineStyle>{curve->lineStyle()};
    QVERIFY(!defaultStyle.isNull());

    delete curve;

    QVERIFY(defaultStyle.isNull());
}

void LineCurveDataTest::replacedDefaultLineStyleIsDestroyedWithCurve()
{
    auto replacement = SolidLine{};
    auto* curve = new LineCurve{};
    auto defaultStyle = QPointer<LineStyle>{curve->lineStyle()};

    curve->setLineStyle(&replacement);
    QVERIFY(!defaultStyle.isNull());
    QCOMPARE(defaultStyle->parent(), curve);

    delete curve;

    QVERIFY(defaultStyle.isNull());
}

void LineCurveDataTest::assignedLineStyleOutlivesReplacement()
{
    auto curve = LineCurve{};
    auto owner = QObject{};
    auto assigned = QPointer<SolidLine>{new SolidLine{&owner}};
    curve.setLineStyle(assigned.data());

    auto other = SolidLine{};
    curve.setLineStyle(&other);

    QVERIFY(!assigned.isNull());
    QCOMPARE(assigned->parent(), &owner);
}

void LineCurveDataTest::destroyedAssignedLineStyleClearsReference()
{
    auto curve = LineCurve{};
    auto* style = new SolidLine{};
    curve.setLineStyle(style);
    QCOMPARE(curve.lineStyle(), style);

    delete style;

    QCOMPARE(curve.lineStyle(), nullptr);
}

void LineCurveDataTest::destroyedAssignedLineStyleNotifiesCurve()
{
    auto curve = LineCurve{};
    auto* style = new SolidLine{};
    curve.setLineStyle(style);
    auto spy = QSignalSpy{&curve, &LineCurve::lineStyleChanged};

    delete style;

    QCOMPARE(spy.count(), 1);
}

void LineCurveDataTest::destroyedReplacedLineStyleDoesNotNotifyCurve()
{
    auto curve = LineCurve{};
    auto* style = new SolidLine{};
    curve.setLineStyle(style);
    auto current = SolidLine{};
    curve.setLineStyle(&current);
    auto spy = QSignalSpy{&curve, &LineCurve::lineStyleChanged};

    delete style;

    QCOMPARE(spy.count(), 0);
    QCOMPARE(curve.lineStyle(), &current);
}

void LineCurveDataTest::appendedPointsReportSameRangeAsBulkAssignment()
{
    // Points deliberately arrive out of order so the appends have to widen the range in
    // both directions, and include an interior point that widens nothing.
    const auto points = QList<QPointF>{{5.0, -1.0}, {2.0, 7.0}, {9.0, 3.0}, {4.0, 0.5}, {-3.0, 11.0}};

    auto bulkXAxis = Axis{};
    auto bulkYAxis = Axis{};
    auto bulkCurve = LineCurve{};
    bulkCurve.setXAxis(&bulkXAxis);
    bulkCurve.setYAxis(&bulkYAxis);
    bulkCurve.setData(points);

    auto appendedXAxis = Axis{};
    auto appendedYAxis = Axis{};
    auto appendedCurve = LineCurve{};
    appendedCurve.setXAxis(&appendedXAxis);
    appendedCurve.setYAxis(&appendedYAxis);
    for (const auto& point : points) {
        appendedCurve.appendData(point.x(), point.y());
    }

    QCOMPARE(appendedXAxis.dataMin(), bulkXAxis.dataMin());
    QCOMPARE(appendedXAxis.dataMax(), bulkXAxis.dataMax());
    QCOMPARE(appendedYAxis.dataMin(), bulkYAxis.dataMin());
    QCOMPARE(appendedYAxis.dataMax(), bulkYAxis.dataMax());
}

void LineCurveDataTest::appendDataAfterClearRebuildsRangeFromScratch()
{
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.appendData(100.0, 1.0);
    curve.appendData(200.0, 2.0);
    QCOMPARE(xAxis.dataMin(), 100.0);
    QCOMPARE(xAxis.dataMax(), 200.0);

    curve.clearData();
    curve.appendData(5.0, 1.0);
    curve.appendData(6.0, 2.0);

    QCOMPARE(xAxis.dataMin(), 5.0);
    QCOMPARE(xAxis.dataMax(), 6.0);
}

void LineCurveDataTest::appendDataIgnoresNonFiniteCoordinates()
{
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.appendData(1.0, 1.0);
    curve.appendData(3.0, 2.0);
    curve.appendData(std::numeric_limits<qreal>::quiet_NaN(), 4.0);

    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
}

void LineCurveDataTest::countTracksStoredPoints()
{
    auto curve = LineCurve{};
    auto countSpy = QSignalSpy{&curve, &PlotSeries::countChanged};

    curve.setDataF(makeData(3), 3);
    QCOMPARE(curve.count(), 3);
    QCOMPARE(curve.property("count").toInt(), 3);
    QCOMPARE(countSpy.count(), 1);

    curve.setDataF(makeData(3, 1.0f), 3);
    QCOMPARE(countSpy.count(), 1);

    curve.appendData(std::numeric_limits<qreal>::quiet_NaN(), 1.0);
    QCOMPARE(curve.count(), 4);
    QCOMPARE(countSpy.count(), 2);

    curve.clearData();
    curve.clearData();
    QCOMPARE(curve.count(), 0);
    QCOMPARE(countSpy.count(), 3);
}

} // namespace QAccelPlot

using QAccelPlot::LineCurveDataTest;
QTEST_MAIN(LineCurveDataTest)
#include "tst_line_curve_data.moc"
