//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QList>
#include <QMetaObject>
#include <QPointer>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace QAccelPlot {

/// \brief Base class for series that color their data points by value through a \c Colormap.
///
/// Owns the \c colormap assignment and resolves the value range it maps: \c Colormap::min and
/// \c Colormap::max where they are set, and otherwise the smallest and largest color value the
/// colormap can map: finite, and under \c Colormap::logScale positive. \c ColorBar shows the colormap and the resolved range of any such series.
///
/// \sa PointCloud, HeatmapSeries, BarSeries, ColorBar, Colormap
class ColormappedSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(ColormappedSeries)
    QML_UNCREATABLE("ColormappedSeries is a base class for series colored by value.")

    /// \brief Maps values to colors. What a series draws without one is described by the series.
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    /// \brief Read-only: lower bound of the value range, taken from \c Colormap::min when it is set
    /// and from the data otherwise. \c ColorBar labels its ticks from this and \c dataValueMax.
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    /// \brief Read-only: upper bound of the value range, taken from \c Colormap::max when it is set
    /// and from the data otherwise.
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    /// \brief Returns the colormap, or \c nullptr when none is assigned.
    Colormap* colormap() const;
    /// \brief Sets the colormap to \a colormap.
    void setColormap(Colormap* colormap);

    /// \brief Returns the resolved lower bound of the value range.
    qreal dataValueMin() const;
    /// \brief Returns the resolved upper bound of the value range.
    qreal dataValueMax() const;

signals:
    /// \brief Emitted when the colormap property changes.
    void colormapChanged();
    /// \brief Emitted when \c dataValueMin or \c dataValueMax changes.
    void valueRangeChanged();

protected:
    /// \brief Smallest and largest color value the colormap can map, which unset bounds resolve to.
    ///
    /// \c include() drops a value the colormap cannot map: a non-finite one, and under
    /// \c Colormap::logScale one below the smallest positive normal float. The shaders take the
    /// logarithm in single precision, and GPUs without subnormal support read subnormals as zero.
    class ColorValueRange {
    public:
        /// \brief Creates an empty range for a colormap that is logarithmic when \a logScale is set.
        explicit ColorValueRange(bool logScale);

        /// \brief Widens the range to \a value when the colormap can map it.
        ///
        /// Defined inline: a series calls it once per data point.
        void include(const double value)
        {
            if (!std::isfinite(value) || value < lowest_) {
                return;
            }
            min_ = std::min(min_, value);
            max_ = std::max(max_, value);
        }

    private:
        friend class ColormappedSeries;

        double lowest_;
        double min_{std::numeric_limits<double>::max()};
        double max_{std::numeric_limits<double>::lowest()};
    };

    /// \brief Creates a series whose data points have \a layout; \a wording names them in warnings.
    ColormappedSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent);

    /// \brief Passes every color value of the series to \a range. Default: none.
    ///
    /// Called by \c updateValueRange() only when the colormap leaves a bound unset. A series
    /// passes its values unfiltered; \a range keeps the ones the colormap can map.
    virtual void collectColorValues(ColorValueRange& range) const;
    /// \brief Resolves the value range again. Call after every change of the values.
    ///
    /// Emits \c valueRangeChanged() and schedules a repaint when the range changed.
    void updateValueRange();
    /// \brief Returns the ramp stops of the colormap, empty without one.
    const std::vector<GradientStopData>& colorStops() const;

private:
    Q_SLOT void onColormapUpdated();
    void reconnectColormapSignals();
    void refreshColorStops();

    QPointer<Colormap> colormap_;
    std::vector<GradientStopData> colorStops_;
    qreal dataValueMin_{0.0};
    qreal dataValueMax_{1.0};
    QList<QMetaObject::Connection> colormapConnections_;
};

} // namespace QAccelPlot
