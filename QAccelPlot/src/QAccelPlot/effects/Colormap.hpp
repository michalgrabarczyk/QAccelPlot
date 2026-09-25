//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <QObject>
#include <QQmlListProperty>
#include <QtQml/qqmlregistration.h>

#include <vector>

namespace QAccelPlot {

/// \brief Maps data values to colors: a color ramp plus the rule that places a value on it.
///
/// A colormap has no geometry. It is a one-dimensional ramp addressed by a normalized coordinate
/// in [0, 1], and \c norm decides how a value reaches that coordinate. Series that color by value,
/// such as \c PointCloud, index the ramp with each point's value. Effects that paint a ramp across
/// the plot, such as \c GradientFill and \c GradientStroke, supply the coordinate from a position
/// instead and own that geometry themselves, so they use only the ramp.
///
/// The ramp comes from \c preset, or from \c stops when a custom ramp is given. \c min and \c max
/// bound the value range; leaving either unset resolves it from the data, so one colormap can be
/// shared between series that resolve different ranges. Each series reports what it resolved, for
/// example through \c PointCloud::dataValueMin().
///
/// \sa PointCloud, GradientFill, GradientStroke
class Colormap : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Colormap)

    /// \brief Built-in color ramp. Ignored when \c stops is non-empty. Default: \c Viridis.
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY colormapChanged)
    /// \brief Custom ramp as a list of objects with \c position and \c color, such as \c GradientStop.
    /// Overrides \c preset when non-empty.
    Q_PROPERTY(QQmlListProperty<QObject> stops READ stops NOTIFY colormapChanged)
    /// \brief Value at ramp position 0. Unset (NaN) resolves it from the data. Default: unset.
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY colormapChanged)
    /// \brief Value at ramp position 1. Unset (NaN) resolves it from the data. Default: unset.
    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY colormapChanged)
    /// \brief How a value is placed on the ramp. Default: \c Linear.
    Q_PROPERTY(Normalization norm READ norm WRITE setNorm NOTIFY colormapChanged)

public:
    /// \brief Built-in color ramps.
    ///
    /// The perceptually uniform ramps are sampled from the matplotlib originals. Prefer them over
    /// \c Rainbow, whose uneven lightness invents boundaries that are not in the data.
    enum class Preset {
        Viridis,   ///< \brief Perceptually uniform, dark blue to yellow.
        Plasma,    ///< \brief Perceptually uniform, dark blue to yellow through magenta.
        Inferno,   ///< \brief Perceptually uniform, black to pale yellow through red.
        Magma,     ///< \brief Perceptually uniform, black to pale pink through purple.
        Turbo,     ///< \brief High-contrast rainbow with smooth lightness, for fine detail.
        Grayscale, ///< \brief Black to white.
        Rainbow    ///< \brief Classic blue-to-red rainbow. Not perceptually uniform.
    };
    Q_ENUM(Preset)

    /// \brief How a value is mapped onto the [0, 1] ramp coordinate.
    enum class Normalization {
        Linear, ///< \brief Proportional between \c min and \c max.
        Log     ///< \brief Proportional in log10. Values at or below zero are treated as unmapped.
    };
    Q_ENUM(Normalization)

    /// \brief Constructs a Colormap with the given \a parent.
    explicit Colormap(QObject* parent = nullptr);

    /// \brief Returns the built-in ramp.
    Preset preset() const;
    /// \brief Sets the built-in ramp to \a preset.
    void setPreset(Preset preset);

    /// \brief Returns the custom ramp stops, empty when the preset supplies the ramp.
    QQmlListProperty<QObject> stops();

    /// \brief Returns the fixed lower bound, or NaN when it is resolved from the data.
    qreal min() const;
    /// \brief Sets the fixed lower bound to \a value. NaN resolves it from the data.
    void setMin(qreal value);

    /// \brief Returns the fixed upper bound, or NaN when it is resolved from the data.
    qreal max() const;
    /// \brief Sets the fixed upper bound to \a value. NaN resolves it from the data.
    void setMax(qreal value);

    /// \brief Returns the normalization.
    Normalization norm() const;
    /// \brief Sets the normalization to \a norm.
    void setNorm(Normalization norm);

    /// \brief Returns the resolved ramp stops, in position order and covering [0, 1].
    ///
    /// Custom \c stops when set, otherwise the \c preset ramp.
    const std::vector<GradientStopData>& resolvedStops() const;

signals:
    /// \brief Emitted when the ramp, its bounds, or the normalization changes.
    void colormapChanged();

private:
    void rebuildStops();

    static void appendStop(QQmlListProperty<QObject>* list, QObject* stop);
    static qsizetype stopCount(QQmlListProperty<QObject>* list);
    static QObject* stopAt(QQmlListProperty<QObject>* list, qsizetype index);
    static void clearStops(QQmlListProperty<QObject>* list);

    Preset preset_{Preset::Viridis};
    QList<QObject*> stopObjects_;
    std::vector<GradientStopData> resolvedStops_;
    qreal min_;
    qreal max_;
    Normalization norm_{Normalization::Linear};
};

} // namespace QAccelPlot
