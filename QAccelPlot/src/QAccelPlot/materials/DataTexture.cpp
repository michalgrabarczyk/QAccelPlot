//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/internal/MaxTextureSize.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/materials/internal/DataTextureUpload.hpp"
#include "QAccelPlot/materials/internal/DataTextureUploadTotals.hpp"

#include <QColorSpace>

#include <cstring>
#include <utility>

namespace QAccelPlot {

DataTexture::DataTexture(std::unique_ptr<QSGTexture> texture)
    : texture_(std::move(texture))
{
}

void DataTexture::upload(QQuickWindow* window, const float* data, const int floatCount)
{
    if (!window || floatCount <= 0) {
        return;
    }

    const auto maxSize = Internal::maxTextureSize(window);
    const auto texWidth = Internal::dataTextureWidth(maxSize);
    const auto texHeight = Internal::dataTextureHeight(floatCount, texWidth);
    if (texHeight > maxSize && !warnedAboutTextureSize_) {
        warnedAboutTextureSize_ = true;
        qCWarning(lcQAccelPlot) << "DataTexture: data texture height" << texHeight << "(for" << floatCount << "floats) exceeds the GPU limit of" << maxSize
                                << "; the series may render incorrectly. Reduce the number of points or rectangles.";
    }

    // Reuse QImage storage across frames. The upload helper either updates the
    // scene-graph texture in place or recreates it through public Qt API,
    // depending on the configured build mode.
    const auto dimensionsChanged = imageBuffer_.width() != texWidth || imageBuffer_.height() != texHeight;
    if (dimensionsChanged) {
        imageBuffer_ = QImage(texWidth, texHeight, QImage::Format_RGBA8888_Premultiplied);
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
    Internal::recordDataTextureUpload(static_cast<std::int64_t>(dataBytes));
}

QSGTexture* DataTexture::texture() const
{
    return texture_.get();
}

} // namespace QAccelPlot
