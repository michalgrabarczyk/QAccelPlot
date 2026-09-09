//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"

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
/// \sa LineCurve, RectangleList, QAccelPlot
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
    enum class LegendSymbol { Line, Fill };
    Q_ENUM(LegendSymbol)

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
    /// \brief Clears cached extents after a series has been emptied.
    void clearDataRanges();

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
