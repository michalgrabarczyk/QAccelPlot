//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/effects/SeriesEffect.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

#include <limits>

namespace QAccelPlot {

/// \brief A LineCurve effect that replaces the solid line color with a color gradient.
///
/// Attach to \c LineCurve::effects to color the line with a gradient. The color stops come from
/// \c colormap when set, otherwise from a Qt \c Gradient assigned to \c gradient. The gradient
/// can run horizontally (along X data values) or vertically (along Y data values).
///
/// \sa GradientFill, Colormap, SeriesEffect, LineCurve
class GradientStroke : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientStroke)

    /// \brief Axis along which the gradient color varies: \c Qt.Horizontal for X values, \c Qt.Vertical for Y values. Default: \c Qt.Horizontal.
    Q_PROPERTY(Qt::Orientation direction READ direction WRITE setDirection NOTIFY directionChanged)
    /// \brief A Qt \c Gradient (or compatible) object supplying the color stops.
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    /// \brief Colormap supplying the color stops. Overrides \c gradient when set.
    ///
    /// Only the ramp is used. \c Colormap::min, \c Colormap::max, and \c Colormap::logScale are ignored;
    /// \c valueMin and \c valueMax place the ramp instead.
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    /// \brief Data value at gradient position 0. Unset by default: the smallest value of each series
    /// along \c direction, or of its axis data range when the series has no valid data point.
    ///
    /// Reads NaN while unset; assign \c undefined to unset it.
    Q_PROPERTY(qreal valueMin READ valueMin WRITE setValueMin RESET resetValueMin NOTIFY valueMinChanged)
    /// \brief Data value at gradient position 1. Unset by default: the largest value of each series
    /// along \c direction, or of its axis data range when the series has no valid data point.
    ///
    /// Reads NaN while unset; assign \c undefined to unset it.
    Q_PROPERTY(qreal valueMax READ valueMax WRITE setValueMax RESET resetValueMax NOTIFY valueMaxChanged)

public:
    /// \brief Constructs a GradientStroke with the given \a parent.
    explicit GradientStroke(QObject* parent = nullptr);

    /// \brief Returns the gradient direction.
    Qt::Orientation direction() const;
    /// \brief Sets the gradient direction to \a direction.
    void setDirection(Qt::Orientation direction);

    /// \brief Returns the Qt Gradient object supplying color stops.
    QObject* gradient() const;
    /// \brief Sets the Qt Gradient object to \a gradient.
    void setGradient(QObject* gradient);

    /// \brief Returns the colormap supplying color stops, or \c nullptr when \c gradient supplies them.
    Colormap* colormap() const;
    /// \brief Sets the colormap to \a colormap. Pass \c nullptr to take the stops from \c gradient.
    void setColormap(Colormap* colormap);

    /// \brief Returns the data value at gradient position 0, or NaN when each series' data range applies.
    qreal valueMin() const;
    /// \brief Sets the data value at gradient position 0 to \a value. NaN unsets it.
    void setValueMin(qreal value);
    /// \brief Unsets the value at gradient position 0, so each series' data range applies again.
    void resetValueMin();

    /// \brief Returns the data value at gradient position 1, or NaN when each series' data range applies.
    qreal valueMax() const;
    /// \brief Sets the data value at gradient position 1 to \a value. NaN unsets it.
    void setValueMax(qreal value);
    /// \brief Unsets the value at gradient position 1, so each series' data range applies again.
    void resetValueMax();

    /// \brief Returns a snapshot of all stroke parameters, updated on the GUI thread when the effect changes.
    ///
    /// Does not read the QML gradient, so \c updatePaintNode() can call it on the render thread.
    const GradientColorPayload& payload() const;

    /// \brief Re-reads the \c gradient stops, which can change without a notification, and emits
    /// \c effectChanged() if they differ from the snapshot.
    void refresh() override;

signals:
    /// \brief Emitted when the direction property changes.
    void directionChanged();
    /// \brief Emitted when the gradient property changes.
    void gradientChanged();
    /// \brief Emitted when the colormap property changes.
    void colormapChanged();
    /// \brief Emitted when the valueMin property changes.
    void valueMinChanged();
    /// \brief Emitted when the valueMax property changes.
    void valueMaxChanged();

private:
    GradientColorPayload readPayload() const;
    void updatePayload();
    void reconnectGradientSignals();
    void disconnectGradientSignals();
    Q_SLOT void onGradientObjectChanged();
    void reconnectColormapSignals();
    void disconnectColormapSignals();

    Qt::Orientation direction_{Qt::Horizontal};
    QObject* gradient_{nullptr};
    QVector<QMetaObject::Connection> gradientConnections_;
    Colormap* colormap_{nullptr};
    QVector<QMetaObject::Connection> colormapConnections_;
    qreal valueMin_{std::numeric_limits<qreal>::quiet_NaN()};
    qreal valueMax_{std::numeric_limits<qreal>::quiet_NaN()};
    GradientColorPayload payload_;
};

} // namespace QAccelPlot
