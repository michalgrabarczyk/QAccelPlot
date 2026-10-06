//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QHoverEvent>
#include <QPointF>

namespace QAccelPlotTest {

/// \brief Creates a hover event at \a position with the constructor the Qt version provides.
inline QHoverEvent hoverEvent(const QEvent::Type type, const QPointF& position, const QPointF& oldPosition)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    return QHoverEvent{type, position, position, oldPosition};
#else
    // Qt 6.2 has no constructor taking a global position.
    return QHoverEvent{type, position, oldPosition};
#endif
}

} // namespace QAccelPlotTest
