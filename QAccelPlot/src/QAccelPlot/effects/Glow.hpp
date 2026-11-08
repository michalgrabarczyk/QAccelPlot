//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/SeriesEffect.hpp"

#include <QColor>

namespace QAccelPlot {

namespace Internal {
struct GlowStyle;
}

/// \brief A soft halo around the lines and markers of a series.
///
/// The halo fades out over \c radius pixels outside the drawn shape. It is drawn in the same pass
/// as the series, with no offscreen blur.
///
/// | Series | Glows around |
/// |---|---|
/// | \c LineCurve | The line and its markers |
/// | \c PointCloud | The markers |
/// | \c BarSeries | The bars |
/// | \c ErrorBarSeries | The error bars |
///
/// \sa SeriesEffect, FadeTrail
class Glow : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(Glow)

    /// \brief Color of the halo. Default: an invalid color, which uses the series color.
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)
    /// \brief Width of the halo in pixels, clamped to at least 0. Default: 8.
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    /// \brief Opacity of the halo next to the shape, clamped to [0, 1]. Default: 0.5.
    Q_PROPERTY(qreal intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    /// \brief Constructs a Glow with the given \a parent.
    explicit Glow(QObject* parent = nullptr);

    /// \brief Returns the halo color, or an invalid color when the series color applies.
    QColor color() const;
    /// \brief Sets the halo color to \a color; an invalid color uses the series color.
    void setColor(const QColor& color);
    /// \brief Uses the series color for the halo again.
    void resetColor();

    /// \brief Returns the halo width in pixels.
    qreal radius() const;
    /// \brief Sets the halo width to \a radius pixels, clamped to at least 0.
    void setRadius(qreal radius);

    /// \brief Returns the halo opacity next to the shape.
    qreal intensity() const;
    /// \brief Sets the halo opacity next to the shape to \a intensity, clamped to [0, 1].
    void setIntensity(qreal intensity);

    /// \cond INTERNAL
    /// \brief Returns how the shaders draw the halo of a series drawn in \a seriesColor.
    Internal::GlowStyle style(const QColor& seriesColor) const;
    /// \endcond

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the radius property changes.
    void radiusChanged();
    /// \brief Emitted when the intensity property changes.
    void intensityChanged();

private:
    QColor color_;
    qreal radius_{8.0};
    qreal intensity_{0.5};
};

} // namespace QAccelPlot
