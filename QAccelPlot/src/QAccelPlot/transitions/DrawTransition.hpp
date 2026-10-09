//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/DataTransition.hpp"

namespace QAccelPlot {

/// \brief An animation transition that reveals the target data point by point from start to end.
///
/// Assign to \c LineCurve::transition to draw the new curve in, or to \c BarSeries::transition to
/// show the new bars one after another.
///
/// \sa MorphTransition, DataTransition
class DrawTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(DrawTransition)

public:
    /// \brief Constructs a DrawTransition with the given \a parent.
    explicit DrawTransition(QObject* parent = nullptr);

protected:
    /// \brief Writes the first points of \a to into \a out: their share is \a easedProgress, and at least one is kept.
    void interpolate(double easedProgress, const Dataset& from, const Dataset& to, Dataset& out) override;
};

} // namespace QAccelPlot
