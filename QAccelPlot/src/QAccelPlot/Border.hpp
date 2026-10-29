//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Outline color and width, exposed as a \c border grouped property, for example <tt>border.width: 1</tt>.
///
/// Used by \c PlotView, \c RectangleSeries, \c BarSeries, \c PlotView.rectangleZoom, \c SelectionTool, and
/// \c ColorBar. Each owner documents its defaults and where the outline is drawn.
class Border : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Outline color.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Outline width in pixels, clamped to at least 0. 0 draws no outline.
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)

public:
    /// \brief Constructs a border with the given default \a color and \a width, owned by \a parent.
    explicit Border(const QColor& color, qreal width, QObject* parent = nullptr);

    /// \brief Returns the outline color.
    QColor color() const;
    /// \brief Sets the outline color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the outline width in pixels.
    qreal width() const;
    /// \brief Sets the outline width to \a width pixels. Negative values are clamped to 0.
    void setWidth(qreal width);

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the width property changes.
    void widthChanged();

private:
    QColor color_;
    qreal width_;
};

} // namespace QAccelPlot
