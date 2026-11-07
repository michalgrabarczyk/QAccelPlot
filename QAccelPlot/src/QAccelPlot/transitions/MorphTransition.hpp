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

/// \brief An update transition that interpolates each kept item from its old values to its new ones.
///
/// Assign it to a series' \c updateTransition. Items are matched by index. Non-finite values are
/// never interpolated: an invalid target value is applied immediately, and a valid target value
/// replaces an invalid source value without animation.
///
/// \sa DrawTransition, GrowTransition, DataTransition
class MorphTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(MorphTransition)

public:
    /// \brief Constructs a MorphTransition with the given \a parent.
    explicit MorphTransition(QObject* parent = nullptr);

    /// \brief Returns the value between \a from and \a to at \a easedProgress, honoring the invalid-data-point contract.
    static double interpolateCoordinate(double from, double to, double easedProgress);

    /// \cond INTERNAL
    Internal::TransitionKind kind() const override;
    /// \endcond
};

} // namespace QAccelPlot
