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
#include <QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

#include <vector>

namespace QAccelPlot {

/// \brief Maps data values to colors: a color ramp plus the rule that places a value on it.
///
/// A colormap has no geometry. It is a one-dimensional ramp addressed by a normalized coordinate
/// in [0, 1], and \c logScale decides how a value reaches that coordinate. Series that color by value,
/// such as \c PointCloud and \c HeatmapSeries, index the ramp with each value. Effects that paint a
/// ramp across the plot, such as \c GradientFill and \c GradientStroke, supply the coordinate from a
/// position instead and own that geometry themselves, so they use only the ramp.
///
/// The ramp comes from \c preset, or from \c stops when a custom ramp is given. \c min and \c max
/// bound the value range; leaving either unset resolves it from the data, so one colormap can be
/// shared between series that resolve different ranges. Each series reports what it resolved
/// through \c ColormappedSeries::dataValueMin() and \c ColormappedSeries::dataValueMax().
///
/// \sa PointCloud, HeatmapSeries, BarSeries, GradientFill, GradientStroke
class Colormap : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_NAMED_ELEMENT(Colormap)

    /// \brief Built-in color ramp. Ignored when \c stops is non-empty. Default: \c Viridis.
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY colormapChanged)
    /// \brief Custom ramp as a list of objects with \c position and \c color, such as \c GradientStop.
    /// Overrides \c preset when non-empty. Stops are read again once the component is complete. The
    /// ramp follows changes to a stop that notifies them, to a \c GradientStop inside a \c Gradient,
    /// and to a \c GradientStop declared in this list.
    Q_PROPERTY(QQmlListProperty<QObject> stops READ stops NOTIFY colormapChanged)
    /// \brief Whether the ramp runs from its last color to its first. Applies to \c preset and
    /// \c stops alike. Default: \c false.
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY colormapChanged)
    /// \brief Value at ramp position 0. Unset (NaN) resolves it from the data. Default: unset.
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY colormapChanged)
    /// \brief Value at ramp position 1. Unset (NaN) resolves it from the data. Default: unset.
    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY colormapChanged)
    /// \brief Whether values are placed on the ramp in log10 instead of linearly. Default: \c false.
    ///
    /// Values at or below zero are unmapped. A \c ColorBar showing the colormap gets decade ticks.
    Q_PROPERTY(bool logScale READ logScale WRITE setLogScale NOTIFY colormapChanged)

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
        Grayscale, ///< \brief Black to white.
        Rainbow    ///< \brief Classic blue-to-red rainbow. Not perceptually uniform.
    };
    Q_ENUM(Preset)

    /// \brief Constructs a Colormap with the given \a parent.
    explicit Colormap(QObject* parent = nullptr);

    /// \brief Returns the built-in ramp.
    Preset preset() const;
    /// \brief Sets the built-in ramp to \a preset.
    void setPreset(Preset preset);

    /// \brief Returns the custom ramp stops, empty when the preset supplies the ramp.
    QQmlListProperty<QObject> stops();

    /// \brief Returns whether the ramp is reversed.
    bool reversed() const;
    /// \brief Sets whether the ramp is reversed to \a reversed.
    void setReversed(bool reversed);

    /// \brief Returns the fixed lower bound, or NaN when it is resolved from the data.
    qreal min() const;
    /// \brief Sets the fixed lower bound to \a value. NaN resolves it from the data.
    void setMin(qreal value);

    /// \brief Returns the fixed upper bound, or NaN when it is resolved from the data.
    qreal max() const;
    /// \brief Sets the fixed upper bound to \a value. NaN resolves it from the data.
    void setMax(qreal value);

    /// \brief Sets both bounds to \a min and \a max with a single \c colormapChanged(). NaN unsets a bound.
    Q_INVOKABLE void setRange(qreal min, qreal max);

    /// \brief Returns \c true when values are placed on the ramp in log10.
    bool logScale() const;
    /// \brief Sets log-scale placement to \a on.
    void setLogScale(bool on);

    /// \brief Returns the resolved ramp stops, in position order and covering [0, 1].
    ///
    /// Custom \c stops when set, otherwise the \c preset ramp, flipped when \c reversed is set.
    const std::vector<GradientStopData>& resolvedStops() const;

signals:
    /// \brief Emitted when the ramp, its bounds, or \c logScale changes.
    void colormapChanged();

protected:
    /// \brief Does nothing; part of \c QQmlParserStatus.
    void classBegin() override;
    /// \brief Reads the stops again after their bindings are evaluated.
    void componentComplete() override;

private:
    Q_SLOT void onStopChanged();
    void onStopDestroyed(QObject* stop);
    // Tracks the stop's lifetime and its position and color edits.
    void connectStop(QObject* stop);
    void disconnectStops();
    // Moves a GradientStop declared in this list under an internal Gradient, so its edits notify.
    void adoptStop(QObject* stop);
    QObject* stopTracker();
    void rebuildStops();
    std::vector<GradientStopData> customStops() const;

    static void appendStop(QQmlListProperty<QObject>* list, QObject* stop);
    static qsizetype stopCount(QQmlListProperty<QObject>* list);
    static QObject* stopAt(QQmlListProperty<QObject>* list, qsizetype index);
    static void clearStops(QQmlListProperty<QObject>* list);

    Preset preset_{Preset::Viridis};
    QList<QObject*> stopObjects_;
    bool reversed_{false};
    std::vector<GradientStopData> resolvedStops_;
    qreal min_;
    qreal max_;
    bool logScale_{false};
};

} // namespace QAccelPlot
