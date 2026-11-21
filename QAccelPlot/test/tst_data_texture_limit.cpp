//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/series/CandlestickSeries.hpp"
#include "QAccelPlot/series/ErrorBarSeries.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGNode>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

namespace {

// Set as QACCELPLOT_MAX_TEXTURE_SIZE in main(): the minimum OpenGL ES 3.0 guarantees.
constexpr auto kTextureLimit = 2048;
constexpr auto kSize = 100;
constexpr auto kFirstUploadFloats = 5000;
constexpr auto kSecondUploadFloats = 9000;

struct UploadNode : QSGNode {
    DataTexture texture;
};

// Uploads twice in one sync, so the second upload takes the in-place path.
class UploadProbe : public QQuickItem {
public:
    UploadProbe()
    {
        setFlag(ItemHasContents);
        setSize({kSize, kSize});
    }

    QSize firstSize;
    QSize secondSize;

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override
    {
        auto* node = oldNode ? static_cast<UploadNode*>(oldNode) : new UploadNode;
        const auto data = std::vector<float>(kSecondUploadFloats, 1.0f);
        node->texture.upload(window(), data.data(), kFirstUploadFloats);
        firstSize = node->texture.texture()->textureSize();
        node->texture.upload(window(), data.data(), kSecondUploadFloats);
        secondSize = node->texture.texture()->textureSize();
        return node;
    }
};

// Whether \a series draws its last record, the only one in view, in a hidden window.
bool drawsLastRecord(PlotSeries& series, std::vector<float>&& data, const int count)
{
    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kSize, kSize);
    auto xAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Qt::Vertical);
    series.setParentItem(window.contentItem());
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    series.setPlotRect({0, 0, kSize, kSize});
    series.setDataF(std::move(data), count);
    const auto image = window.grabWindow();
    series.setParentItem(nullptr);
    return qGray(image.pixel(kSize / 2, kSize / 2)) > 200;
}

// \a count records of \a stride floats: all left of the view except a last one given by \a last.
std::vector<float> recordsEndingWith(const int count, const int stride, const std::vector<float>& last)
{
    auto data = std::vector<float>(static_cast<size_t>(count - 1) * stride, 0.5f);
    for (auto index = 0; index < count - 1; ++index) {
        data[static_cast<size_t>(index) * stride] = static_cast<float>(index - 2 * count);
    }
    data.insert(data.end(), last.begin(), last.end());
    return data;
}

} // namespace

class DataTextureLimitTest : public QObject {
    Q_OBJECT

private slots:
    void capacityFitsTextureLimit_data();
    void capacityFitsTextureLimit();
    void uploadFitsTextureLimit();
    void recordsPastFirstRowsAreDrawn();
};

void DataTextureLimitTest::capacityFitsTextureLimit_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<qint64>("floats");

    QTest::newRow("OpenGL ES 3.0 minimum") << 2048 << qint64{2048} * 2048;
    QTest::newRow("Vulkan minimum") << 4096 << qint64{4096} * 4096;
    QTest::newRow("full row width") << 8192 << qint64{8192} * 8192;
    QTest::newRow("rows beyond the row width") << 16384 << qint64{8192} * 16384;
}

void DataTextureLimitTest::capacityFitsTextureLimit()
{
    QFETCH(int, limit);
    QFETCH(qint64, floats);

    QCOMPARE(Internal::dataTextureItemCapacity(limit, 1), floats);
    QCOMPARE(Internal::dataTextureItemCapacity(limit, 4), floats / 4);
}

void DataTextureLimitTest::uploadFitsTextureLimit()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Data textures require a hardware scene graph backend");
    }
    window.resize(kSize, kSize);
    auto probe = UploadProbe{};
    probe.setParentItem(window.contentItem());

    window.grabWindow();

    QCOMPARE(probe.firstSize, QSize(kTextureLimit, 3));
    QCOMPARE(probe.secondSize, QSize(kTextureLimit, 5));
    probe.setParentItem(nullptr);
}

void DataTextureLimitTest::recordsPastFirstRowsAreDrawn()
{
    if (QQuickWindow{}.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCount = 3000;

    auto cloud = PointCloud{};
    cloud.setColor(Qt::white);
    QVERIFY(drawsLastRecord(cloud, recordsEndingWith(kCount, 2, {0.5f, 0.5f}), kCount));

    auto errorBars = ErrorBarSeries{};
    errorBars.setColor(Qt::white);
    errorBars.setLineWidth(6.0);
    QVERIFY(drawsLastRecord(errorBars, recordsEndingWith(kCount, 4, {0.5f, 0.5f, 0.2f, 0.8f}), kCount));

    auto candles = CandlestickSeries{};
    candles.setUpColor(Qt::white);
    candles.setCandleWidth(0.4);
    QVERIFY(drawsLastRecord(candles, recordsEndingWith(kCount, 5, {0.5f, 0.3f, 0.8f, 0.2f, 0.7f}), kCount));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QACCELPLOT_MAX_TEXTURE_SIZE", QByteArray::number(QAccelPlot::kTextureLimit));
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::DataTextureLimitTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_data_texture_limit.moc"
