//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/linestyles/SolidLine.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/LineCurveVertexCache.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QPointer>
#include <QThread>
#include <QtTest/QtTest>

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
    void compatibleLineCacheCanBeReused();
    void incompatibleCacheCannotBeReused();
    void emptyCacheClearsCachedState();
    void invalidCacheIsRejected();
    void invalidRawArgumentsAreRejected();
    void invalidVectorArgumentsAreRejected();
    void rawUpdateHonorsTransition();
    void destroyedTransitionClearsReference();
    void replacedTransitionDestructionDoesNotNotify();
    void stoppedTransitionBeforeFirstFrame_data();
    void stoppedTransitionBeforeFirstFrame();
    void disablingTransitionMidRunCancelsIt();
    void appendDataCancelsRunningTransition();
    void axisAggregatesCurrentSeriesRanges();
    void pointListDataPreservesModernEpochPrecision();
    void separateDoubleDataPreservesModernEpochPrecision();
    void interleavedDoubleDataPreservesModernEpochPrecision();
    void invalidInterleavedDoubleDataIsRejected();
    void postedDoubleDataPreservesModernEpochPrecision();
    void postedDoubleDataFromWorkerThreadIsApplied();
    void invalidPostedDoubleDataIsRejected();
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

std::vector<char> makeLineCache(const std::vector<float>& data, const int pointCount)
{
    auto cache = std::vector<char>{};
    LineCurveLineRenderer{}.buildVertexCache(data, pointCount, cache);
    return cache;
}

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

void LineCurveDataTest::compatibleLineCacheCanBeReused()
{
    auto data = makeData(4);
    const auto expectedCache = makeLineCache(data, 4);
    auto cache = LineCurveVertexCache{};

    QVERIFY(cache.install(std::vector<char>(expectedCache), LineCurveVertexCache::Layout::Line, 4, expectedCache.size()));
    QVERIFY(cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 4));
    QVERIFY(cache.bytes() == expectedCache);
}

void LineCurveDataTest::incompatibleCacheCannotBeReused()
{
    auto data = makeData(6);
    const auto bytes = makeLineCache(data, 6);
    auto cache = LineCurveVertexCache{};
    QVERIFY(cache.install(std::vector<char>(bytes), LineCurveVertexCache::Layout::Line, 6, bytes.size()));

    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 3));
    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Points, 6));

    cache.rebuild(LineCurveVertexCache::Layout::Points, 3, [](std::vector<char>& rebuiltBytes) { rebuiltBytes.assign(12, 'p'); });
    QVERIFY(cache.valid());
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::Points);
    QCOMPARE(cache.pointCount(), 3);
    QCOMPARE(cache.bytes().size(), std::size_t{12});
}

void LineCurveDataTest::emptyCacheClearsCachedState()
{
    auto data = makeData(4);
    auto bytes = makeLineCache(data, 4);
    const auto expectedSize = bytes.size();
    auto cache = LineCurveVertexCache{};
    QVERIFY(cache.install(std::move(bytes), LineCurveVertexCache::Layout::Line, 4, expectedSize));
    QVERIFY(cache.valid());

    QVERIFY(!cache.install({}, LineCurveVertexCache::Layout::Line, 4, expectedSize));

    QVERIFY(!cache.valid());
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::None);
    QCOMPARE(cache.pointCount(), 0);
    QVERIFY(cache.bytes().empty());
}

void LineCurveDataTest::invalidCacheIsRejected()
{
    auto cache = LineCurveVertexCache{};
    auto bytes = std::vector<char>(7);

    QVERIFY(!cache.install(std::move(bytes), LineCurveVertexCache::Layout::Line, 4, 8));
    QVERIFY(!cache.valid());
    QVERIFY(cache.bytes().empty());
}

void LineCurveDataTest::invalidRawArgumentsAreRejected()
{
    auto curve = LineCurve{};
    auto data = makeData(2);
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};
    curve.setDataF(data.data(), 2);
    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received a null data pointer.*"));
    curve.setDataF(nullptr, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.setDataF(data.data(), -1);

    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);
}

void LineCurveDataTest::invalidVectorArgumentsAreRejected()
{
    auto curve = LineCurve{};
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};

    curve.setDataF(makeData(2), 2);
    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataF(makeData(2), 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataFNoRange(makeData(2), 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataFNoRangeWithCache(makeData(2), 3, {});
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.setDataF(makeData(2), -1);

    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);
}

void LineCurveDataTest::rawUpdateHonorsTransition()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    transition.setEnabled(false);
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataFNoRange(first.data(), 2);
    transition.setEnabled(true);

    auto second = makeData(3, 8.0f);
    curve.setDataFNoRange(second.data(), 3);

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
    curve.setMarkerShape(LineCurve::PointShape::Circle);
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
    curve.setDataFNoRange(first.data(), 2);
    QVERIFY(transition.running());

    transition.setEnabled(false);
    auto second = makeData(3, 8.0f);
    curve.setDataFNoRange(second.data(), 3);

    QVERIFY(!transition.running());
}

void LineCurveDataTest::appendDataCancelsRunningTransition()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataFNoRange(first.data(), 2);
    QVERIFY(transition.running());

    curve.appendData(5.0, 6.0);

    QVERIFY(!transition.running());
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

void LineCurveDataTest::invalidInterleavedDoubleDataIsRejected()
{
    auto curve = LineCurve{};
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received 4 doubles for 3 points.*"));
    curve.setData(std::vector<double>{0.0, 0.0, 1.0, 1.0}, 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.setData(std::vector<double>{}, -1);

    QCOMPARE(xRangeSpy.count(), 0);
}

void LineCurveDataTest::postedDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};

    curve.postData(std::vector<double>{epochMilliseconds, 10.0, epochMilliseconds + 0.5, 20.0, epochMilliseconds + 1.0, 30.0}, 3);
    QCOMPARE(xRangeSpy.count(), 0);

    QTRY_COMPARE(xRangeSpy.count(), 1);
    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

void LineCurveDataTest::postedDoubleDataFromWorkerThreadIsApplied()
{
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setYAxis(&yAxis);
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};

    auto* worker = QThread::create([&curve]() { curve.postData(std::vector<double>{0.0, -2.5, 1.0, 7.25}, 2); });
    worker->start();
    QVERIFY(worker->wait());
    delete worker;

    QTRY_COMPARE(yRangeSpy.count(), 1);
    QCOMPARE(yAxis.dataMin(), -2.5);
    QCOMPARE(yAxis.dataMax(), 7.25);
}

void LineCurveDataTest::invalidPostedDoubleDataIsRejected()
{
    auto curve = LineCurve{};
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received 4 doubles for 3 points.*"));
    curve.postData(std::vector<double>{0.0, 0.0, 1.0, 1.0}, 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.postData(std::vector<double>{}, -1);
    QCoreApplication::processEvents();

    QCOMPARE(xRangeSpy.count(), 0);
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

} // namespace QAccelPlot

using QAccelPlot::LineCurveDataTest;
QTEST_MAIN(LineCurveDataTest)
#include "tst_line_curve_data.moc"
