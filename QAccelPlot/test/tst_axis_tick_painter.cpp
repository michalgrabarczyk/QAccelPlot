//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QImage>
#include <QJSEngine>
#include <QMutex>
#include <QPainter>
#include <QQuickWindow>
#include <QThread>
#include <QtTest/QtTest>

#include <utility>

class FixedTickLabelFormatter final : public QAccelPlot::TickLabelFormatter {
public:
    explicit FixedTickLabelFormatter(QString label)
        : label_(std::move(label))
    {
    }

protected:
    QString doFormat(qreal, qreal) const override
    {
        return label_;
    }

private:
    QString label_;
};

class RecordingTickLabelFormatter final : public QAccelPlot::TickLabelFormatter {
public:
    mutable QList<QPair<qreal, qreal>> calls;

protected:
    QString doFormat(const qreal value, const qreal tickStep) const override
    {
        calls.append({value, tickStep});
        return QString::number(value);
    }
};

class ThreadRecordingTickLabelFormatter final : public QAccelPlot::TickLabelFormatter {
public:
    QList<QThread*> formatThreads() const
    {
        const auto locker = QMutexLocker{&mutex_};
        return formatThreads_;
    }

protected:
    QString doFormat(qreal value, qreal) const override
    {
        const auto locker = QMutexLocker{&mutex_};
        formatThreads_.append(QThread::currentThread());
        return QString::number(value);
    }

private:
    mutable QMutex mutex_;
    mutable QList<QThread*> formatThreads_;
};

class PaintThreadRecordingAxis final : public QAccelPlot::Axis {
public:
    using QAccelPlot::Axis::Axis;

    QThread* paintThread() const
    {
        const auto locker = QMutexLocker{&mutex_};
        return paintThread_;
    }

    void paint(QPainter* painter) override
    {
        {
            const auto locker = QMutexLocker{&mutex_};
            paintThread_ = QThread::currentThread();
        }
        QAccelPlot::Axis::paint(painter);
    }

private:
    mutable QMutex mutex_;
    QThread* paintThread_{nullptr};
};

class TestAxisTickPainter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void computeNiceStep_roundRange();
    void computeNiceStep_subUnitRange();
    void computeNiceStep_negativeToPositive();
    void computeNiceStep_nonRoundRange();
    void computeNiceStep_zeroRange();
    void computeNiceStep_zeroTickCount();
    void computeNiceStep_singleTick();

    // Corner cases
    void computeNiceStep_veryLargeRange();
    void computeNiceStep_verySmallFractionalRange();
    void computeNiceStep_negativeOnlyRange();
    void computeNiceStep_residualAtTwoBoundary();
    void paintTicks_timeLabelsMatchUnclippedReference();
    void paintTicks_verticalLabelsUseAvailableWidth();

    void computeTicks_linearTicksAndLabels();
    void computeTicks_passesTickStepToFormatter();
    void computeTicks_logScaleTicksAndSubticks();
    void computeTicks_logScaleMajorTicksReceiveNonZeroStep();
    void computeTicks_nullTickerIsEmpty();
    void axis_formatsTickLabelsOnGuiThread();
};

void TestAxisTickPainter::initTestCase()
{
    // Qt falls back to the basic render loop for some drivers, e.g. Mesa llvmpipe under Xvfb in Linux CI,
    // where Axis::paint() would run on the GUI thread. Force the threaded loop so the cross-thread case is
    // exercised; it must be set before the first QQuickWindow creates the process-wide render loop.
    if (!qEnvironmentVariableIsSet("QSG_RENDER_LOOP")) {
        qputenv("QSG_RENDER_LOOP", "threaded");
    }
}

void TestAxisTickPainter::computeNiceStep_roundRange()
{
    // [0, 100] / 10 → roughStep=10, residual=1 → step=10
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 100.0, 10);
    QCOMPARE(step, 10.0);
}

void TestAxisTickPainter::computeNiceStep_subUnitRange()
{
    // [0, 1] / 5 → roughStep=0.2, magnitude=0.1, residual=2 → step=0.2
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 1.0, 5);
    QCOMPARE(step, 0.2);
}

void TestAxisTickPainter::computeNiceStep_negativeToPositive()
{
    // [-50, 50] / 10 → roughStep=10 → step=10
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(-50.0, 50.0, 10);
    QCOMPARE(step, 10.0);
}

void TestAxisTickPainter::computeNiceStep_nonRoundRange()
{
    // [0, 73] / 10 → roughStep=7.3, magnitude=1, residual=7.3 → niceResidual=10 → step=10
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 73.0, 10);
    QCOMPARE(step, 10.0);
}

void TestAxisTickPainter::computeNiceStep_zeroRange()
{
    // range <= 0 → returns 1.0 (guard)
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(5.0, 5.0, 10);
    QCOMPARE(step, 1.0);
}

void TestAxisTickPainter::computeNiceStep_zeroTickCount()
{
    // tickCount == 0 → returns 1.0 (guard)
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 100.0, 0);
    QCOMPARE(step, 1.0);
}

void TestAxisTickPainter::computeNiceStep_singleTick()
{
    // [0, 50] / 1 → roughStep=50, magnitude=10, residual=5 → step=50
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 50.0, 1);
    QCOMPARE(step, 50.0);
}

void TestAxisTickPainter::computeNiceStep_veryLargeRange()
{
    // [0, 1e12] / 10 → roughStep=1e11, magnitude=1e11, residual=1 → step=1e11
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 1e12, 10);
    QCOMPARE(step, 1e11);
}

void TestAxisTickPainter::computeNiceStep_verySmallFractionalRange()
{
    // [1.0, 1.001] / 5 → roughStep=0.0002, magnitude=1e-4, residual=2 → step=0.0002
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(1.0, 1.001, 5);
    QCOMPARE(step, 0.0002);
}

void TestAxisTickPainter::computeNiceStep_negativeOnlyRange()
{
    // [-100, -10] → range=90, roughStep=9, magnitude=1, residual=9 → niceResidual=10 → step=10
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(-100.0, -10.0, 10);
    QCOMPARE(step, 10.0);
}

void TestAxisTickPainter::computeNiceStep_residualAtTwoBoundary()
{
    // [0, 20] / 10 → roughStep=2, magnitude=1, residual=2 → niceResidual=2 → step=2
    const auto step = QAccelPlot::AxisTickPainter::computeNiceStep(0.0, 20.0, 10);
    QCOMPARE(step, 2.0);
}

void TestAxisTickPainter::paintTicks_timeLabelsMatchUnclippedReference()
{
    constexpr auto kImageWidth = 300;
    constexpr auto kImageHeight = 80;
    constexpr auto kAxisY = qreal{20.0};
    constexpr auto kLabelCenterY = qreal{30.0};
    constexpr auto kFirstTickX = qreal{75.0};
    constexpr auto kTickSpacing = qreal{150.0};
    constexpr auto kReferenceLabelWidth = qreal{140.0};
    constexpr auto kReferenceLabelHeight = qreal{20.0};

    auto font = QFont{};
    font.setPixelSize(12);
    const auto labels = QStringList{QStringLiteral("09:31:00"), QStringLiteral("09:31:00.000")};
    const auto rotations = QList<qreal>{0.0, 30.0};

    for (const auto& label : labels) {
        for (const auto rotation : rotations) {
            auto formatter = FixedTickLabelFormatter{label};
            auto ticker = QAccelPlot::AxisTicker{};
            ticker.setTickCount(1);
            ticker.setSubtickCount(0);
            ticker.setTickLength(0.0);
            ticker.setTickLabelPadding(0.0);
            ticker.setTickLabelRotation(rotation);
            ticker.setTickLabelFont(font);
            ticker.setTickColor(Qt::transparent);
            ticker.setTickLabelColor(Qt::white);
            ticker.setTickLabelFormatter(&formatter);

            auto actual = QImage{kImageWidth, kImageHeight, QImage::Format_ARGB32_Premultiplied};
            actual.fill(Qt::transparent);
            {
                auto painter = QPainter{&actual};
                painter.setPen(Qt::transparent);
                auto params = QAccelPlot::AxisTickPainter::Params{};
                params.orientation = QAccelPlot::Axis::Horizontal;
                params.side = QAccelPlot::Axis::Bottom;
                params.ticker = &ticker;
                const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(0.0, 1.0, false, &ticker);
                QAccelPlot::AxisTickPainter::paintTicks(&painter, QRectF{kFirstTickX, 0.0, kTickSpacing, kImageHeight}, 0.0, kAxisY, params, ticks,
                    [](const qreal value, const qreal length) { return value * length; });
            }

            auto expected = QImage{kImageWidth, kImageHeight, QImage::Format_ARGB32_Premultiplied};
            expected.fill(Qt::transparent);
            {
                auto painter = QPainter{&expected};
                painter.setFont(font);
                painter.setPen(Qt::white);
                const auto drawReferenceLabel = [&painter, &label, rotation](const qreal centerX) {
                    painter.save();
                    painter.translate(centerX, kLabelCenterY);
                    painter.rotate(rotation);
                    painter.drawText(
                        QRectF{-kReferenceLabelWidth / 2.0, -kReferenceLabelHeight / 2.0, kReferenceLabelWidth, kReferenceLabelHeight}, Qt::AlignCenter, label);
                    painter.restore();
                };
                drawReferenceLabel(kFirstTickX);
                drawReferenceLabel(kFirstTickX + kTickSpacing);
            }

            QVERIFY2(actual == expected, qPrintable(QStringLiteral("Label '%1' at %2 degrees was clipped").arg(label).arg(rotation)));
        }
    }
}

void TestAxisTickPainter::paintTicks_verticalLabelsUseAvailableWidth()
{
    constexpr auto kImageWidth = 120;
    constexpr auto kImageHeight = 80;
    constexpr auto kAxisX = qreal{110.0};
    constexpr auto kOldLabelLeft = 70;

    auto font = QFont{};
    font.setPixelSize(12);
    const auto label = QStringLiteral("09:31:00.000");
    auto formatter = FixedTickLabelFormatter{label};
    auto ticker = QAccelPlot::AxisTicker{};
    ticker.setTickCount(1);
    ticker.setSubtickCount(0);
    ticker.setTickLength(0.0);
    ticker.setTickLabelPadding(0.0);
    ticker.setTickLabelFont(font);
    ticker.setTickColor(Qt::transparent);
    ticker.setTickLabelColor(Qt::white);
    ticker.setTickLabelFormatter(&formatter);

    auto actual = QImage{kImageWidth, kImageHeight, QImage::Format_ARGB32_Premultiplied};
    actual.fill(Qt::transparent);
    {
        auto painter = QPainter{&actual};
        painter.setPen(Qt::transparent);
        auto params = QAccelPlot::AxisTickPainter::Params{};
        params.orientation = QAccelPlot::Axis::Vertical;
        params.side = QAccelPlot::Axis::Left;
        params.ticker = &ticker;
        const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(0.25, 0.75, false, &ticker);
        QAccelPlot::AxisTickPainter::paintTicks(&painter, actual.rect(), kAxisX, 0.0, params, ticks, [](qreal, qreal) { return qreal{40.0}; });
    }

    auto foundPixelOutsideOldLabelRect = false;
    for (auto y = 0; y < actual.height() && !foundPixelOutsideOldLabelRect; ++y) {
        for (auto x = 0; x < kOldLabelLeft; ++x) {
            if (actual.pixelColor(x, y).alpha() != 0) {
                foundPixelOutsideOldLabelRect = true;
                break;
            }
        }
    }

    QVERIFY2(foundPixelOutsideOldLabelRect, "Vertical tick label did not use the available axis width");
}

void TestAxisTickPainter::computeTicks_linearTicksAndLabels()
{
    auto ticker = QAccelPlot::AxisTicker{};
    ticker.setTickCount(5);
    ticker.setSubtickCount(1);

    const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(0.0, 1.0, false, &ticker);

    // [0, 1] / 5 -> step=0.2 -> ticks at 0, 0.2, ..., 1.0 with one subtick between each pair.
    QCOMPARE(ticks.majorTicks.size(), 6);
    QCOMPARE(ticks.subtickValues.size(), 5);
    QCOMPARE(ticks.majorTicks.first().value, 0.0);
    QVERIFY(qFuzzyCompare(ticks.majorTicks.last().value, 1.0));
    QVERIFY(qFuzzyCompare(ticks.subtickValues.first(), 0.1));
    for (const auto& tick : ticks.majorTicks) {
        QCOMPARE(tick.label, ticker.tickLabelFormatter()->format(tick.value, 0.2));
    }
}

void TestAxisTickPainter::computeTicks_passesTickStepToFormatter()
{
    auto engine = QJSEngine{};
    auto formatter = FixedTickLabelFormatter{QString{}};
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value, tickStep) { return 'step=' + tickStep; })")));
    auto ticker = QAccelPlot::AxisTicker{};
    ticker.setTickCount(10);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(0.0, 100.0, false, &ticker);

    QVERIFY(!ticks.majorTicks.isEmpty());
    for (const auto& tick : ticks.majorTicks) {
        QCOMPARE(tick.label, QStringLiteral("step=10"));
    }
}

void TestAxisTickPainter::computeTicks_logScaleTicksAndSubticks()
{
    auto ticker = QAccelPlot::AxisTicker{};

    const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(1.0, 100.0, true, &ticker);

    // Decades 1, 10 and 100 are major ticks; multiples 2..9 of 1 and 10 are subticks.
    QCOMPARE(ticks.majorTicks.size(), 3);
    QCOMPARE(ticks.majorTicks.at(0).value, 1.0);
    QCOMPARE(ticks.majorTicks.at(1).value, 10.0);
    QCOMPARE(ticks.majorTicks.at(2).value, 100.0);
    QCOMPARE(ticks.subtickValues.size(), 16);
}

void TestAxisTickPainter::computeTicks_logScaleMajorTicksReceiveNonZeroStep()
{
    auto formatter = RecordingTickLabelFormatter{};
    auto ticker = QAccelPlot::AxisTicker{};
    ticker.setTickLabelFormatter(&formatter);

    QAccelPlot::AxisTickPainter::computeTicks(0.001, 100.0, true, &ticker);

    QVERIFY(!formatter.calls.isEmpty());
    for (const auto& call : formatter.calls) {
        QVERIFY2(call.second > 0.0, qPrintable(QStringLiteral("major tick %1 was formatted with tickStep %2").arg(call.first).arg(call.second)));
    }
}

void TestAxisTickPainter::computeTicks_nullTickerIsEmpty()
{
    const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(0.0, 1.0, false, nullptr);

    QVERIFY(ticks.majorTicks.isEmpty());
    QVERIFY(ticks.subtickValues.isEmpty());
}

void TestAxisTickPainter::axis_formatsTickLabelsOnGuiThread()
{
    // Axis::paint() runs on the render thread with the threaded render loop. Tick labels must still be
    // formatted on the GUI thread, since a formatter's tickLabel JS callback is bound to the QML engine's thread.
    auto window = QQuickWindow{};
    window.resize(300, 60);
    auto formatter = ThreadRecordingTickLabelFormatter{};
    auto axis = PaintThreadRecordingAxis{window.contentItem(), QAccelPlot::Axis::Bottom};
    axis.setSize(QSizeF{300.0, 60.0});
    axis.ticker()->setTickLabelFormatter(&formatter);

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.grabWindow();

    QVERIFY(axis.paintThread() != nullptr);
    if (axis.paintThread() == QThread::currentThread()) {
        QSKIP("Axis::paint() ran on the GUI thread (no threaded render loop available), so the cross-thread case is not exercised");
    }

    const auto formatThreads = formatter.formatThreads();
    QVERIFY(!formatThreads.isEmpty());
    for (const auto* thread : formatThreads) {
        QCOMPARE(thread, QThread::currentThread());
    }
}

QTEST_MAIN(TestAxisTickPainter)
#include "tst_axis_tick_painter.moc"
