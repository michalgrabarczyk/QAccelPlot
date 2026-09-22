//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"

#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QString>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <limits>

namespace QAccelPlot {

/// \brief Common QML item contract for data series hosted by \c PlotView.
///
/// PlotSeries owns the integration shared by every plot type: axis bindings,
/// plot-area layout, data-range reporting, and legend metadata. Concrete series
/// remain responsible for their data model, rendering, and hit testing.
///
/// \sa LineCurve, PointCloud, RectangleList, QAccelPlot
class PlotSeries : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotSeries)
    QML_UNCREATABLE("PlotSeries is a base class for concrete plot series.")

    /// \brief Identifying name used by the default legend.
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /// \brief Horizontal axis used for data-to-pixel coordinate mapping.
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    /// \brief Vertical axis used for data-to-pixel coordinate mapping.
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    /// \brief Plot area in parent-item coordinates, assigned by \c PlotView.
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)
    /// \brief Symbol style requested from the default legend.
    Q_PROPERTY(LegendSymbol legendSymbol READ legendSymbol WRITE setLegendSymbol NOTIFY legendSymbolChanged)

public:
    /// \brief Supported default legend symbols.
    ///
    /// \c Line draws the series line style and marker, \c Fill a filled swatch, and \c Marker only
    /// the series marker shape (used by unconnected series such as \c PointCloud).
    enum class LegendSymbol { Line, Fill, Marker };
    Q_ENUM(LegendSymbol)

    /// \brief Marker shapes shared by every series that draws markers.
    ///
    /// Every shape except \c Pixel fits within a square of half-width \c markerSize. The shaders
    /// select a shape by this value minus one, so append new shapes at the end and never reorder
    /// or insert. Series that always draw markers, such as \c PointCloud, do not accept \c None.
    enum class MarkerShape {
        None,          ///< \brief No markers.
        Circle,        ///< \brief Circle.
        Square,        ///< \brief Square.
        Diamond,       ///< \brief Diamond, narrower than it is tall.
        TriangleUp,    ///< \brief Equilateral triangle pointing up.
        TriangleDown,  ///< \brief Equilateral triangle pointing down.
        TriangleLeft,  ///< \brief Equilateral triangle pointing left.
        TriangleRight, ///< \brief Equilateral triangle pointing right.
        Cross,         ///< \brief Plus sign (+).
        XCross,        ///< \brief Diagonal cross (×).
        HLine,         ///< \brief Short horizontal line.
        VLine,         ///< \brief Short vertical line, e.g. for rug and event plots.
        Star,          ///< \brief Five-pointed star.
        Asterisk,      ///< \brief Eight-armed asterisk: a thin plus and a thin diagonal cross.
        Pixel,         ///< \brief A single pixel; ignores \c markerSize, \c markerFilled, and anti-aliasing. Suited to very dense scatter plots.
        Hexagon,       ///< \brief Regular hexagon with a vertex up.
        Pentagon       ///< \brief Regular pentagon with a vertex up.
    };
    Q_ENUM(MarkerShape)

    explicit PlotSeries(QQuickItem* parent = nullptr);

    QString name() const;
    void setName(const QString& name);

    Axis* xAxis() const;
    void setXAxis(Axis* axis);

    Axis* yAxis() const;
    void setYAxis(Axis* axis);

    QRectF plotRect() const;
    /// \brief Updates the series geometry to exactly cover \a rect.
    void setPlotRect(const QRectF& rect);

    LegendSymbol legendSymbol() const;
    void setLegendSymbol(LegendSymbol symbol);

signals:
    void nameChanged();
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void legendSymbolChanged();
    /// \brief Emitted when the X data extent of this series changes.
    void xDataRangeChanged(qreal min, qreal max);
    /// \brief Emitted when the Y data extent of this series changes.
    void yDataRangeChanged(qreal min, qreal max);

protected:
    /// \brief Reports this series' data extents to its bound axes.
    void setDataRanges(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    /// \brief Reports this series' X data extent to its bound horizontal axis. Non-finite extents are ignored.
    void setXDataRange(qreal min, qreal max);
    /// \brief Reports this series' Y data extent to its bound vertical axis. Non-finite extents are ignored.
    void setYDataRange(qreal min, qreal max);
    /// \brief Widens the reported X extent to include \a x.
    ///
    /// Lets an append-style ingestion path update the range in O(1) instead of rescanning
    /// the whole buffer. A non-finite \a x leaves the extent unchanged.
    void extendXDataRange(qreal x);
    /// \brief Widens the reported Y extent to include \a y. A non-finite \a y leaves the extent unchanged.
    void extendYDataRange(qreal y);
    /// \brief Clears cached extents after a series has been emptied.
    void clearDataRanges();
    /// \brief Clears the cached X extent, e.g. when no sample has a valid X coordinate.
    void clearXDataRange();
    /// \brief Clears the cached Y extent, e.g. when no sample has a valid Y coordinate.
    void clearYDataRange();
    /// \brief Called when a bound axis switches between linear and logarithmic scale, or a different axis is bound.
    ///
    /// Log scale changes which samples are valid, so series that apply the invalid-sample contract
    /// override this to refresh ranges and cached geometry. The default implementation does nothing.
    virtual void onAxisScaleChanged();
    /// \brief Returns the plot area to render into, adopting it from the parent plot if needed.
    ///
    /// Normally \c plotRect has already been assigned, and this just returns it. A series
    /// constructed in C++ with the plot as its parent is added before its own constructor runs,
    /// so the plot cannot assign \c plotRect at that point; calling this from \c updatePaintNode
    /// adopts the plot area lazily. Without a parent plot, returns the current size.
    QRectF resolvePlotRect();

private:
    void onAxisRangeChanged();
    void reportXDataRangeToAxis() const;
    void reportYDataRangeToAxis() const;

    QString name_;
    QPointer<Axis> xAxis_;
    QPointer<Axis> yAxis_;
    QRectF plotRect_;
    LegendSymbol legendSymbol_{LegendSymbol::Line};
    qreal lastXMin_{std::numeric_limits<qreal>::max()};
    qreal lastXMax_{std::numeric_limits<qreal>::lowest()};
    qreal lastYMin_{std::numeric_limits<qreal>::max()};
    qreal lastYMax_{std::numeric_limits<qreal>::lowest()};
};

} // namespace QAccelPlot
