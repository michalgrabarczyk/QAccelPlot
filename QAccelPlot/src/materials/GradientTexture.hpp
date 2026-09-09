//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"

#include <QImage>
#include <QSGMaterialShader>

#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)

namespace QAccelPlot {

/// \brief Cached one-dimensional texture used by gradient materials.
class GradientTexture {
public:
    GradientTexture() = default;
    ~GradientTexture();

    GradientTexture(const GradientTexture&) = delete;
    GradientTexture& operator=(const GradientTexture&) = delete;

    /// \brief Updates the 256-sample texture when \a stops changed.
    void upload(QQuickWindow* window, const std::vector<GradientStopData>& stops);
    /// \brief Commits the texture to the requested shader sampler binding.
    void commit(QSGMaterialShader::RenderState& state, int binding, QSGTexture** texture);
    /// \brief Returns the scene-graph comparison key for the uploaded texture, or zero when no texture exists.
    qint64 comparisonKey() const;

private:
    bool matches(const std::vector<GradientStopData>& stops) const;

    QImage image_;
    QSGTexture* texture_{nullptr};
    std::vector<GradientStopData> stops_;
};

} // namespace QAccelPlot
