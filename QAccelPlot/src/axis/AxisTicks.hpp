//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QList>
#include <QString>

namespace QAccelPlot {

/// \brief A single major tick: its data-space value and its formatted label.
struct AxisTick {
    qreal value{0.0}; ///< \brief Data-space tick value.
    QString label;    ///< \brief Formatted tick label.
};

/// \brief The visible tick and subtick values, with formatted labels, for one axis viewport.
///
/// Holds data-space values and label text only; no pixel geometry. Placement (padding, font metrics,
/// rotation, value-to-pixel mapping) is resolved in \c Axis::paint().
///
/// Built on the GUI thread by \c AxisTickPainter::computeTicks(), because label formatting may call
/// into the QML/JS engine, and consumed by \c Axis::paint(), which runs on the render thread with the
/// threaded render loop.
struct AxisTicks {
    QList<qreal> subtickValues; ///< \brief Data-space values of the visible subticks.
    QList<AxisTick> majorTicks; ///< \brief Visible major ticks with their labels.
};

} // namespace QAccelPlot
