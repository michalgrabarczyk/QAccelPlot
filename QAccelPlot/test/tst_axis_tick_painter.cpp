//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "AxisTickPainter.hpp"
#include "TickLabelFormatter.hpp"

#include <QImage>
#include <QPainter>
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

class TestAxisTickPainter : public QObject {
    Q_OBJECT

private slots:
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
};

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
                params.viewportMin = 0.0;
                params.viewportMax = 1.0;
                params.orientation = QAccelPlot::Axis::Horizontal;
                params.side = QAccelPlot::Axis::Bottom;
                params.ticker = &ticker;
                QAccelPlot::AxisTickPainter::paintTicks(&painter, QRectF{kFirstTickX, 0.0, kTickSpacing, kImageHeight}, 0.0, kAxisY, params,
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
        params.viewportMin = 0.25;
        params.viewportMax = 0.75;
        params.orientation = QAccelPlot::Axis::Vertical;
        params.side = QAccelPlot::Axis::Left;
        params.ticker = &ticker;
        QAccelPlot::AxisTickPainter::paintTicks(&painter, actual.rect(), kAxisX, 0.0, params,
            [](qreal, qreal) { return qreal{40.0}; });
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

QTEST_MAIN(TestAxisTickPainter)
#include "tst_axis_tick_painter.moc"
