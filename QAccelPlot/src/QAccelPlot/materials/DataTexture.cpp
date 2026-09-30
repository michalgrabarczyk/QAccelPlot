//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/materials/internal/DataTextureUpload.hpp"

#include <QColorSpace>

#include <cstring>
#include <utility>

namespace QAccelPlot {

namespace {

// Conservative per-dimension texture size bound, which the row width also stays within, below
// which virtually every RHI backend Qt Quick supports (Direct3D 11/12, Metal, Vulkan, OpenGL
// 3.3+) is guaranteed to allow texture creation. Actual hardware limits are commonly higher
// (e.g. 16384), but querying the live RHI limit needs QRhi, which isn't available as public API
// in every supported Qt version.
constexpr int kMaxSafeTextureHeight{8192};

} // namespace

DataTexture::DataTexture(std::unique_ptr<QSGTexture> texture)
    : texture_(std::move(texture))
{
}

void DataTexture::upload(QQuickWindow* window, const float* data, const int floatCount)
{
    if (!window || floatCount <= 0) {
        return;
    }

    const auto texHeight = Internal::dataTextureHeight(floatCount);
    if (texHeight > kMaxSafeTextureHeight && !warnedAboutTextureSize_) {
        warnedAboutTextureSize_ = true;
        qCWarning(lcQAccelPlot) << "DataTexture: data texture height" << texHeight << "(for" << floatCount << "floats) exceeds the safe limit of"
                                << kMaxSafeTextureHeight
                                << "; texture creation may fail on some GPUs and the series may render nothing. Reduce the "
                                   "number of points or rectangles.";
    }

    // Reuse QImage storage across frames. The upload helper either updates the
    // scene-graph texture in place or recreates it through public Qt API,
    // depending on the configured build mode.
    const auto dimensionsChanged = imageBuffer_.width() != Internal::kDataTextureWidth || imageBuffer_.height() != texHeight;
    if (dimensionsChanged) {
        imageBuffer_ = QImage(Internal::kDataTextureWidth, texHeight, QImage::Format_RGBA8888_Premultiplied);
        imageBuffer_.setColorSpace(QColorSpace());
    }

    if (imageBuffer_.isNull()) {
        return;
    }

    const auto dataBytes = static_cast<size_t>(floatCount) * sizeof(float);
    memcpy(imageBuffer_.bits(), data, dataBytes);

    if (dimensionsChanged) {
        const auto tailBytes = imageBuffer_.sizeInBytes() - static_cast<qsizetype>(dataBytes);
        if (tailBytes > 0) {
            memset(imageBuffer_.bits() + dataBytes, 0, static_cast<size_t>(tailBytes));
        }
    }

    Internal::uploadDataTexture(texture_, window, imageBuffer_);
}

QSGTexture* DataTexture::texture() const
{
    return texture_.get();
}

} // namespace QAccelPlot
