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

#include <array>
#include <limits>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

// Control points sampled at even spacing from the matplotlib ramps of the same name. Ten samples
// reproduce them closely enough that linear interpolation between them is visually indistinguishable
// at the 256-texel lookup texture the renderer builds.
using RampSamples = std::vector<const char*>;

const RampSamples& rampFor(const Colormap::Preset preset)
{
    static const auto viridis
        = RampSamples{"#440154", "#482878", "#3e4a89", "#31688e", "#26828e", "#1f9e89", "#35b779", "#6dcd59", "#b4de2c", "#fde725"};
    static const auto plasma
        = RampSamples{"#0d0887", "#46039f", "#7201a8", "#9c179e", "#bd3786", "#d8576b", "#ed7953", "#fb9f3a", "#fdc926", "#f0f921"};
    static const auto inferno
        = RampSamples{"#000004", "#1b0c41", "#4a0c6b", "#781c6d", "#a52c60", "#cf4446", "#ed6925", "#fb9b06", "#f7d13d", "#fcffa4"};
    static const auto magma
        = RampSamples{"#000004", "#180f3d", "#440f76", "#721f81", "#9e2f7f", "#cd4071", "#f1605d", "#fd9668", "#feca8d", "#fcfdbf"};
    static const auto turbo = RampSamples{"#30123b", "#4145ab", "#4675ed", "#39a2fc", "#1bcfd4", "#24eca6", "#61fc6c", "#a4fc3b", "#d1e834",
        "#f3c63a", "#fe9b2d", "#f36315", "#d93806", "#b11901", "#7a0403"};
    static const auto grayscale = RampSamples{"#000000", "#ffffff"};
    static const auto rainbow = RampSamples{"#0000ff", "#00ffff", "#00ff00", "#ffff00", "#ff0000"};

    switch (preset) {
    case Colormap::Preset::Plasma:
        return plasma;
    case Colormap::Preset::Inferno:
        return inferno;
    case Colormap::Preset::Magma:
        return magma;
    case Colormap::Preset::Turbo:
        return turbo;
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

// Explicit stops win; a list that yields none falls through to the preset rather than
// leaving the ramp empty.
void Colormap::rebuildStops()
{
    if (!stopObjects_.isEmpty()) {
        auto stops = QVariantList{};
        stops.reserve(stopObjects_.size());
        for (auto* stop : stopObjects_) {
            stops.append(QVariant::fromValue(stop));
        }
        resolvedStops_ = readGradientStopList(stops);
        if (!resolvedStops_.empty()) {
            return;
        }
    }
    resolvedStops_ = presetStops(preset_);
}

} // namespace QAccelPlot
