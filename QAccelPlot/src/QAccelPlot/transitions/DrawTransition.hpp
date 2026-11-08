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

/// \brief A transition that reveals items one after another, in index order.
///
/// As an \c enterTransition it draws the added items in; as an \c exitTransition it hides the
/// removed items from the last to the first. A line draws its last revealed segment partway.
///
/// \c stagger defaults to 1: each item appears at once, and \c easing shapes how the reveal moves
/// along the items. An opacity animation fades each item in from the moment the reveal reaches it:
///
/// \code{.qml}
/// QAccelPlot.DrawTransition {
///     NumberAnimation { property: "opacity"; from: 0; duration: 250 }
/// }
/// \endcode
///
/// \sa GrowTransition, MorphTransition, EnterExitTransition
class DrawTransition : public EnterExitTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(DrawTransition)

public:
    /// \brief Constructs a DrawTransition with the given \a parent.
    explicit DrawTransition(QObject* parent = nullptr);

    /// \cond INTERNAL
    Internal::TransitionKind kind() const override;
    /// \endcond
};

} // namespace QAccelPlot
