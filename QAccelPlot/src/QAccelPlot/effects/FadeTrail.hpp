//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/SeriesEffect.hpp"

#include <QEasingCurve>

namespace QAccelPlot {

namespace Internal {
struct ItemTrail;
}

/// \brief Fades the oldest items of a series, like the trace of an oscilloscope.
///
/// Items are ordered by index: the first item is the oldest. The oldest \c length share of the
/// items fades from \c minOpacity at the first item to full opacity, along \c easing; the newer
/// items are drawn as usual. Supported by \c LineCurve, line and markers, and \c PointCloud.
///
/// \sa SeriesEffect, Glow
class FadeTrail : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(FadeTrail)

    /// \brief Share of the items that fades, counted from the oldest, clamped to [0, 1]. Default: 0.5.
    Q_PROPERTY(qreal length READ length WRITE setLength NOTIFY lengthChanged)
    /// \brief Opacity of the oldest item, clamped to [0, 1]. Default: 0.
    Q_PROPERTY(qreal minOpacity READ minOpacity WRITE setMinOpacity NOTIFY minOpacityChanged)
    /// \brief How the opacity rises along the fading items, from the oldest. Default: \c QEasingCurve::InQuad.
    ///
    /// \c InQuad keeps most of the trail dim, like the decaying trace of an oscilloscope;
    /// \c QEasingCurve::Linear rises evenly. Values outside [0, 1] are clamped.
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)

public:
    /// \brief Constructs a FadeTrail with the given \a parent.
    explicit FadeTrail(QObject* parent = nullptr);

    /// \brief Returns the share of the items that fades.
    qreal length() const;
    /// \brief Sets the share of the items that fades to \a length, clamped to [0, 1].
    void setLength(qreal length);

    /// \brief Returns the opacity of the oldest item.
    qreal minOpacity() const;
    /// \brief Sets the opacity of the oldest item to \a opacity, clamped to [0, 1].
    void setMinOpacity(qreal opacity);

    /// \brief Returns how the opacity rises along the fading items.
    QEasingCurve easing() const;
    /// \brief Sets how the opacity rises along the fading items to \a easing.
    void setEasing(const QEasingCurve& easing);

    /// \cond INTERNAL
    /// \brief Returns how the shaders fade the oldest of \a itemCount drawn items.
    Internal::ItemTrail trail(int itemCount) const;
    /// \endcond

signals:
    /// \brief Emitted when the length property changes.
    void lengthChanged();
    /// \brief Emitted when the minOpacity property changes.
    void minOpacityChanged();
    /// \brief Emitted when the easing property changes.
    void easingChanged();

private:
    qreal length_{0.5};
    qreal minOpacity_{0.0};
    QEasingCurve easing_{QEasingCurve::InQuad};
};

} // namespace QAccelPlot
