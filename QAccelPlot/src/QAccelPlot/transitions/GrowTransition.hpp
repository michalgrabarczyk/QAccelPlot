//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/EnterExitTransition.hpp"

namespace QAccelPlot {

/// \brief A transition that grows item values from a baseline.
///
/// As an \c enterTransition, the added items grow from \c baseline to their values; as an
/// \c exitTransition, the removed items shrink back to it. Only values grow; positions stay:
///
/// | Series | Grows |
/// |---|---|
/// | \c LineCurve, \c PointCloud | Y |
/// | \c BarSeries | The bar value |
/// | \c ErrorBarSeries | The center and both bounds |
///
/// With \c stagger, the items grow one after another in a wave.
///
/// \sa DrawTransition, MorphTransition, EnterExitTransition
class GrowTransition : public EnterExitTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(GrowTransition)

    /// \brief Value the items grow from, in data units.
    ///
    /// Unset by default, after a reset, or when NaN: \c BarSeries::baselineValue for bars, 0 for other
    /// series. A default the value axis cannot place, infinite or at or below 0 on a log axis, is
    /// replaced by the nearest viewport edge.
    Q_PROPERTY(qreal baseline READ baseline WRITE setBaseline RESET resetBaseline NOTIFY baselineChanged)

public:
    /// \brief Constructs a GrowTransition with the given \a parent.
    explicit GrowTransition(QObject* parent = nullptr);

    /// \brief Returns the baseline, or NaN when the series baseline applies.
    qreal baseline() const;
    /// \brief Sets the baseline to \a baseline; NaN applies the series baseline.
    void setBaseline(qreal baseline);
    /// \brief Applies the series baseline again.
    void resetBaseline();

signals:
    /// \brief Emitted when the baseline property changes.
    void baselineChanged();

private:
    qreal baseline_;
};

} // namespace QAccelPlot
