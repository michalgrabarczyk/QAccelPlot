//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <QList>
#include <QStringList>

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QAccelPlot::Internal {

/// \brief Reads the Qt Quick animations of \c "opacity" among \a animations, which run together, as one timeline.
///
/// The animations are read, never run. \c PropertyAnimation and its numeric subtypes,
/// \c PropertyAction, \c PauseAnimation, \c SequentialAnimation, and \c ParallelAnimation are
/// supported, with \c loops. A missing \c from continues from the opacity reached so far, and a
/// missing \c to is 1. Other animations, and animations of other properties, add only their duration.
///
/// \param problems When not null, receives one message for each part of \a animations that is ignored.
OpacityTimeline readOpacityAnimations(const QList<QObject*>& animations, QStringList* problems = nullptr);

} // namespace QAccelPlot::Internal
