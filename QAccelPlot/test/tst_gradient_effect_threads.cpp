//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QMutex>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSet>
#include <QSignalBlocker>
#include <QThread>
#include <QVariantList>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

// Records every thread that reads a gradient property, standing in for a QML Gradient.
class ReadLog {
public:
    void record()
    {
        const auto locker = QMutexLocker{&mutex_};
        threads_.insert(QThread::currentThread());
    }

    QSet<QThread*> threads() const
    {
        const auto locker = QMutexLocker{&mutex_};
        return threads_;
    }

private:
    mutable QMutex mutex_;
    QSet<QThread*> threads_;
};

class ProbeStop : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal position READ position CONSTANT)
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)

public:
    ProbeStop(const qreal position, const QColor& color, ReadLog& log, QObject* parent)
        : QObject(parent)
        , position_(position)
        , color_(color)
        , log_(log)
    {
    }

    qreal position() const
    {
        log_.record();
        return position_;
    }

    QColor color() const
    {
        log_.record();
        return color_;
    }

    void setColor(const QColor& color)
    {
        color_ = color;
        emit colorChanged();
    }

signals:
    void colorChanged();

private:
    qreal position_;
    QColor color_;
    ReadLog& log_;
};

class ProbeGradient : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList stops READ stops NOTIFY stopsChanged)

public:
    ProbeGradient(const QColor& color, ReadLog& log)
        : log_(log)
        , first_(new ProbeStop(0.0, color, log, this))
        , last_(new ProbeStop(1.0, color, log, this))
    {
    }

    QVariantList stops() const
    {
        log_.record();
        return {QVariant::fromValue<QObject*>(first_), QVariant::fromValue<QObject*>(last_)};
    }

    void setColor(const QColor& color)
    {
        first_->setColor(color);
        last_->setColor(color);
        emit stopsChanged();
    }

    // Like a QML GradientStop declared outside its Gradient, which changes without notifying it.
    void setColorSilently(const QColor& color)
    {
        const auto blocker = QSignalBlocker{first_};
        const auto lastBlocker = QSignalBlocker{last_};
        first_->setColor(color);
        last_->setColor(color);
    }

signals:
    void stopsChanged();

private:
    ReadLog& log_;
    ProbeStop* first_;
    ProbeStop* last_;
};

// 100 x 100 px plot showing data in [0, 1] x [0, 1]; the curve runs along pixel row 50.
constexpr auto kSize = 100;

std::unique_ptr<Axis> makeAxis(const Axis::Orientation orientation)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(0.0);
    axis->setViewportMax(1.0);
    return axis;
}

// The curve with a red GradientStroke over a blue GradientFill, shown in a threaded-render-loop window.
class Scene {
public:
    Scene()
    {
        window.setColor(Qt::black);
        window.resize(kSize, kSize);
        curve.setParentItem(window.contentItem());
        curve.setXAxis(xAxis.get());
        curve.setYAxis(yAxis.get());
        curve.setPlotRect({0, 0, kSize, kSize});
        curve.setColor(Qt::white);
        curve.setLineWidth(8);
        curve.setAntialiasingEnabled(false);
        stroke->setGradient(&strokeGradient);
        fill->setGradient(&fillGradient);
        fill->setOpacity(1.0);
        auto effects = curve.effects();
        effects.append(&effects, stroke);
        effects.append(&effects, fill);
        curve.setData(QList<QPointF>{{0.0, 0.5}, {1.0, 0.5}});
    }

    bool isSoftware()
    {
        return window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    bool show()
    {
        window.show();
        return QTest::qWaitForWindowExposed(&window);
    }

    QColor linePixel()
    {
        return QColor{window.grabWindow().pixel(50, 50)};
    }

    QColor fillPixel()
    {
        return QColor{window.grabWindow().pixel(50, 80)};
    }

    QQuickWindow window;
    std::unique_ptr<Axis> xAxis = makeAxis(Axis::Horizontal);
    std::unique_ptr<Axis> yAxis = makeAxis(Axis::Vertical);
    ReadLog log;
    ProbeGradient strokeGradient{Qt::red, log};
    ProbeGradient fillGradient{Qt::blue, log};
    LineCurve curve;
    GradientStroke* stroke{new GradientStroke(&curve)};
    GradientFill* fill{new GradientFill(&curve)};
};

} // namespace

class GradientEffectThreadsTest : public QObject {
    Q_OBJECT

private slots:
    void gradientsAreReadOnGuiThreadOnly();
    void unsignalledStopChangesAreRendered();
    void effectChangesAreRendered();
};

void GradientEffectThreadsTest::gradientsAreReadOnGuiThreadOnly()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    QVERIFY(scene.show());
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::red});
    QCOMPARE(scene.fillPixel(), QColor{Qt::blue});

    scene.strokeGradient.setColor(Qt::green);
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::green});
    scene.fillGradient.setColor(Qt::yellow);
    QTRY_COMPARE(scene.fillPixel(), QColor{Qt::yellow});

    QCOMPARE(scene.log.threads(), QSet<QThread*>{qApp->thread()});
}

void GradientEffectThreadsTest::unsignalledStopChangesAreRendered()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    QVERIFY(scene.show());
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::red});

    scene.strokeGradient.setColorSilently(Qt::green);
    scene.window.update();
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::green});
}

void GradientEffectThreadsTest::effectChangesAreRendered()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    QVERIFY(scene.show());
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::red});

    scene.stroke->setEnabled(false);
    QTRY_COMPARE(scene.linePixel(), QColor{Qt::white});
    QCOMPARE(scene.fillPixel(), QColor{Qt::blue});

    auto effects = scene.curve.effects();
    effects.clear(&effects);
    QTRY_COMPARE(scene.fillPixel(), QColor{Qt::black});
    QCOMPARE(scene.linePixel(), QColor{Qt::white});
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // The threaded render loop runs updatePaintNode() off the GUI thread, which this test checks against.
    qputenv("QSG_RENDER_LOOP", "threaded");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::GradientEffectThreadsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_gradient_effect_threads.moc"
