//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisLabel.hpp"
#include "QAccelPlot/axis/internal/AxisLabelLayout.hpp"

#include <QImage>
#include <QPainter>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <cmath>
#include <memory>
#include <optional>

namespace QAccelPlot {

using Internal::axisLabelCenter;
using Internal::AxisLabelLayoutParams;
using Internal::rotatedBoundingSize;

namespace {

constexpr auto kTolerance = 1e-9;

bool near(const qreal actual, const qreal expected)
{
    return std::abs(actual - expected) <= kTolerance;
}

AxisLabelLayoutParams leftAxisParams()
{
    auto params = AxisLabelLayoutParams{};
    params.dataRect = QRectF{0.0, 10.0, 50.0, 200.0};
    params.baseline = 48.0;
    params.outward = Qt::LeftEdge;
    params.textSize = QSizeF{60.0, 14.0};
    params.rotation = -90.0;
    params.padding = 30.0;
    return params;
}

/// \brief Returns the vertical extent of the pixels in \a image that differ from \a background.
std::optional<std::pair<int, int>> inkRows(const QImage& image, const QColor& background)
{
    auto first = std::optional<int>{};
    auto last = 0;
    for (auto y = 0; y < image.height(); ++y) {
        for (auto x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != background) {
                if (!first) {
                    first = y;
                }
                last = y;
                break;
            }
        }
    }
    if (!first) {
        return std::nullopt;
    }
    return std::make_pair(*first, last);
}

} // namespace

class AxisLabelTest : public QObject {
    Q_OBJECT

private slots:
    void rotatedBoxSwapsSidesAtRightAngles();
    void centerMatchesTheDefaultLayout();
    void startAndEndAreFlushWithTheAxisEnds();
    void horizontalAxisAlignsLeftToRight();
    void paddingClearsTheRotatedBox();
    void offsetIsAddedLast();
    void rotationFollowsTheSideUntilSet();
    void resetRestoresTheSideRotation();
    void paintedLabelFollowsAlignment();
    void delegateReplacesThePaintedLabel();
};

void AxisLabelTest::rotatedBoxSwapsSidesAtRightAngles()
{
    const auto size = QSizeF{60.0, 14.0};
    const auto upright = rotatedBoundingSize(size, -90.0);
    QVERIFY(near(upright.width(), 14.0));
    QVERIFY(near(upright.height(), 60.0));
    const auto diagonal = rotatedBoundingSize(size, 45.0);
    QVERIFY(near(diagonal.width(), 74.0 * std::sqrt(0.5)));
    QVERIFY(near(diagonal.height(), 74.0 * std::sqrt(0.5)));
}

void AxisLabelTest::centerMatchesTheDefaultLayout()
{
    const auto center = axisLabelCenter(leftAxisParams());
    QVERIFY(near(center.x(), 48.0 - 30.0 - 7.0));
    QVERIFY(near(center.y(), 110.0));
}

void AxisLabelTest::startAndEndAreFlushWithTheAxisEnds()
{
    auto params = leftAxisParams();
    params.alongFraction = 0.0;
    QVERIFY(near(axisLabelCenter(params).y(), 210.0 - 30.0));
    params.alongFraction = 1.0;
    QVERIFY(near(axisLabelCenter(params).y(), 10.0 + 30.0));
}

void AxisLabelTest::horizontalAxisAlignsLeftToRight()
{
    auto params = AxisLabelLayoutParams{};
    params.dataRect = QRectF{25.0, 0.0, 300.0, 50.0};
    params.baseline = 2.0;
    params.outward = Qt::BottomEdge;
    params.textSize = QSizeF{40.0, 14.0};
    params.padding = 30.0;
    params.alongFraction = 0.0;
    QVERIFY(near(axisLabelCenter(params).x(), 45.0));
    QVERIFY(near(axisLabelCenter(params).y(), 2.0 + 30.0 + 7.0));
    params.alongFraction = 1.0;
    QVERIFY(near(axisLabelCenter(params).x(), 305.0));
    params.outward = Qt::TopEdge;
    QVERIFY(near(axisLabelCenter(params).y(), 2.0 - 30.0 - 7.0));
}

void AxisLabelTest::paddingClearsTheRotatedBox()
{
    auto params = leftAxisParams();
    params.rotation = 0.0;
    params.alongFraction = 1.0;
    const auto center = axisLabelCenter(params);
    QVERIFY(near(center.x() + 30.0, 48.0 - 30.0));
    QVERIFY(near(center.y(), 10.0 + 7.0));
}

void AxisLabelTest::offsetIsAddedLast()
{
    auto params = leftAxisParams();
    const auto base = axisLabelCenter(params);
    params.offset = QPointF{3.0, -4.0};
    QCOMPARE(axisLabelCenter(params), base + QPointF(3.0, -4.0));
}

void AxisLabelTest::rotationFollowsTheSideUntilSet()
{
    auto axis = Axis{nullptr, Axis::Bottom};
    QCOMPARE(axis.label()->rotation(), 0.0);
    auto spy = QSignalSpy{axis.label(), &AxisLabel::rotationChanged};
    axis.setSide(Axis::Left);
    QCOMPARE(axis.label()->rotation(), -90.0);
    QCOMPARE(spy.count(), 1);
    axis.setSide(Axis::Right);
    QCOMPARE(axis.label()->rotation(), 90.0);
    QCOMPARE(spy.count(), 2);
    axis.label()->setRotation(0.0);
    QCOMPARE(spy.count(), 3);
    axis.setSide(Axis::Left);
    QCOMPARE(axis.label()->rotation(), 0.0);
    QCOMPARE(spy.count(), 3);
}

void AxisLabelTest::resetRestoresTheSideRotation()
{
    auto axis = Axis{nullptr, Axis::Left};
    axis.label()->setRotation(30.0);
    auto spy = QSignalSpy{axis.label(), &AxisLabel::rotationChanged};
    QVERIFY(axis.label()->metaObject()->property(axis.label()->metaObject()->indexOfProperty("rotation")).reset(axis.label()));
    QCOMPARE(axis.label()->rotation(), -90.0);
    QCOMPARE(spy.count(), 1);
    axis.label()->setRotation(std::nan(""));
    QCOMPARE(spy.count(), 1);
}

void AxisLabelTest::paintedLabelFollowsAlignment()
{
    const auto paintedRows = [](const Axis::LabelAlignment alignment) {
        auto axis = Axis{nullptr, Axis::Left};
        axis.setSize(QSizeF{50.0, 300.0});
        axis.label()->setText(QStringLiteral("Voltage"));
        axis.label()->setColor(Qt::black);
        axis.setBaselineColor(Qt::white);
        axis.label()->setAlignment(alignment);
        auto image = QImage{50, 300, QImage::Format_ARGB32};
        image.fill(Qt::white);
        auto painter = QPainter{&image};
        axis.paint(&painter);
        painter.end();
        return inkRows(image, Qt::white);
    };
    const auto start = paintedRows(Axis::Start);
    const auto end = paintedRows(Axis::End);
    QVERIFY(start && end);
    QVERIFY(start->first > 150);
    QVERIFY(end->second < 150);
}

void AxisLabelTest::delegateReplacesThePaintedLabel()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Axis {
            id: axis
            width: 50; height: 300
            side: QAccelPlot.Axis.Left
            label.text: "Voltage"
            label.delegate: first
            property Component first: Text { objectName: "first"; text: axis.label.text; anchors.top: parent.top }
            property Component second: Rectangle { objectName: "second"; x: 4; y: 8 }
        }
    )",
        QUrl{});
    auto axis = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(axis, qPrintable(component.errorString()));
    const auto* axisItem = qobject_cast<QQuickItem*>(axis.get());
    auto* label = axis->property("label").value<AxisLabel*>();
    QVERIFY(label);

    const auto labelItem = [axisItem](const char* name) { return axisItem->findChild<QQuickItem*>(QString::fromLatin1(name)); };
    const auto first = QPointer<QQuickItem>{labelItem("first")};
    QVERIFY(first);
    QCOMPARE(first->parentItem(), axisItem);
    QCOMPARE(first->property("text").toString(), QStringLiteral("Voltage"));
    QCOMPARE(first->y(), 0.0);

    QVERIFY(label->setProperty("delegate", axis->property("second")));
    QVERIFY(!first->isVisible());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(first.isNull());
    const auto second = QPointer<QQuickItem>{labelItem("second")};
    QVERIFY(second);
    QCOMPARE(second->parentItem(), axisItem);
    QCOMPARE(second->position(), QPointF(4.0, 8.0));

    QVERIFY(label->setProperty("delegate", QVariant::fromValue<QQmlComponent*>(nullptr)));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(second.isNull());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::AxisLabelTest)
#include "tst_axis_label.moc"
