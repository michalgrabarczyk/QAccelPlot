//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/Category.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"
#include "QAccelPlot/transitions/GrowTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <cstdlib>
#include <vector>

namespace QAccelPlot {

namespace {

// 200 x 200 px plot showing data in [0, 10] x [0, 10].
constexpr auto kSize = 200;
constexpr auto kDataSpan = 10.0;
constexpr auto kHeight = 8.0;
// Low enough that a bar still covers it early in a transition.
constexpr auto kProbeY = 1.0;
constexpr auto kLongDuration = 10'000;

const auto kSeriesColor = QColor{Qt::blue};
const auto kUnknownColor = QColor{Qt::red};
const auto kKnownColor = QColor{Qt::green};

const auto kCategoryName = QStringLiteral("categoryName");

// Returns the test color \a pixel shows, or \a pixel when it is none of them.
QColor testColor(const QColor& pixel)
{
    constexpr auto kTolerance = 60;
    for (const auto& color : {kSeriesColor, kUnknownColor, kKnownColor}) {
        if (std::abs(pixel.red() - color.red()) < kTolerance && std::abs(pixel.green() - color.green()) < kTolerance
            && std::abs(pixel.blue() - color.blue()) < kTolerance) {
            return color;
        }
    }
    return pixel;
}

QVariantMap barAtX(const qreal x, const QVariant& category = {})
{
    auto bar = QVariantMap{{QStringLiteral("position"), x}, {QStringLiteral("value"), kHeight}};
    if (category.isValid()) {
        bar.insert(QStringLiteral("category"), category);
    }
    return bar;
}

QVariantMap rectangleAtX(const qreal x, const QVariant& category = {})
{
    auto rectangle
        = QVariantMap{{QStringLiteral("x1"), x - 0.5}, {QStringLiteral("y1"), 0.0}, {QStringLiteral("x2"), x + 0.5}, {QStringLiteral("y2"), kHeight}};
    if (category.isValid()) {
        rectangle.insert(QStringLiteral("category"), category);
    }
    return rectangle;
}

// A hidden window with a series that lists the categories -1 ("Unknown") and 0 ("Known").
template <typename Series> class Scene {
public:
    Scene()
    {
        xAxis_.setOrientation(Qt::Horizontal);
        yAxis_.setOrientation(Qt::Vertical);
        for (auto* axis : {&xAxis_, &yAxis_}) {
            axis->setViewportMin(0.0);
            axis->setViewportMax(kDataSpan);
        }
        window_.setColor(Qt::black);
        window_.resize(kSize, kSize);
        series.setParentItem(window_.contentItem());
        series.setXAxis(&xAxis_);
        series.setYAxis(&yAxis_);
        series.setPlotRect({0, 0, kSize, kSize});
        series.setColor(kSeriesColor);
        series.setCategories(
            {new Category{-1, QStringLiteral("Unknown"), kUnknownColor, &series}, new Category{0, QStringLiteral("Known"), kKnownColor, &series}});
    }

    bool isSoftware() const
    {
        return window_.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    QColor colorAt(const qreal x)
    {
        const auto image = window_.grabWindow();
        return testColor(image.pixelColor(qRound(x / kDataSpan * kSize), qRound((1.0 - kProbeY / kDataSpan) * kSize)));
    }

private:
    Axis xAxis_;
    Axis yAxis_;
    QQuickWindow window_;

public:
    Series series;
};

} // namespace

class UncategorizedDataPointsTest : public QObject {
    Q_OBJECT

private slots:
    void barsWithoutACategoryUseTheSeriesColor();
    void barsClearedByDataPointCategoriesUseTheSeriesColor();
    void replacedBarsKeepOrClearTheirCategory();
    void movedCategoryValuesKeepMinusOneAsACategory();
    void removedBarWithoutACategoryKeepsTheSeriesColor();
    void barsLosingTheirCategoriesAnimateInTheSeriesColor();
    void categoriesSetDuringATransitionLeaveUnsetBarsAlone();
    void rectanglesWithoutACategoryUseTheSeriesColor();
    void rectanglesClearedByDataPointCategoriesUseTheSeriesColor();
};

void UncategorizedDataPointsTest::barsWithoutACategoryUseTheSeriesColor()
{
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }

    scene.series.setData(QVariantList{barAtX(2.0, 0), barAtX(5.0), barAtX(8.0, -1)});

    QVERIFY(!scene.series.barAt(1).contains(kCategoryName));
    QCOMPARE(scene.series.barAt(2).value(kCategoryName).toString(), QStringLiteral("Unknown"));
    QCOMPARE(scene.colorAt(2.0), kKnownColor);
    QCOMPARE(scene.colorAt(5.0), kSeriesColor);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
}

void UncategorizedDataPointsTest::barsClearedByDataPointCategoriesUseTheSeriesColor()
{
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{barAtX(2.0), barAtX(5.0), barAtX(8.0)});

    scene.series.setDataPointCategories({0, QVariant{}, -1});

    QVERIFY(!scene.series.barAt(1).contains(kCategoryName));
    QCOMPARE(scene.series.barAt(2).value(kCategoryName).toString(), QStringLiteral("Unknown"));
    QCOMPARE(scene.colorAt(5.0), kSeriesColor);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
}

void UncategorizedDataPointsTest::replacedBarsKeepOrClearTheirCategory()
{
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{barAtX(2.0), barAtX(5.0), barAtX(8.0, -1)});

    // Giving one bar a category leaves the others without one.
    scene.series.replaceData(1, 5.0, kHeight, 0);
    QCOMPARE(scene.colorAt(2.0), kSeriesColor);
    QCOMPARE(scene.colorAt(5.0), kKnownColor);

    scene.series.replaceData(1, 5.0, kHeight, QVariant{});
    QVERIFY(!scene.series.barAt(1).contains(kCategoryName));
    QCOMPARE(scene.colorAt(5.0), kSeriesColor);

    scene.series.replaceData(2, 8.0, kHeight);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
    scene.series.replaceData(2, 8.0, kHeight, QVariant{});
    QCOMPARE(scene.colorAt(8.0), kSeriesColor);
    scene.series.replaceData(2, 8.0, kHeight, -1);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
}

void UncategorizedDataPointsTest::movedCategoryValuesKeepMinusOneAsACategory()
{
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }

    scene.series.setData(std::vector<double>{2.0, kHeight, 5.0, kHeight}, std::vector<int>{-1, 0}, 2);

    QCOMPARE(scene.series.barAt(0).value(kCategoryName).toString(), QStringLiteral("Unknown"));
    QCOMPARE(scene.colorAt(2.0), kUnknownColor);
    QCOMPARE(scene.colorAt(5.0), kKnownColor);
}

void UncategorizedDataPointsTest::removedBarWithoutACategoryKeepsTheSeriesColor()
{
    auto grow = GrowTransition{};
    grow.setDuration(kLongDuration);
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{barAtX(2.0, 0), barAtX(7.0)});
    scene.series.setExitTransition(&grow);

    scene.series.setData(QVariantList{barAtX(2.0, 0)});

    QVERIFY(grow.running());
    QCOMPARE(scene.colorAt(2.0), kKnownColor);
    QCOMPARE(scene.colorAt(7.0), kSeriesColor);
}

void UncategorizedDataPointsTest::barsLosingTheirCategoriesAnimateInTheSeriesColor()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{barAtX(2.0, 0)});
    scene.series.setUpdateTransition(&morph);

    scene.series.setData(std::vector<double>{2.0, 6.0}, 1);

    QVERIFY(morph.running());
    QCOMPARE(scene.colorAt(2.0), kSeriesColor);
}

void UncategorizedDataPointsTest::categoriesSetDuringATransitionLeaveUnsetBarsAlone()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto scene = Scene<BarSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{barAtX(2.0), barAtX(7.0)});
    scene.series.setUpdateTransition(&morph);
    scene.series.setData(std::vector<double>{2.0, 6.0, 7.0, 6.0}, 2);
    QVERIFY(morph.running());

    scene.series.setDataPointCategories({QVariant{}, 0});

    QCOMPARE(scene.colorAt(2.0), kSeriesColor);
    QCOMPARE(scene.colorAt(7.0), kKnownColor);
}

void UncategorizedDataPointsTest::rectanglesWithoutACategoryUseTheSeriesColor()
{
    auto scene = Scene<RectangleSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }

    scene.series.setData(QVariantList{rectangleAtX(2.0, 0), rectangleAtX(5.0), rectangleAtX(8.0, -1)});

    QVERIFY(!scene.series.rectangleAt(1).contains(kCategoryName));
    QCOMPARE(scene.series.rectangleAt(2).value(kCategoryName).toString(), QStringLiteral("Unknown"));
    QCOMPARE(scene.colorAt(2.0), kKnownColor);
    QCOMPARE(scene.colorAt(5.0), kSeriesColor);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
}

void UncategorizedDataPointsTest::rectanglesClearedByDataPointCategoriesUseTheSeriesColor()
{
    auto scene = Scene<RectangleSeries>{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene.series.setData(QVariantList{rectangleAtX(2.0), rectangleAtX(5.0), rectangleAtX(8.0)});

    scene.series.setDataPointCategories({0, QVariant{}, -1});

    QVERIFY(!scene.series.rectangleAt(1).contains(kCategoryName));
    QCOMPARE(scene.series.rectangleAt(2).value(kCategoryName).toString(), QStringLiteral("Unknown"));
    QCOMPARE(scene.colorAt(5.0), kSeriesColor);
    QCOMPARE(scene.colorAt(8.0), kUnknownColor);
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::UncategorizedDataPointsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_uncategorized_data_points.moc"
