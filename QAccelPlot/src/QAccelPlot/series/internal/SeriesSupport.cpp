//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QQuickWindow>
#include <QSGRendererInterface>

// QRhi is semi-public from Qt 6.6, but its header is only on the include path through the
// private Qt modules, so querying the texture limit needs both.
#if QACCELPLOT_USE_QT_PRIVATE_API && QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#define QACCELPLOT_CAN_QUERY_RHI 1
#include <rhi/qrhi.h>
#else
#define QACCELPLOT_CAN_QUERY_RHI 0
#endif

#include <algorithm>

namespace QAccelPlot::Internal {

namespace {

constexpr auto kFallbackMaxTextureSize = 8192;

int gpuMaxTextureSize(QQuickWindow* window)
{
#if QACCELPLOT_CAN_QUERY_RHI
    if (auto* rendererInterface = window ? window->rendererInterface() : nullptr) {
        const auto* rhi = static_cast<QRhi*>(rendererInterface->getResource(window, QSGRendererInterface::RhiResource));
        if (rhi) {
            return rhi->resourceLimit(QRhi::TextureSizeMax);
        }
    }
#else
    Q_UNUSED(window)
#endif
    return kFallbackMaxTextureSize;
}

} // namespace

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

int maxTextureSize(QQuickWindow* window)
{
    static const auto overrideSize = qEnvironmentVariableIntValue("QACCELPLOT_MAX_TEXTURE_SIZE");
    const auto gpuSize = gpuMaxTextureSize(window);
    return overrideSize > 0 ? std::min(overrideSize, gpuSize) : gpuSize;
}

} // namespace QAccelPlot::Internal
