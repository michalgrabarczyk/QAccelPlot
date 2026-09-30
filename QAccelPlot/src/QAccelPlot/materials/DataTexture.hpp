//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QImage>
#include <QSGTexture>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

/// \brief Series data uploaded to the GPU as an RGBA8888 texture, one float per texel.
///
/// Materials hold it through a \c std::shared_ptr, so several nodes of one series can sample the
/// same upload. It is created and destroyed on the render thread, like the materials holding it.
class DataTexture {
public:
    /// \brief Constructs an empty data texture; \c texture() stays \c nullptr until the first upload.
    DataTexture() = default;
    /// \brief Wraps the existing \a texture, for example in tests.
    explicit DataTexture(std::unique_ptr<QSGTexture> texture);

    /// \brief Uploads \a floatCount raw floats from \a data, reusing GPU and CPU buffers where possible.
    ///
    /// Does nothing when \a window is \c nullptr or \a floatCount is not positive.
    void upload(QQuickWindow* window, const float* data, int floatCount);

    /// \brief Returns the texture the shaders sample, or \c nullptr before the first successful upload.
    QSGTexture* texture() const;

private:
    std::unique_ptr<QSGTexture> texture_;
    QImage imageBuffer_;
    bool warnedAboutTextureSize_{false};
};

} // namespace QAccelPlot
