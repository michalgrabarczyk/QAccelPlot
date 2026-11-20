//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/internal/ResettableValue.hpp"
#include "QAccelPlot/linestyles/LineStyle.hpp"

#include <QColor>
#include <QObject>
#include <QPointer>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

/// \brief Controls the lines a \c BandSeries draws along its lower and upper edges.
///
/// Accessible via the \c edges CONSTANT grouped property of \c BandSeries, for example
/// <tt>edges.width: 1</tt>.
///
/// \sa BandSeries
class BandEdges : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Edge line width in pixels, clamped to at least 0. Default: 0, no edge lines.
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    /// \brief Edge line color. Default: the band's \c color at full opacity. Assign \c undefined to return to it.
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)
    /// \brief Edge line style (SolidLine, DashLine, or NoLine). Default: SolidLine.
    Q_PROPERTY(LineStyle* lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)

public:
    /// \brief Constructs a BandEdges owned by \a parent.
    explicit BandEdges(QObject* parent = nullptr);

    /// \brief Returns the edge line width in pixels.
    qreal width() const;
    /// \brief Sets the edge line width to \a width pixels. Negative values are clamped to 0.
    void setWidth(qreal width);

    /// \brief Returns the edge line color in effect.
    QColor color() const;
    /// \brief Sets the edge line color to \a color. An invalid color restores the default.
    void setColor(const QColor& color);
    /// \brief Restores the default edge line color.
    void resetColor();
    /// \brief Sets the color used while none is set to \a color. Called by the owning BandSeries.
    void setDefaultColor(const QColor& color);

    /// \brief Returns the edge line style, or \c nullptr after the assigned style is destroyed.
    LineStyle* lineStyle() const;
    /// \brief Sets the edge line style to \a style. The style is not owned.
    void setLineStyle(LineStyle* style);

signals:
    /// \brief Emitted when the width property changes.
    void widthChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the lineStyle property changes.
    void lineStyleChanged();

private:
    void onLineStyleDestroyed();

    qreal width_{0.0};
    Internal::ResettableValue<QColor> color_;
    QPointer<LineStyle> lineStyle_;
};

} // namespace QAccelPlot
