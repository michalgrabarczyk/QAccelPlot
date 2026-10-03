//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QQuickItem>

namespace QAccelPlot {

/// \brief Item that fills its bounds and outlines them with a line of one logical pixel.
class OutlinedRectangle : public QQuickItem {
public:
    explicit OutlinedRectangle(QQuickItem* parent = nullptr);

    /// \brief Sets the \a fill and \a border colors.
    void setColors(const QColor& fill, const QColor& border);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    QColor fill_;
    QColor border_;
};

} // namespace QAccelPlot
