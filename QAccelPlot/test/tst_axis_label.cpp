//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisLabel.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/axis/internal/AxisLabelLayout.hpp"
#include "QAccelPlot/formatters/UnitTickLabelFormatter.hpp"

#include <QImage>
#include <QPainter>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
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

class PolishableAxis final : public Axis {
public:
    using Axis::Axis;
    using Axis::updatePolish;
};

/// \brief Paints \a axis with only its axis label visible, in black on white.
QImage paintLabelOnly(PolishableAxis& axis)
{
    axis.ticker()->setTickColor(Qt::white);
    axis.ticker()->setSubtickColor(Qt::white);
    axis.ticker()->setTickLabelColor(Qt::white);
    axis.setBaselineColor(Qt::white);
    axis.label()->setColor(Qt::black);
    axis.updatePolish();
    auto image = QImage{axis.size().toSize(), QImage::Format_ARGB32};
    image.fill(Qt::white);
    auto painter = QPainter{&image};
    axis.paint(&painter);
    painter.end();
    return image;
}

/// \brief Returns a bottom axis showing 0 to \a viewportMax with the axis label text \a text.
std::unique_ptr<PolishableAxis> labeledAxis(const QString& text, const qreal viewportMax = 0.004)
{
    auto axis = std::make_unique<PolishableAxis>(nullptr, Axis::Bottom);
    axis->setSize(QSizeF{300.0, 60.0});
    axis->setViewportRange(0.0, viewportMax);
    axis->label()->setText(text);
    return axis;
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
    void unitTextFollowsTheViewport();
    void axesSharingAFormatterKeepTheirOwnPrefix();
    void paintedLabelShowsThePrefixedUnit();
    void qmlAxisTakesAUnitFormatter();
    void customUnitFormatPaintsTheTextAndUnit();
    void undefinedRestoresTheDefaultUnitFormat();
    void emptyTextPaintsTheUnitAlone();
    void emptyUnitPaintsThePrefixAlone();
    void withoutUnitTextNothingIsPaintedForEmptyText();
    void delegateBindsToTheUnitText();
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

void AxisLabelTest::unitTextFollowsTheViewport()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto axis = PolishableAxis{nullptr, Axis::Bottom};
    axis.ticker()->setTickLabelFormatter(&formatter);
    auto spy = QSignalSpy{axis.label(), &AxisLabel::unitTextChanged};
    QVERIFY(axis.label()->unitText().isEmpty());

    axis.setViewportRange(0.0, 0.004);
    axis.updatePolish();
    QCOMPARE(axis.label()->unitText(), QStringLiteral("mm"));
    QCOMPARE(axis.label()->property("unitText").toString(), QStringLiteral("mm"));
    QCOMPARE(spy.count(), 1);

    axis.setViewportRange(0.0, 0.003);
    axis.updatePolish();
    QCOMPARE(spy.count(), 1);

    axis.setViewportRange(0.0, 4e-6);
    axis.updatePolish();
    QCOMPARE(axis.label()->unitText(), QStringLiteral("µm"));
    QCOMPARE(spy.count(), 2);

    axis.setViewportRange(0.0, 4000.0);
    axis.updatePolish();
    QCOMPARE(axis.label()->unitText(), QStringLiteral("km"));
    QCOMPARE(spy.count(), 3);

    axis.ticker()->resetTickLabelFormatter();
    axis.updatePolish();
    QVERIFY(axis.label()->unitText().isEmpty());
    QCOMPARE(spy.count(), 4);
}

void AxisLabelTest::axesSharingAFormatterKeepTheirOwnPrefix()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto zoomedIn = PolishableAxis{nullptr, Axis::Bottom};
    auto zoomedOut = PolishableAxis{nullptr, Axis::Left};
    zoomedIn.ticker()->setTickLabelFormatter(&formatter);
    zoomedOut.ticker()->setTickLabelFormatter(&formatter);

    zoomedIn.setViewportRange(0.0, 4e-9);
    zoomedOut.setViewportRange(0.0, 4000.0);
    zoomedIn.updatePolish();
    zoomedOut.updatePolish();

    QCOMPARE(zoomedIn.label()->unitText(), QStringLiteral("nm"));
    QCOMPARE(zoomedOut.label()->unitText(), QStringLiteral("km"));
    QCOMPARE(zoomedIn.formatValue(1.23456e-9, 700.0), QStringLiteral("1.2346 nm"));
    QCOMPARE(zoomedOut.formatValue(1234.56, 700.0), QStringLiteral("1.2346 km"));
}

void AxisLabelTest::paintedLabelShowsThePrefixedUnit()
{
    const auto size = QSizeF{300.0, 60.0};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto withUnit = PolishableAxis{nullptr, Axis::Bottom};
    withUnit.setSize(size);
    withUnit.setViewportRange(0.0, 0.004);
    withUnit.ticker()->setTickLabelFormatter(&formatter);
    withUnit.label()->setText(QStringLiteral("Distance"));
    auto reference = PolishableAxis{nullptr, Axis::Bottom};
    reference.setSize(size);
    reference.setViewportRange(0.0, 0.004);
    reference.label()->setText(QStringLiteral("Distance (mm)"));
    auto textOnly = PolishableAxis{nullptr, Axis::Bottom};
    textOnly.setSize(size);
    textOnly.setViewportRange(0.0, 0.004);
    textOnly.label()->setText(QStringLiteral("Distance"));

    const auto painted = paintLabelOnly(withUnit);

    QVERIFY(inkRows(painted, Qt::white));
    QCOMPARE(painted, paintLabelOnly(reference));
    QVERIFY(painted != paintLabelOnly(textOnly));
}

void AxisLabelTest::qmlAxisTakesAUnitFormatter()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Axis {
            viewportMin: 0; viewportMax: 0.004
            label.text: "Distance"
            ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                unit: "m"
                prefixMode: QAccelPlot.UnitTickLabelFormatter.LargestMagnitude
            }
        }
    )",
        QUrl{});
    auto object = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(object, qPrintable(component.errorString()));
    const auto* axis = qobject_cast<Axis*>(object.get());
    QVERIFY(axis);
    const auto* formatter = qobject_cast<UnitTickLabelFormatter*>(axis->ticker()->tickLabelFormatter());
    QVERIFY(formatter);
    QCOMPARE(formatter->unit(), QStringLiteral("m"));
    QCOMPARE(AxisTickPainter::computeTicks(axis->viewportMin(), axis->viewportMax(), false, axis->ticker()).unitText, QStringLiteral("mm"));
}

void AxisLabelTest::customUnitFormatPaintsTheTextAndUnit()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto withUnit = labeledAxis(QStringLiteral("Distance"));
    withUnit->ticker()->setTickLabelFormatter(&formatter);
    auto spy = QSignalSpy{withUnit->label(), &AxisLabel::unitFormatChanged};
    QCOMPARE(withUnit->label()->unitFormat(), QStringLiteral("%1 (%2)"));

    withUnit->label()->setUnitFormat(QStringLiteral("%1 [%2]"));

    QCOMPARE(withUnit->label()->unitFormat(), QStringLiteral("%1 [%2]"));
    QCOMPARE(spy.count(), 1);
    const auto reference = labeledAxis(QStringLiteral("Distance [mm]"));
    QCOMPARE(paintLabelOnly(*withUnit), paintLabelOnly(*reference));
}

void AxisLabelTest::undefinedRestoresTheDefaultUnitFormat()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Axis {
            label.unitFormat: "%1 / %2"
            function restore() { label.unitFormat = undefined }
        }
    )",
        QUrl{});
    auto object = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(object, qPrintable(component.errorString()));
    auto* const label = qobject_cast<Axis*>(object.get())->label();
    QCOMPARE(label->unitFormat(), QStringLiteral("%1 / %2"));
    auto spy = QSignalSpy{label, &AxisLabel::unitFormatChanged};

    QVERIFY(QMetaObject::invokeMethod(object.get(), "restore"));

    QCOMPARE(label->unitFormat(), QStringLiteral("%1 (%2)"));
    QCOMPARE(spy.count(), 1);
}

void AxisLabelTest::emptyTextPaintsTheUnitAlone()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto unitOnly = labeledAxis(QString{});
    unitOnly->ticker()->setTickLabelFormatter(&formatter);
    unitOnly->label()->setUnitFormat(QStringLiteral("%1 [%2]"));
    const auto reference = labeledAxis(QStringLiteral("mm"));

    const auto painted = paintLabelOnly(*unitOnly);

    QVERIFY(inkRows(painted, Qt::white));
    QCOMPARE(painted, paintLabelOnly(*reference));
}

void AxisLabelTest::emptyUnitPaintsThePrefixAlone()
{
    auto formatter = UnitTickLabelFormatter{};
    auto counts = labeledAxis(QStringLiteral("Count"), 4000.0);
    counts->ticker()->setTickLabelFormatter(&formatter);
    const auto reference = labeledAxis(QStringLiteral("Count (k)"), 4000.0);

    const auto painted = paintLabelOnly(*counts);

    QCOMPARE(counts->label()->unitText(), QStringLiteral("k"));
    QCOMPARE(painted, paintLabelOnly(*reference));
    const auto ticks = AxisTickPainter::computeTicks(0.0, 4000.0, false, counts->ticker());
    auto labels = QStringList{};
    for (const auto& tick : ticks.majorTicks) {
        labels.append(tick.label);
    }
    QVERIFY(labels.contains(QStringLiteral("1")));
    QVERIFY(labels.contains(QStringLiteral("4")));
    QVERIFY(!labels.contains(QStringLiteral("1000")));
}

void AxisLabelTest::withoutUnitTextNothingIsPaintedForEmptyText()
{
    auto axis = labeledAxis(QString{});
    axis->label()->setUnitFormat(QStringLiteral("%1 [%2]"));

    QVERIFY(!inkRows(paintLabelOnly(*axis), Qt::white));
}

void AxisLabelTest::delegateBindsToTheUnitText()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Axis {
            id: axis
            width: 300; height: 60
            viewportMin: 0; viewportMax: 0.004
            ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter { unit: "m" }
            label.delegate: Text { objectName: "title"; text: "Distance in " + axis.label.unitText }
        }
    )",
        QUrl{});
    auto object = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(object, qPrintable(component.errorString()));
    auto* const axis = qobject_cast<Axis*>(object.get());
    auto window = QQuickWindow{};
    axis->setParentItem(window.contentItem());
    window.resize(300, 60);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto* title = axis->findChild<QQuickItem*>(QStringLiteral("title"));
    QVERIFY(title);
    QTRY_COMPARE(title->property("text").toString(), QStringLiteral("Distance in mm"));

    axis->setViewportRange(0.0, 4e-6);

    QTRY_COMPARE(title->property("text").toString(), QStringLiteral("Distance in µm"));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::AxisLabelTest)
#include "tst_axis_label.moc"
