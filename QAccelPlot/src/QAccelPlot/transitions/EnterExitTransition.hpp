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

/// \brief Abstract base class for transitions that bring items in or take them out.
///
/// Assign one to a series' \c enterTransition to animate added items, or to its \c exitTransition
/// to animate removed items; an exit plays the movement backwards, while opacity animations play
/// as written. Assigned to \c updateTransition, it redraws all new items as one sequence when a
/// kept item changes, and the enter and exit transitions are skipped.
///
/// \sa DrawTransition, GrowTransition
class EnterExitTransition : public DataTransition {
    Q_OBJECT
    QML_ANONYMOUS

private:
    // Only the transitions of the library derive from EnterExitTransition.
    friend class DrawTransition;
    friend class GrowTransition;

    explicit EnterExitTransition(QObject* parent = nullptr);
};

} // namespace QAccelPlot
