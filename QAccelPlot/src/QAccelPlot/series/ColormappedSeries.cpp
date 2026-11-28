//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/ColormappedSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"

#include <cmath>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

} // namespace

Colormap* ColormappedSeries::colormap() const
{
    return colormap_.data();
}

void ColormappedSeries::setColormap(Colormap* colormap)
{
    if (colormap_ == colormap) {
        return;
    }
    colormap_ = colormap;
    reconnectColormapSignals();
    refreshColorStops();
    updateValueRange();
    emit colormapChanged();
    update();
}

qreal ColormappedSeries::dataValueMin() const
{
    return dataValueMin_;
}

qreal ColormappedSeries::dataValueMax() const
{
    return dataValueMax_;
}

ColormappedSeries::ColorValueRange::ColorValueRange(const bool logScale)
    : lowest_{logScale ? static_cast<double>(std::numeric_limits<float>::min()) : std::numeric_limits<double>::lowest()}
{
}

ColormappedSeries::ColormappedSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent)
    : PlotSeries(std::move(layout), std::move(wording), parent)
{
}

void ColormappedSeries::collectColorValues([[maybe_unused]] ColorValueRange& range) const
{
}

void ColormappedSeries::updateValueRange()
{
    const auto fixedMin = colormap_ ? colormap_->min() : kNaN;
    const auto fixedMax = colormap_ ? colormap_->max() : kNaN;
    auto range = ColorValueRange{colormap_ && colormap_->logScale()};
    if (std::isnan(fixedMin) || std::isnan(fixedMax)) {
        collectColorValues(range);
    }
    const auto found = range.min_ <= range.max_;
    const auto resolvedMin = std::isnan(fixedMin) ? (found ? range.min_ : 0.0) : fixedMin;
    const auto resolvedMax = std::isnan(fixedMax) ? (found ? range.max_ : 1.0) : fixedMax;
    if (nearlyEqual(resolvedMin, dataValueMin_) && nearlyEqual(resolvedMax, dataValueMax_)) {
        return;
    }
    dataValueMin_ = resolvedMin;
    dataValueMax_ = resolvedMax;
    emit valueRangeChanged();
    update();
}

const std::vector<GradientStopData>& ColormappedSeries::colorStops() const
{
    return colorStops_;
}

void ColormappedSeries::onColormapUpdated()
{
    refreshColorStops();
    updateValueRange();
    update();
}

void ColormappedSeries::reconnectColormapSignals()
{
    for (const auto& connection : colormapConnections_) {
        disconnect(connection);
    }
    colormapConnections_.clear();
    if (!colormap_) {
        return;
    }

    colormapConnections_.append(connect(colormap_.data(), &Colormap::colormapChanged, this, &ColormappedSeries::onColormapUpdated));
    colormapConnections_.append(connect(colormap_.data(), &QObject::destroyed, this, [this]() {
        colorStops_.clear();
        updateValueRange();
        emit colormapChanged();
        update();
    }));
}

void ColormappedSeries::refreshColorStops()
{
    colorStops_ = colormap_ ? colormap_->resolvedStops() : std::vector<GradientStopData>{};
}

} // namespace QAccelPlot
