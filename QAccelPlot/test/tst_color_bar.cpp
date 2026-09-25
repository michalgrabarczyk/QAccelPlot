//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/colorbar/ColorBar.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QFontMetricsF>
#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtTest/QtTest>

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

/// A value-colored point cloud keyed by a color bar that fills a black window.
struct Scene {
    QQuickWindow window;
    Colormap colormap;
    PointCloud cloud;
    ColorBar bar{window.contentItem()};

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
    void drawsTitleBesideTickLabels();
    void qmlColorBarInsidePlot();
};

void ColorBarTest::defaults()
{
    const auto bar = ColorBar{};

    QCOMPARE(bar.series(), nullptr);
    QCOMPARE(bar.orientation(), ColorBar::Vertical);
    QCOMPARE(bar.barThickness(), 12.0);
    QCOMPARE(bar.borderWidth(), 1.0);
    QCOMPARE(bar.labelPadding(), 6.0);
    QCOMPARE(bar.labelColor(), ColorPalette::dark().axisLine);
    QCOMPARE(bar.borderColor(), ColorPalette::dark().axisLine);
    QCOMPARE(bar.ticker()->tickLengthIn(), 0.0);
    QCOMPARE(bar.ticker()->tickLengthOut(), 4.0);
    QCOMPARE(bar.ticker()->subtickLengthIn(), 0.0);
    QCOMPARE(bar.ticker()->subtickLengthOut(), 2.0);
    QCOMPARE(bar.ticker()->subtickCount(), 0);
    QCOMPARE(bar.ticker()->tickWidth(), 1.0);
}

void ColorBarTest::negativeSizesAreClamped()
{
    auto bar = ColorBar{};
    bar.setBarThickness(-3.0);
    bar.setBorderWidth(-1.0);
    bar.setLabelPadding(-2.0);

    QCOMPARE(bar.barThickness(), 0.0);
    QCOMPARE(bar.borderWidth(), 0.0);
    QCOMPARE(bar.labelPadding(), 0.0);
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
    // Strip, outward tick, and label padding, plus the widest tick label.
    QVERIFY(vertical.bar.implicitWidth() > 12.0 + 4.0 + 5.0);

    const auto withoutTitle = vertical.bar.implicitWidth();
    vertical.bar.setLabel(QStringLiteral("Value"));
    vertical.render();
    QVERIFY(vertical.bar.implicitWidth() >= withoutTitle + vertical.bar.labelPadding() + QFontMetricsF{vertical.bar.labelFont()}.height());

    auto horizontal = Scene{ColorBar::Horizontal, QSize{200, 60}};
    QVERIFY(!horizontal.render().isNull());
    QCOMPARE(horizontal.bar.implicitWidth(), 160.0);
    QVERIFY(horizontal.bar.implicitHeight() > 12.0 + 4.0 + 5.0);
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
