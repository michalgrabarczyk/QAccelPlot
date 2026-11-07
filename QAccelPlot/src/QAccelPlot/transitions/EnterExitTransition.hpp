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
/// to animate removed items; an exit plays the transition backwards. Assigned to \c updateTransition,
/// it redraws all new items as one sequence, and the enter and exit transitions are skipped.
///
/// \sa DrawTransition, GrowTransition
class EnterExitTransition : public DataTransition {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Whether items fade in as they enter, and out as they exit. Default: \c false.
    ///
    /// Each item fades over its own share of the duration, so a fade needs a \c stagger below 1.
    Q_PROPERTY(bool fade READ fade WRITE setFade NOTIFY fadeChanged)

public:
    /// \brief Returns \c true if items fade as they enter and exit.
    bool fade() const;
    /// \brief Sets whether items fade as they enter and exit to \a fade.
    void setFade(bool fade);

signals:
    /// \brief Emitted when the fade property changes.
    void fadeChanged();

private:
    // Only the transitions of the library derive from EnterExitTransition.
    friend class DrawTransition;
    friend class GrowTransition;

    explicit EnterExitTransition(QObject* parent = nullptr);

    bool fade_{false};
};

} // namespace QAccelPlot
