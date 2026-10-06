//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QCursor>
#include <QRect>
#include <QScreen>
#include <QWindow>

namespace QAccelPlotTest {

/// \brief Moves \a window to a corner of its screen that the mouse cursor is not in.
///
/// A real cursor over a shown window sends hover events that override the ones a test synthesizes.
/// Call before showing the window.
inline void moveAwayFromCursor(QWindow& window)
{
    const auto* screen = window.screen();
    if (!screen) {
        return;
    }
    // Room for the window frame, whose size is unknown before the window is shown.
    constexpr auto kFrameMargin = 40;
    const auto area = screen->availableGeometry();
    const auto cursor = QCursor::pos(screen);
    const auto size = window.size();
    const auto left = area.left() + kFrameMargin;
    const auto top = area.top() + kFrameMargin;
    const auto right = area.right() - size.width() - kFrameMargin;
    const auto bottom = area.bottom() - size.height() - kFrameMargin;
    for (const auto& corner : {QPoint{left, top}, QPoint{right, top}, QPoint{left, bottom}, QPoint{right, bottom}}) {
        const auto occupied = QRect{corner, size}.adjusted(-kFrameMargin, -kFrameMargin, kFrameMargin, kFrameMargin);
        if (!occupied.contains(cursor)) {
            window.setPosition(corner);
            return;
        }
    }
}

} // namespace QAccelPlotTest
