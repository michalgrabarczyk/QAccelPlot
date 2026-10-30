//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"
#include "QAccelPlot/series/MarkerShape.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QColor>
#include <QObject>

namespace QAccelPlot {

/// \brief How a legend draws one series: its symbol, color, line, and marker.
///
/// Returned by \c PlotSeries::legendEntry() and exposed by \c LegendModel as the \c entry role.
/// The line fields apply to the \c Line symbol and the marker fields to \c Line and \c Marker.
///
/// \sa LegendModel, PlotSeries::legendEntry()
struct LegendEntry {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief Symbol style.
    Q_PROPERTY(::QAccelPlot::PlotSeries::LegendSymbol symbol MEMBER symbol CONSTANT)
    /// \brief Color of the line, marker, or fill.
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
    /// \brief Line width in pixels.
    Q_PROPERTY(qreal lineWidth MEMBER lineWidth CONSTANT)
    /// \brief Line style, or null for a solid line. Owned by the series.
    Q_PROPERTY(::QAccelPlot::LineStyle* lineStyle MEMBER lineStyle CONSTANT)
    /// \brief Marker shape; \c None draws no marker.
    Q_PROPERTY(::QAccelPlot::MarkerShape markerShape MEMBER markerShape CONSTANT)
    /// \brief Marker size in pixels.
    Q_PROPERTY(qreal markerSize MEMBER markerSize CONSTANT)
    /// \brief Whether the marker is filled.
    Q_PROPERTY(bool markerFilled MEMBER markerFilled CONSTANT)
    /// \brief Outline width in pixels of a hollow marker.
    Q_PROPERTY(qreal markerStrokeWidth MEMBER markerStrokeWidth CONSTANT)
    /// \brief Whether the line and marker are antialiased.
    Q_PROPERTY(bool antialiasing MEMBER antialiasing CONSTANT)
    /// \brief Antialiasing feather width in pixels.
    Q_PROPERTY(qreal antialiasingFeather MEMBER antialiasingFeather CONSTANT)

public:
    PlotSeries::LegendSymbol symbol{PlotSeries::LegendSymbol::Line}; ///< \brief Symbol style.
    QColor color;                                                    ///< \brief Line, marker, or fill color.
    qreal lineWidth{1.0};                                            ///< \brief Line width in pixels.
    LineStyle* lineStyle{nullptr};                                   ///< \brief Line style, or null for solid.
    MarkerShape markerShape{MarkerShape::None};                      ///< \brief Marker shape.
    qreal markerSize{0.0};                                           ///< \brief Marker size in pixels.
    bool markerFilled{true};                                         ///< \brief Whether the marker is filled.
    qreal markerStrokeWidth{1.0};                                    ///< \brief Hollow marker outline width.
    bool antialiasing{true};                                         ///< \brief Whether antialiasing is on.
    qreal antialiasingFeather{1.0};                                  ///< \brief Antialiasing feather width.
};

} // namespace QAccelPlot
