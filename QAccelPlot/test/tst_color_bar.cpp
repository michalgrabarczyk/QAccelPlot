//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/axis/ColorBar.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QFontMetricsF>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QImage>
#include <QMouseEvent>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QWheelEvent>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

// Column (vertical bar) or row (horizontal bar) through the middle of the default 12 px strip.
constexpr auto kStripCenter = 6;
// Column right of a vertical strip that major ticks (4 px) cross and subticks (2 px) do not.
constexpr auto kMajorTickColumn = 15;
constexpr auto kGrayTolerance = 8;

constexpr auto kWheelStep = 120;
constexpr auto kTolerance = 1e-9;

class TestableColorBar final : public ColorBar {
public:
    using ColorBar::ColorBar;
    using ColorBar::hoverEnterEvent;
    using ColorBar::hoverLeaveEvent;
    using ColorBar::mouseDoubleClickEvent;
    using ColorBar::mouseMoveEvent;
    using ColorBar::mousePressEvent;
    using ColorBar::mouseReleaseEvent;
    using ColorBar::wheelEvent;
};

QMouseEvent mouseEvent(const QEvent::Type type, const QPointF& pos, const Qt::MouseButton button, const Qt::MouseButtons buttons)
{
    return QMouseEvent{type, pos, pos, pos, button, buttons, Qt::NoModifier};
}

void drag(TestableColorBar& bar, const QPointF& from, const QPointF& to, const Qt::MouseButton button = Qt::LeftButton)
{
    auto press = mouseEvent(QEvent::MouseButtonPress, from, button, button);
    bar.mousePressEvent(&press);
    auto move = mouseEvent(QEvent::MouseMove, to, Qt::NoButton, button);
    bar.mouseMoveEvent(&move);
    auto release = mouseEvent(QEvent::MouseButtonRelease, to, button, Qt::NoButton);
    bar.mouseReleaseEvent(&release);
}

void doubleClick(TestableColorBar& bar, const QPointF& pos)
{
    auto event = mouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton);
    bar.mouseDoubleClickEvent(&event);
}

/// Sends one wheel step to \a bar and returns whether the bar accepted it.
bool wheel(TestableColorBar& bar, const QPointF& pos, const int deltaY)
{
    auto event = QWheelEvent{pos, pos, QPoint{}, QPoint{0, deltaY}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false};
    bar.wheelEvent(&event);
    return event.isAccepted();
}

bool fuzzyEqual(const qreal actual, const qreal expected)
{
    return std::abs(actual - expected) <= kTolerance * std::max(1.0, std::abs(expected));
}

/// A value-colored point cloud keyed by a color bar that fills a black window.
struct Scene {
    QQuickWindow window;
    Colormap colormap;
    PointCloud cloud;
    TestableColorBar bar{window.contentItem()};

    Scene(const ColorBar::Orientation orientation, const QSize& size)
    {
        window.setColor(Qt::black);
        window.resize(size);
        colormap.setPreset(Colormap::Preset::Grayscale);
        colormap.setMin(0.0);
        colormap.setMax(1.0);
        cloud.setColormap(&colormap);
        cloud.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f}, std::vector<float>{0.0f, 1.0f}, 2);
        bar.setSeries(&cloud);
        bar.setOrientation(orientation);
        bar.setSize(QSizeF{size});
    }

    /// Shows the window on first use, then polishes and renders it.
    QImage render()
    {
        if (!window.isVisible()) {
            window.show();
            if (!QTest::qWaitForWindowExposed(&window)) {
                return {};
            }
        }
        return window.grabWindow();
    }

    /// Length in pixels of the strip along the ramp, which is centered in the bar.
    qreal stripLength() const
    {
        if (bar.orientation() == ColorBar::Horizontal) {
            return bar.width() - 1.0;
        }
        // Half a tick label is kept free at each end.
        return bar.height() - AxisTickPainter::tickLabelSize(QFontMetricsF{bar.ticker()->tickLabelFont()}, QStringLiteral("0")).height();
    }
};

/// An interactive scene that has been rendered once, so the bar is laid out.
struct InteractiveScene : Scene {
    explicit InteractiveScene(const ColorBar::Orientation orientation)
        : Scene{orientation, orientation == ColorBar::Vertical ? QSize{100, 200} : QSize{201, 60}}
    {
        bar.setInteractive(true);
        render();
    }

    QPointF center() const
    {
        return {bar.width() / 2.0, bar.height() / 2.0};
    }
};

/// Returns the number of separate runs of \a color pixels in column \a x of \a image.
int runsInColumn(const QImage& image, const int x, const QColor& color)
{
    auto runs = 0;
    auto inRun = false;
    for (auto y = 0; y < image.height(); ++y) {
        const auto matches = image.pixelColor(x, y) == color;
        if (matches && !inRun) {
            ++runs;
        }
        inRun = matches;
    }
    return runs;
}

/// Returns \c true when any pixel with x at or beyond \a firstColumn is not black.
bool hasContentFrom(const QImage& image, const int firstColumn)
{
    for (auto y = 0; y < image.height(); ++y) {
        for (auto x = firstColumn; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != QColor{Qt::black}) {
                return true;
            }
        }
    }
    return false;
}

bool isUniform(const QImage& image, const QColor& color)
{
    for (auto y = 0; y < image.height(); ++y) {
        for (auto x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != color) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

#define QCOMPARE_NEAR(actual, expected)                                                                                                                        \
    QVERIFY2(fuzzyEqual((actual), (expected)), qPrintable(QStringLiteral("actual %1, expected %2").arg(actual, 0, 'g', 17).arg(expected, 0, 'g', 17)))

class ColorBarTest : public QObject {
    Q_OBJECT

private slots:
    void defaults();
    void negativeSizesAreClamped();
    void clearsSeriesWhenDestroyed();
    void verticalStripRunsFromMinimumAtBottom();
    void horizontalStripRunsFromMinimumOnLeft();
    void reversedColormapFlipsStrip();
    void drawsNothingWithoutColormap();
    void drawsLinearTicks();
    void drawsLogTicks();
    void followsColormapChanges();
    void followsReplacedColormap();
    void implicitSizeFitsContents();
    void tickLabelWidthFixesImplicitWidth();
    void tickLabelsAreClippedToTickLabelWidth();
    void drawsTitleBesideTickLabels();
    void qmlColorBarInsidePlot();
    void pixelToValueInvertsValueToPixel();
    void gesturesAreIgnoredUnlessInteractive();
    void hoverIsReportedWhileInteractive();
    void qmlBorderColorFollowsHovered();
    void leavingInteractiveModeClearsHover();
    void horizontalDragPansRange();
    void verticalDragIsInverted();
    void dragPinsDataResolvedRange();
    void dragOnLogColormapPansInLogSpace();
    void rightButtonDoesNotPan();
    void wheelZoomsAroundCursor();
    void wheelOutUndoesWheelIn();
    void wheelOnLogColormapZoomsInLogSpace();
    void zoomStopsAtSinglePrecision();
    void doubleClickRescalesToData();
    void gesturesAreIgnoredWithoutColormap();
    void sharedColormapMovesEverySeries();
    void qmlInteractiveColorBar();
    void onlyInteractiveBarKeepsEventsFromPlot();
};

void ColorBarTest::defaults()
{
    const auto bar = ColorBar{};

    QCOMPARE(bar.series(), nullptr);
    QCOMPARE(bar.orientation(), ColorBar::Vertical);
    QCOMPARE(bar.barThickness(), 12.0);
    QCOMPARE(bar.borderWidth(), 1.0);
    QCOMPARE(bar.labelPadding(), 6.0);
    QCOMPARE(bar.tickLabelWidth(), 16.0);
    QCOMPARE(bar.labelColor(), ColorPalette::dark().axisLine);
    QCOMPARE(bar.borderColor(), ColorPalette::dark().axisLine);
    QCOMPARE(bar.ticker()->tickLengthIn(), 0.0);
    QCOMPARE(bar.ticker()->tickLengthOut(), 4.0);
    QCOMPARE(bar.ticker()->subtickLengthIn(), 0.0);
    QCOMPARE(bar.ticker()->subtickLengthOut(), 2.0);
    QCOMPARE(bar.ticker()->subtickCount(), 0);
    QCOMPARE(bar.ticker()->tickWidth(), 1.0);
    QCOMPARE(bar.interactive(), false);
    QCOMPARE(bar.zoomScaleFactor(), 0.9);
    QCOMPARE(bar.acceptedMouseButtons(), Qt::MouseButtons{Qt::NoButton});
    QCOMPARE(bar.hovered(), false);
    QCOMPARE(bar.acceptHoverEvents(), false);
}

void ColorBarTest::negativeSizesAreClamped()
{
    auto bar = ColorBar{};
    bar.setBarThickness(-3.0);
    bar.setBorderWidth(-1.0);
    bar.setLabelPadding(-2.0);
    bar.setTickLabelWidth(-5.0);

    QCOMPARE(bar.barThickness(), 0.0);
    QCOMPARE(bar.borderWidth(), 0.0);
    QCOMPARE(bar.labelPadding(), 0.0);
    QCOMPARE(bar.tickLabelWidth(), 0.0);
}

void ColorBarTest::clearsSeriesWhenDestroyed()
{
    auto bar = ColorBar{};
    auto* cloud = new PointCloud{};
    bar.setSeries(cloud);
    QCOMPARE(bar.series(), cloud);

    auto spy = QSignalSpy{&bar, &ColorBar::seriesChanged};
    delete cloud;
    QCOMPARE(bar.series(), nullptr);
    QCOMPARE(spy.count(), 1);
}

void ColorBarTest::verticalStripRunsFromMinimumAtBottom()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    const auto top = qGray(image.pixel(kStripCenter, 20));
    const auto middle = qGray(image.pixel(kStripCenter, 100));
    const auto bottom = qGray(image.pixel(kStripCenter, 180));
    QVERIFY2(top > 220, qPrintable(QString::number(top)));
    QVERIFY2(std::abs(middle - 128) <= kGrayTolerance, qPrintable(QString::number(middle)));
    QVERIFY2(bottom < 35, qPrintable(QString::number(bottom)));
}

void ColorBarTest::horizontalStripRunsFromMinimumOnLeft()
{
    auto scene = Scene{ColorBar::Horizontal, QSize{200, 60}};
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    const auto left = qGray(image.pixel(10, kStripCenter));
    const auto middle = qGray(image.pixel(100, kStripCenter));
    const auto right = qGray(image.pixel(190, kStripCenter));
    QVERIFY2(left < 35, qPrintable(QString::number(left)));
    QVERIFY2(std::abs(middle - 128) <= kGrayTolerance, qPrintable(QString::number(middle)));
    QVERIFY2(right > 220, qPrintable(QString::number(right)));
}

void ColorBarTest::reversedColormapFlipsStrip()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.colormap.setReversed(true);
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    QVERIFY(qGray(image.pixel(kStripCenter, 20)) < 35);
    QVERIFY(qGray(image.pixel(kStripCenter, 180)) > 220);
}

void ColorBarTest::drawsNothingWithoutColormap()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.setLabel(QStringLiteral("Value"));
    scene.cloud.setColormap(nullptr);
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    QVERIFY(isUniform(image, Qt::black));
}

void ColorBarTest::drawsLinearTicks()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.ticker()->setTickColor(Qt::green);
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    // [0, 1] with the default target of 5 ticks steps by 0.2.
    QCOMPARE(runsInColumn(image, kMajorTickColumn, Qt::green), 6);
}

void ColorBarTest::drawsLogTicks()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.ticker()->setTickColor(Qt::green);
    scene.colormap.setNorm(Colormap::Normalization::Log);
    scene.colormap.setMin(1.0);
    scene.colormap.setMax(1000.0);
    const auto image = scene.render();
    QVERIFY(!image.isNull());

    // One major tick per decade: 1, 10, 100, 1000.
    QCOMPARE(runsInColumn(image, kMajorTickColumn, Qt::green), 4);
}

void ColorBarTest::followsColormapChanges()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    QVERIFY(!scene.render().isNull());
    QCOMPARE(scene.bar.valueToPixel(1.0, 100.0), 0.0);

    scene.colormap.setMax(2.0);
    scene.render();
    QCOMPARE(scene.bar.valueToPixel(1.0, 100.0), 50.0);

    scene.colormap.setNorm(Colormap::Normalization::Log);
    scene.colormap.setMin(1.0);
    scene.colormap.setMax(100.0);
    scene.render();
    QCOMPARE(scene.bar.valueToPixel(10.0, 100.0), 50.0);
}

void ColorBarTest::followsReplacedColormap()
{
    auto scene = Scene{ColorBar::Horizontal, QSize{200, 60}};
    QVERIFY(!scene.render().isNull());

    auto replacement = Colormap{};
    replacement.setMin(10.0);
    replacement.setMax(20.0);
    scene.cloud.setColormap(&replacement);
    scene.render();
    QCOMPARE(scene.bar.valueToPixel(15.0, 100.0), 50.0);

    // Only the assigned colormap is followed.
    replacement.setMax(30.0);
    scene.colormap.setMax(5.0);
    scene.render();
    QCOMPARE(scene.bar.valueToPixel(20.0, 100.0), 50.0);
}

void ColorBarTest::implicitSizeFitsContents()
{
    auto vertical = Scene{ColorBar::Vertical, QSize{100, 200}};
    QVERIFY(!vertical.render().isNull());
    QCOMPARE(vertical.bar.implicitHeight(), 160.0);
    // Strip, outward tick, and label padding, plus the reserved tick label width.
    QCOMPARE(vertical.bar.implicitWidth(), 12.0 + 4.0 + vertical.bar.ticker()->tickLabelPadding() + 16.0);

    const auto withoutTitle = vertical.bar.implicitWidth();
    vertical.bar.setLabel(QStringLiteral("Value"));
    vertical.render();
    QVERIFY(vertical.bar.implicitWidth() >= withoutTitle + vertical.bar.labelPadding() + QFontMetricsF{vertical.bar.labelFont()}.height());

    auto horizontal = Scene{ColorBar::Horizontal, QSize{200, 60}};
    QVERIFY(!horizontal.render().isNull());
    QCOMPARE(horizontal.bar.implicitWidth(), 160.0);
    QVERIFY(horizontal.bar.implicitHeight() > 12.0 + 4.0 + 5.0);
}

void ColorBarTest::tickLabelWidthFixesImplicitWidth()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    QVERIFY(!scene.render().isNull());
    // Strip, outward tick, and label padding.
    const auto beforeLabels = 12.0 + 4.0 + scene.bar.ticker()->tickLabelPadding();
    QCOMPARE(scene.bar.implicitWidth(), beforeLabels + 16.0);

    // Wider labels do not widen the bar.
    scene.colormap.setRange(-1000.5, 1000.5);
    scene.render();
    QCOMPARE(scene.bar.implicitWidth(), beforeLabels + 16.0);

    scene.bar.setTickLabelWidth(40.0);
    scene.render();
    QCOMPARE(scene.bar.implicitWidth(), beforeLabels + 40.0);
    scene.bar.setTickLabelWidth(0.0);
    scene.render();
    QCOMPARE(scene.bar.implicitWidth(), beforeLabels);

    // The labels of a horizontal bar sit below the strip, so their width is not reserved.
    scene.bar.setOrientation(ColorBar::Horizontal);
    scene.render();
    const auto height = scene.bar.implicitHeight();
    scene.bar.setTickLabelWidth(40.0);
    scene.render();
    QCOMPARE(scene.bar.implicitHeight(), height);
}

void ColorBarTest::tickLabelsAreClippedToTickLabelWidth()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.ticker()->setTickLabelColor(Qt::white);
    scene.colormap.setRange(-1000.5, 1000.5);
    // Strip, outward tick, and label padding.
    const auto firstLabelColumn = static_cast<int>(12.0 + 4.0 + scene.bar.ticker()->tickLabelPadding());
    QVERIFY(hasContentFrom(scene.render(), firstLabelColumn + 6));

    scene.bar.setTickLabelWidth(6.0);
    QVERIFY(hasContentFrom(scene.render(), firstLabelColumn));
    QVERIFY(!hasContentFrom(scene.render(), firstLabelColumn + 6));

    scene.bar.setTickLabelWidth(0.0);
    QVERIFY(!hasContentFrom(scene.render(), firstLabelColumn));
}

void ColorBarTest::drawsTitleBesideTickLabels()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.setBorderWidth(0.0);
    QVERIFY(!scene.render().isNull());
    const auto titleColumn = static_cast<int>(scene.bar.implicitWidth());

    QVERIFY(!hasContentFrom(scene.render(), titleColumn));
    scene.bar.setLabelColor(Qt::white);
    scene.bar.setLabel(QStringLiteral("Value"));
    QVERIFY(hasContentFrom(scene.render(), titleColumn));
}

void ColorBarTest::qmlColorBarInsidePlot()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot

        QAccelPlot.Plot {
            width: 300
            height: 200
            QAccelPlot.PointCloud {
                id: cloud
                objectName: "cloud"
                colormap: QAccelPlot.Colormap { preset: QAccelPlot.Colormap.Plasma; min: 0; max: 5 }
            }
            QAccelPlot.ColorBar {
                objectName: "bar"
                series: cloud
                orientation: QAccelPlot.ColorBar.Horizontal
                label: "Jump length"
            }
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));

    auto* const bar = root->findChild<ColorBar*>(QStringLiteral("bar"));
    auto* const cloud = root->findChild<PointCloud*>(QStringLiteral("cloud"));
    QVERIFY(bar);
    QVERIFY(cloud);
    QCOMPARE(bar->series(), cloud);
    QCOMPARE(bar->orientation(), ColorBar::Horizontal);
    QCOMPARE(bar->label(), QStringLiteral("Jump length"));
    QCOMPARE(root->property("series").value<QList<PlotSeries*>>().size(), 1);
}

void ColorBarTest::pixelToValueInvertsValueToPixel()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.colormap.setRange(2.0, 6.0);
    QVERIFY(!scene.render().isNull());

    // A vertical bar has its maximum at the top.
    QCOMPARE_NEAR(scene.bar.pixelToValue(0.0, 100.0), 6.0);
    QCOMPARE_NEAR(scene.bar.pixelToValue(25.0, 100.0), 5.0);
    QCOMPARE_NEAR(scene.bar.pixelToValue(scene.bar.valueToPixel(3.5, 100.0), 100.0), 3.5);
    QCOMPARE(scene.bar.pixelToValue(10.0, 0.0), 2.0);

    scene.bar.setOrientation(ColorBar::Horizontal);
    scene.colormap.setNorm(Colormap::Normalization::Log);
    scene.colormap.setRange(1.0, 100.0);
    scene.render();
    QCOMPARE_NEAR(scene.bar.pixelToValue(50.0, 100.0), 10.0);
}

void ColorBarTest::gesturesAreIgnoredUnlessInteractive()
{
    auto scene = Scene{ColorBar::Horizontal, QSize{201, 60}};
    QVERIFY(!scene.render().isNull());
    const auto center = QPointF{100.5, 30.0};

    drag(scene.bar, center, center + QPointF{50.0, 0.0});
    QVERIFY(!wheel(scene.bar, center, kWheelStep));
    doubleClick(scene.bar, center);
    QCOMPARE(scene.colormap.min(), 0.0);
    QCOMPARE(scene.colormap.max(), 1.0);

    scene.bar.setInteractive(true);
    QCOMPARE(scene.bar.acceptedMouseButtons(), Qt::MouseButtons{Qt::LeftButton});
    scene.bar.setInteractive(false);
    QCOMPARE(scene.bar.acceptedMouseButtons(), Qt::MouseButtons{Qt::NoButton});
}

void ColorBarTest::hoverIsReportedWhileInteractive()
{
    auto scene = Scene{ColorBar::Vertical, QSize{100, 200}};
    scene.bar.setInteractive(true);
    QVERIFY(scene.bar.acceptHoverEvents());

    auto spy = QSignalSpy{&scene.bar, &ColorBar::hoveredChanged};
    auto enter = QHoverEvent{QEvent::HoverEnter, QPointF{6.0, 100.0}, QPointF{6.0, 100.0}, Qt::NoModifier};
    scene.bar.hoverEnterEvent(&enter);
    QVERIFY(scene.bar.hovered());
    QCOMPARE(spy.count(), 1);

    auto leave = QHoverEvent{QEvent::HoverLeave, {}, {}, Qt::NoModifier};
    scene.bar.hoverLeaveEvent(&leave);
    QVERIFY(!scene.bar.hovered());
    QCOMPARE(spy.count(), 2);
}

void ColorBarTest::qmlBorderColorFollowsHovered()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot

        QAccelPlot.ColorBar {
            interactive: true
            borderColor: hovered ? "#00ff00" : "#ff0000"
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* const bar = qobject_cast<ColorBar*>(root.get());
    QVERIFY(bar);
    QCOMPARE(bar->borderColor(), QColor{"#ff0000"});

    auto enter = QHoverEvent{QEvent::HoverEnter, QPointF{6.0, 100.0}, QPointF{6.0, 100.0}, Qt::NoModifier};
    QCoreApplication::sendEvent(bar, &enter);
    QCOMPARE(bar->borderColor(), QColor{"#00ff00"});

    auto leave = QHoverEvent{QEvent::HoverLeave, {}, {}, Qt::NoModifier};
    QCoreApplication::sendEvent(bar, &leave);
    QCOMPARE(bar->borderColor(), QColor{"#ff0000"});
}

void ColorBarTest::leavingInteractiveModeClearsHover()
{
    auto scene = InteractiveScene{ColorBar::Vertical};
    auto enter = QHoverEvent{QEvent::HoverEnter, scene.center(), scene.center(), Qt::NoModifier};
    scene.bar.hoverEnterEvent(&enter);
    QVERIFY(scene.bar.hovered());

    auto spy = QSignalSpy{&scene.bar, &ColorBar::hoveredChanged};
    scene.bar.setInteractive(false);
    QVERIFY(!scene.bar.hovered());
    QCOMPARE(spy.count(), 1);
    QVERIFY(!scene.bar.acceptHoverEvents());
}

void ColorBarTest::horizontalDragPansRange()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.colormap.setRange(10.0, 30.0);

    // The colors follow the cursor: dragging right by a quarter of the strip shows lower values.
    drag(scene.bar, scene.center(), scene.center() + QPointF{scene.stripLength() / 4.0, 20.0});
    QCOMPARE_NEAR(scene.colormap.min(), 5.0);
    QCOMPARE_NEAR(scene.colormap.max(), 25.0);
}

void ColorBarTest::verticalDragIsInverted()
{
    auto scene = InteractiveScene{ColorBar::Vertical};
    scene.colormap.setRange(10.0, 30.0);

    // The minimum is at the bottom, so dragging down shows higher values.
    drag(scene.bar, scene.center(), scene.center() + QPointF{20.0, scene.stripLength() / 4.0});
    QCOMPARE_NEAR(scene.colormap.min(), 15.0);
    QCOMPARE_NEAR(scene.colormap.max(), 35.0);
}

void ColorBarTest::dragPinsDataResolvedRange()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.cloud.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f}, std::vector<float>{2.0f, 6.0f}, 2);
    scene.colormap.setRange(qQNaN(), qQNaN());
    QCOMPARE(scene.cloud.dataValueMin(), 2.0);
    QCOMPARE(scene.cloud.dataValueMax(), 6.0);

    drag(scene.bar, scene.center(), scene.center() - QPointF{scene.stripLength() / 2.0, 0.0});
    QCOMPARE_NEAR(scene.colormap.min(), 4.0);
    QCOMPARE_NEAR(scene.colormap.max(), 8.0);
    QCOMPARE_NEAR(scene.cloud.dataValueMin(), 4.0);
    QCOMPARE_NEAR(scene.cloud.dataValueMax(), 8.0);
}

void ColorBarTest::dragOnLogColormapPansInLogSpace()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.colormap.setNorm(Colormap::Normalization::Log);
    scene.colormap.setRange(1.0, 100.0);

    // Half of two decades is one decade.
    drag(scene.bar, scene.center(), scene.center() - QPointF{scene.stripLength() / 2.0, 0.0});
    QCOMPARE_NEAR(scene.colormap.min(), 10.0);
    QCOMPARE_NEAR(scene.colormap.max(), 1000.0);
}

void ColorBarTest::rightButtonDoesNotPan()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};

    drag(scene.bar, scene.center(), scene.center() + QPointF{50.0, 0.0}, Qt::RightButton);
    QCOMPARE(scene.colormap.min(), 0.0);
    QCOMPARE(scene.colormap.max(), 1.0);
}

void ColorBarTest::wheelZoomsAroundCursor()
{
    auto scene = InteractiveScene{ColorBar::Vertical};
    scene.colormap.setRange(0.0, 80.0);
    scene.bar.setZoomScaleFactor(0.5);

    // A quarter of the strip above its bottom end, where the value is 20.
    const auto cursor = scene.center() + QPointF{0.0, scene.stripLength() / 4.0};
    QVERIFY(wheel(scene.bar, cursor, kWheelStep));
    QCOMPARE_NEAR(scene.colormap.min(), 10.0);
    QCOMPARE_NEAR(scene.colormap.max(), 50.0);
}

void ColorBarTest::wheelOutUndoesWheelIn()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.colormap.setRange(-4.0, 12.0);
    const auto cursor = scene.center() + QPointF{30.0, 0.0};

    wheel(scene.bar, cursor, kWheelStep);
    QVERIFY(scene.colormap.max() - scene.colormap.min() < 16.0);
    wheel(scene.bar, cursor, -kWheelStep);
    QCOMPARE_NEAR(scene.colormap.min(), -4.0);
    QCOMPARE_NEAR(scene.colormap.max(), 12.0);
}

void ColorBarTest::wheelOnLogColormapZoomsInLogSpace()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.colormap.setNorm(Colormap::Normalization::Log);
    scene.colormap.setRange(1.0, 10000.0);
    scene.bar.setZoomScaleFactor(0.5);

    // The center of four decades is 100.
    wheel(scene.bar, scene.center(), kWheelStep);
    QCOMPARE_NEAR(scene.colormap.min(), 10.0);
    QCOMPARE_NEAR(scene.colormap.max(), 1000.0);
}

void ColorBarTest::zoomStopsAtSinglePrecision()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.colormap.setRange(1000.0, 1001.0);
    scene.bar.setZoomScaleFactor(0.5);

    for (auto step = 0; step < 100; ++step) {
        wheel(scene.bar, scene.center(), kWheelStep);
    }
    // Single precision resolves about seven digits, and the series draws with that.
    const auto extent = scene.colormap.max() - scene.colormap.min();
    QVERIFY2(extent > 1000.0 * 1e-6, qPrintable(QString::number(extent, 'g', 17)));
    QVERIFY2(extent < 1000.0 * 1e-5, qPrintable(QString::number(extent, 'g', 17)));
}

void ColorBarTest::doubleClickRescalesToData()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.cloud.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f}, std::vector<float>{2.0f, 6.0f}, 2);
    scene.colormap.setRange(3.0, 4.0);

    doubleClick(scene.bar, scene.center());
    QVERIFY(std::isnan(scene.colormap.min()));
    QVERIFY(std::isnan(scene.colormap.max()));
    QCOMPARE(scene.cloud.dataValueMin(), 2.0);
    QCOMPARE(scene.cloud.dataValueMax(), 6.0);
}

void ColorBarTest::gesturesAreIgnoredWithoutColormap()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    scene.cloud.setColormap(nullptr);
    scene.render();

    drag(scene.bar, scene.center(), scene.center() + QPointF{50.0, 0.0});
    QVERIFY(!wheel(scene.bar, scene.center(), kWheelStep));
    doubleClick(scene.bar, scene.center());
    QCOMPARE(scene.colormap.min(), 0.0);
    QCOMPARE(scene.colormap.max(), 1.0);

    scene.bar.setSeries(nullptr);
    QVERIFY(!wheel(scene.bar, scene.center(), kWheelStep));
    scene.bar.rescaleToData();
}

void ColorBarTest::sharedColormapMovesEverySeries()
{
    auto scene = InteractiveScene{ColorBar::Horizontal};
    auto other = PointCloud{};
    other.setColormap(&scene.colormap);
    scene.colormap.setRange(10.0, 30.0);

    drag(scene.bar, scene.center(), scene.center() + QPointF{scene.stripLength() / 4.0, 0.0});
    QCOMPARE_NEAR(other.dataValueMin(), 5.0);
    QCOMPARE_NEAR(other.dataValueMax(), 25.0);
}

void ColorBarTest::qmlInteractiveColorBar()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot

        Item {
            QAccelPlot.Colormap { id: ramp; objectName: "colormap"; min: 3; max: 4 }
            QAccelPlot.PointCloud { id: cloud; colormap: ramp }
            QAccelPlot.ColorBar {
                objectName: "bar"
                series: cloud
                interactive: true
                zoomScaleFactor: 0.8
                Component.onCompleted: {
                    ramp.setRange(1, 2);
                    rescaleToData();
                }
            }
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));

    const auto* const bar = root->findChild<ColorBar*>(QStringLiteral("bar"));
    const auto* const colormap = root->findChild<Colormap*>(QStringLiteral("colormap"));
    QVERIFY(bar);
    QVERIFY(colormap);
    QCOMPARE(bar->interactive(), true);
    QCOMPARE(bar->zoomScaleFactor(), 0.8);
    QVERIFY(std::isnan(colormap->min()));
    QVERIFY(std::isnan(colormap->max()));
}

void ColorBarTest::onlyInteractiveBarKeepsEventsFromPlot()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot

        QAccelPlot.Plot {
            id: plot
            width: 400
            height: 300
            legendVisible: false
            xAxis: QAccelPlot.Axis { objectName: "xAxis"; viewportMin: 0; viewportMax: 10 }
            yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }
            QAccelPlot.PointCloud {
                id: cloud
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                colormap: QAccelPlot.Colormap { objectName: "colormap"; min: 0; max: 5 }
            }
            QAccelPlot.ColorBar {
                objectName: "bar"
                series: cloud
                orientation: QAccelPlot.ColorBar.Horizontal
                x: plot.plotRect.x + 20
                y: plot.plotRect.y + 20
            }
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QQuickItem>{qobject_cast<QQuickItem*>(component.create())};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* const bar = root->findChild<ColorBar*>(QStringLiteral("bar"));
    const auto* const colormap = root->findChild<Colormap*>(QStringLiteral("colormap"));
    const auto* const xAxis = root->findChild<Axis*>(QStringLiteral("xAxis"));
    QVERIFY(bar && colormap && xAxis);

    auto window = QQuickWindow{};
    window.resize(400, 300);
    root->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(!window.grabWindow().isNull());

    const auto press = bar->mapToScene(QPointF{bar->width() / 2.0, 6.0}).toPoint();
    const auto dragOnBar = [&window, press]() {
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, press);
        QTest::mouseMove(&window, press + QPoint{40, 0});
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, press + QPoint{40, 0});
    };
    const auto wheelOnBar = [&window, press]() {
        auto event = QWheelEvent{
            QPointF{press}, window.mapToGlobal(QPointF{press}), QPoint{}, QPoint{0, kWheelStep}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false};
        QCoreApplication::sendEvent(&window, &event);
    };

    // A bar that is not interactive leaves the gestures to the plot.
    dragOnBar();
    QVERIFY(xAxis->viewportMin() < 0.0);
    wheelOnBar();
    QVERIFY(xAxis->viewportMax() - xAxis->viewportMin() < 10.0);
    QCOMPARE(colormap->min(), 0.0);
    QCOMPARE(colormap->max(), 5.0);

    bar->setInteractive(true);
    const auto viewportMin = xAxis->viewportMin();
    const auto viewportMax = xAxis->viewportMax();
    dragOnBar();
    QVERIFY(colormap->min() < 0.0);
    QCOMPARE_NEAR(colormap->max() - colormap->min(), 5.0);
    wheelOnBar();
    QVERIFY(colormap->max() - colormap->min() < 5.0);
    QCOMPARE(xAxis->viewportMin(), viewportMin);
    QCOMPARE(xAxis->viewportMax(), viewportMax);
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel positions are logical, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::ColorBarTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_color_bar.moc"
