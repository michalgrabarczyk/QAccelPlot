//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/GradientLineMaterial.hpp"
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"

#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <memory>

namespace {
class TrackedRoot final : public QSGNode {
public:
    explicit TrackedRoot(bool& destroyed)
        : destroyed_(destroyed)
    {
    }

    ~TrackedRoot() override
    {
        destroyed_ = true;
    }

private:
    bool& destroyed_;
};
}

class LineCurveLineRendererTest : public QObject {
    Q_OBJECT
private slots:
    void gradientChangesPreserveOwnedRoot();
};

void LineCurveLineRendererTest::gradientChangesPreserveOwnedRoot()
{
    using namespace QAccelPlot;
    const auto data = std::vector<float>{0, 0, 1, 1};
    auto gradient = GradientColorPayload{};
    const auto fill = GradientFillPayload{};
    const auto params = LineCurveRenderParams{nullptr, data, {data.data(), nullptr}, 2, false, Qt::red, false, 1, {0, 0}, {1, 1}, {100, 100}, nullptr, nullptr,
        false, false, true, 1, gradient, fill, nullptr, nullptr};
    const auto renderer = LineCurveLineRenderer{};
    auto initial = std::unique_ptr<QSGNode>{renderer.paint(nullptr, params)};
    auto destroyed = false;
    auto* tracked = new TrackedRoot{destroyed};
    while (auto* child = initial->firstChild()) {
        initial->removeChildNode(child);
        tracked->appendChildNode(child);
    }
    auto root = std::unique_ptr<QSGNode>{tracked};
    gradient.stops = {{0, Qt::red}, {1, Qt::blue}};
    gradient.gradientValueMin = 0;
    gradient.gradientValueMax = 1;

    for (const auto enabled : {true, false, true}) {
        gradient.enabled = enabled;
        root.reset(renderer.paint(root.release(), params));
        QVERIFY(!destroyed);
        QCOMPARE(root.get(), tracked);
        auto* stroke = static_cast<QSGGeometryNode*>(root->lastChild());
        QCOMPARE(dynamic_cast<GradientLineMaterial*>(stroke->material()) != nullptr, enabled);
        QCOMPARE(root->childCount(), 2);
    }
}

QTEST_MAIN(LineCurveLineRendererTest)
#include "tst_line_curve_line_renderer.moc"
