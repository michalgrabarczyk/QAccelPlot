//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "transitions/DataTransition.hpp"

namespace QAccelPlot {

/// \brief An animation transition that reveals the target curve by drawing it point-by-point from start to end.
///
/// Assign to \c LineCurve::transition to animate data updates with a progressive draw-in effect.
///
/// \sa MorphTransition, DataTransition
class DrawTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(DrawTransition)

public:
    /// \brief Constructs a DrawTransition with the given \a parent.
    explicit DrawTransition(QObject* parent = nullptr);

protected:
    /// \brief Advances the transition by progressively revealing the target curve up to \a easedProgress (0–1).
    void interpolate(float easedProgress, const std::vector<float>& fromData, int fromPointCount, const std::vector<float>& toData, int toPointCount,
        std::vector<float>& outData, int& outPointCount) override;
};

} // namespace QAccelPlot
