//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/Colormap.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/effects/GradientUtils.hpp"

#include <QColor>

#include <algorithm>
#include <array>
#include <limits>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

// 32 control points sampled at even spacing from the matplotlib ramps of the same name. Linear
// interpolation between them stays within about one CIELAB unit of the originals.
using RampSamples = std::vector<const char*>;

const RampSamples& rampFor(const Colormap::Preset preset)
{
    static const auto viridis = RampSamples{"#440154", "#470d60", "#48196b", "#482474", "#472e7c", "#453882", "#414286", "#3e4b89", "#3a548c", "#365d8d",
        "#32658e", "#2e6d8e", "#2b758e", "#287d8e", "#25858e", "#228c8d", "#20948c", "#1e9c89", "#20a386", "#25ab82", "#2db27d", "#39ba76", "#48c16e",
        "#58c765", "#6acd5b", "#7ed34f", "#92d742", "#a8db34", "#bedf26", "#d4e21b", "#e9e41a", "#fde725"};
    static const auto plasma = RampSamples{"#0d0887", "#220690", "#320597", "#40049d", "#4e02a2", "#5b01a5", "#6800a8", "#7501a8", "#8104a7", "#8d0ba5",
        "#9814a0", "#a31d9a", "#ad2693", "#b6308b", "#bf3984", "#c7427c", "#cf4c74", "#d6556d", "#dd5e66", "#e3685f", "#e97258", "#ee7c51", "#f3874a",
        "#f79243", "#fa9d3b", "#fca935", "#fdb52e", "#fdc229", "#fccf25", "#f9dd24", "#f5eb27", "#f0f921"};
    static const auto inferno = RampSamples{"#000004", "#040313", "#0b0725", "#160b39", "#220c4c", "#310a5c", "#3f0a66", "#4d0c6b", "#5a116e", "#67166e",
        "#741b6e", "#811f6c", "#8e2469", "#9b2964", "#a82e5f", "#b53358", "#c13a51", "#cc4248", "#d74b3f", "#e05536", "#e8612c", "#ef6d22", "#f57b17",
        "#f8890c", "#fb9806", "#fca80d", "#fbb81c", "#f9c830", "#f6d847", "#f2e763", "#f3f585", "#fcffa4"};
    static const auto magma = RampSamples{"#000004", "#040312", "#0b0823", "#140e35", "#1e1149", "#2a115d", "#38106d", "#461077", "#54137d", "#601880",
        "#6d1e81", "#7a2382", "#872781", "#942c80", "#a2307e", "#af347b", "#bd3977", "#ca3e72", "#d6456c", "#e24d66", "#ec5860", "#f3655c", "#f8745c",
        "#fb8360", "#fd9366", "#fea26f", "#feb27a", "#fec185", "#fed093", "#fddfa1", "#fceeb0", "#fcfdbf"};
    static const auto grayscale = RampSamples{"#000000", "#ffffff"};
    static const auto rainbow = RampSamples{"#0000ff", "#00ffff", "#00ff00", "#ffff00", "#ff0000"};

    switch (preset) {
    case Colormap::Preset::Plasma:
        return plasma;
    case Colormap::Preset::Inferno:
        return inferno;
    case Colormap::Preset::Magma:
        return magma;
    case Colormap::Preset::Grayscale:
        return grayscale;
    case Colormap::Preset::Rainbow:
        return rainbow;
    case Colormap::Preset::Viridis:
        break;
    }
    return viridis;
}

std::vector<GradientStopData> presetStops(const Colormap::Preset preset)
{
    const auto& samples = rampFor(preset);
    auto stops = std::vector<GradientStopData>{};
    stops.reserve(samples.size());
    const auto lastIndex = static_cast<float>(samples.size() - 1);
    for (auto index = std::size_t{0}; index < samples.size(); ++index) {
        auto stop = GradientStopData{};
        stop.position = static_cast<float>(index) / lastIndex;
        stop.color = QColor{QString::fromLatin1(samples[index])};
        stops.push_back(stop);
    }
    return stops;
}

} // namespace

Colormap::Colormap(QObject* parent)
    : QObject(parent)
    , min_(kNaN)
    , max_(kNaN)
{
    rebuildStops();
}

Colormap::Preset Colormap::preset() const
{
    return preset_;
}

void Colormap::setPreset(const Preset preset)
{
    if (preset_ == preset) {
        return;
    }
    preset_ = preset;
    rebuildStops();
    emit colormapChanged();
}

QQmlListProperty<QObject> Colormap::stops()
{
    return QQmlListProperty<QObject>{this, this, &Colormap::appendStop, &Colormap::stopCount, &Colormap::stopAt, &Colormap::clearStops};
}

void Colormap::appendStop(QQmlListProperty<QObject>* list, QObject* stop)
{
    auto* self = static_cast<Colormap*>(list->data);
    self->stopObjects_.append(stop);
    self->rebuildStops();
    emit self->colormapChanged();
}

qsizetype Colormap::stopCount(QQmlListProperty<QObject>* list)
{
    return static_cast<Colormap*>(list->data)->stopObjects_.count();
}

QObject* Colormap::stopAt(QQmlListProperty<QObject>* list, const qsizetype index)
{
    return static_cast<Colormap*>(list->data)->stopObjects_.value(index);
}

void Colormap::clearStops(QQmlListProperty<QObject>* list)
{
    auto* self = static_cast<Colormap*>(list->data);
    self->stopObjects_.clear();
    self->rebuildStops();
    emit self->colormapChanged();
}

bool Colormap::reversed() const
{
    return reversed_;
}

void Colormap::setReversed(const bool reversed)
{
    if (reversed_ == reversed) {
        return;
    }
    reversed_ = reversed;
    rebuildStops();
    emit colormapChanged();
}

qreal Colormap::min() const
{
    return min_;
}

void Colormap::setMin(const qreal value)
{
    // Both NaN means unset either way, and NaN never compares equal to itself.
    if (nearly_equal(min_, value) || (std::isnan(min_) && std::isnan(value))) {
        return;
    }
    min_ = value;
    emit colormapChanged();
}

qreal Colormap::max() const
{
    return max_;
}

void Colormap::setMax(const qreal value)
{
    if (nearly_equal(max_, value) || (std::isnan(max_) && std::isnan(value))) {
        return;
    }
    max_ = value;
    emit colormapChanged();
}

Colormap::Normalization Colormap::norm() const
{
    return norm_;
}

void Colormap::setNorm(const Normalization norm)
{
    if (norm_ == norm) {
        return;
    }
    norm_ = norm;
    emit colormapChanged();
}

const std::vector<GradientStopData>& Colormap::resolvedStops() const
{
    return resolvedStops_;
}

void Colormap::rebuildStops()
{
    resolvedStops_ = customStops();
    if (resolvedStops_.empty()) {
        resolvedStops_ = presetStops(preset_);
    }
    if (reversed_) {
        std::reverse(resolvedStops_.begin(), resolvedStops_.end());
        for (auto& stop : resolvedStops_) {
            stop.position = 1.0f - stop.position;
        }
    }
}

// A stop list that yields no valid stops is treated as unset, so the preset still supplies a ramp.
std::vector<GradientStopData> Colormap::customStops() const
{
    auto stops = QVariantList{};
    stops.reserve(stopObjects_.size());
    for (auto* stop : stopObjects_) {
        stops.append(QVariant::fromValue(stop));
    }
    return readGradientStopList(stops);
}

} // namespace QAccelPlot
