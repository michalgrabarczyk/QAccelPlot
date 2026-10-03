//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QQuickItem>

namespace QAccelPlot {

class OutlinedRectangle;
class SelectionTool;

/// \brief Draws the gesture or the selected region of a SelectionTool, clipped to the plot area.
///
/// The item lives in the overlay of the tool's plot, below the overlay's other children, and is
/// owned by the tool.
class SelectionRectangle final : public QQuickItem {
public:
    explicit SelectionRectangle(SelectionTool& tool);

private:
    void attach();
    void sync();

    SelectionTool& tool_;
    OutlinedRectangle* rectangle_;
};

} // namespace QAccelPlot
