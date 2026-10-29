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

/// \brief Item that fills its bounds and outlines them inside its edges.
class OutlinedRectangle : public QQuickItem {
public:
    explicit OutlinedRectangle(QQuickItem* parent = nullptr);

    /// \brief Sets the \a fill color and an outline of \a borderColor, \a borderWidth logical pixels wide.
    void setStyle(const QColor& fill, const QColor& borderColor, qreal borderWidth);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    QColor fill_;
    QColor border_;
    qreal borderWidth_{0.0};
};

} // namespace QAccelPlot
