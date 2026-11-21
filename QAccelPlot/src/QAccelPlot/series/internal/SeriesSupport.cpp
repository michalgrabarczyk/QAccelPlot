//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/internal/MaxTextureSize.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"

#include <QQuickWindow>
#include <QSGRendererInterface>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot::Internal {

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

bool supportsCustomShaderRendering(const QQuickWindow* window)
{
    const auto* rendererInterface = window ? window->rendererInterface() : nullptr;
    return rendererInterface && rendererInterface->graphicsApi() != QSGRendererInterface::Software;
}

int recordCapacity(QQuickWindow* window, const int floatsPerRecord, const int vertexBytesPerRecord)
{
    constexpr auto kMaxExactVertexId = qint64{1} << 24;
    const auto textureRecords = dataTextureItemCapacity(maxTextureSize(window), floatsPerRecord);
    const auto geometryRecords = qint64{std::numeric_limits<int>::max() / vertexBytesPerRecord};
    return static_cast<int>(std::min({textureRecords, geometryRecords, kMaxExactVertexId}));
}

qreal clampAntialiasingFeather(const qreal feather)
{
    constexpr auto kMinFeather = qreal{0.0};
    constexpr auto kMaxFeather = qreal{10.0};
    return std::clamp(feather, kMinFeather, kMaxFeather);
}

bool assignNonNegative(qreal& member, const qreal requested)
{
    const auto clamped = std::max(requested, qreal{0.0});
    if (!std::isfinite(clamped) || nearlyEqual(member, clamped)) {
        return false;
    }
    member = clamped;
    return true;
}

GrowViewport growViewport(const Axis* valueAxis)
{
    if (!valueAxis) {
        return {};
    }
    const auto low = std::min(valueAxis->viewportMin(), valueAxis->viewportMax());
    const auto high = std::max(valueAxis->viewportMin(), valueAxis->viewportMax());
    return {low, high, valueAxis->logScale()};
}

double growBaseline(const double baseline, const Axis* valueAxis)
{
    return placeGrowBaseline(baseline, growViewport(valueAxis));
}

} // namespace QAccelPlot::Internal
