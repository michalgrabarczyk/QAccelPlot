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

/// \brief An animation transition that smoothly interpolates point positions between two datasets.
///
/// Each point is linearly blended from its old position to its new position.
/// When the point counts differ, the extra points start from, or end on, the last point of the
/// shorter dataset. \c BarSeries grows and shrinks such bars at its baseline instead.
/// Non-finite coordinates are never interpolated: an invalid target coordinate is applied immediately,
/// and a valid target coordinate replaces an invalid source coordinate without animation.
///
/// \sa DrawTransition, DataTransition
class MorphTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(MorphTransition)

public:
    /// \brief Constructs a MorphTransition with the given \a parent.
    explicit MorphTransition(QObject* parent = nullptr);

    /// \brief Returns the coordinate between \a from and \a to at \a easedProgress, honoring the invalid-sample contract.
    static double interpolateCoordinate(double from, double to, double easedProgress);

protected:
    /// \brief Interpolates every value of every point between \a from and \a to at \a easedProgress.
    void interpolate(double easedProgress, const Dataset& from, const Dataset& to, Dataset& out) override;
};

} // namespace QAccelPlot
