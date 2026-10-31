//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/effects/LineCurveEffect.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief A LineCurve effect that replaces the solid line color with a color gradient.
///
/// Attach to \c LineCurve::effects to color the line with a gradient. The color stops come from
/// \c colormap when set, otherwise from a Qt \c Gradient assigned to \c gradient. The gradient
/// can run horizontally (along X data values) or vertically (along Y data values).
///
/// \sa GradientFill, Colormap, LineCurveEffect, LineCurve
class GradientStroke : public LineCurveEffect {
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
    /// \brief How the gradient normalization minimum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource valueMinSource READ valueMinSource WRITE setValueMinSource NOTIFY valueMinSourceChanged)
    /// \brief Data-space value that maps to gradient position 0.0. Only used when valueMinSource is Fixed.
    Q_PROPERTY(qreal valueMin READ valueMin WRITE setValueMin NOTIFY valueMinChanged)
    /// \brief How the gradient normalization maximum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource valueMaxSource READ valueMaxSource WRITE setValueMaxSource NOTIFY valueMaxSourceChanged)
    /// \brief Data-space value that maps to gradient position 1.0. Only used when valueMaxSource is Fixed.
    Q_PROPERTY(qreal valueMax READ valueMax WRITE setValueMax NOTIFY valueMaxChanged)

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

    /// \brief Returns how the gradient normalization minimum is determined.
    GradientValueSource valueMinSource() const;
    /// \brief Sets the gradient normalization minimum source to \a source.
    void setValueMinSource(GradientValueSource source);

    /// \brief Returns the minimum data value for gradient normalization.
    qreal valueMin() const;
    /// \brief Sets the minimum data value for gradient normalization to \a value.
    void setValueMin(qreal value);

    /// \brief Returns how the gradient normalization maximum is determined.
    GradientValueSource valueMaxSource() const;
    /// \brief Sets the gradient normalization maximum source to \a source.
    void setValueMaxSource(GradientValueSource source);

    /// \brief Returns the maximum data value for gradient normalization.
    qreal valueMax() const;
    /// \brief Sets the maximum data value for gradient normalization to \a value.
    void setValueMax(qreal value);

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
    /// \brief Emitted when the valueMinSource property changes.
    void valueMinSourceChanged();
    /// \brief Emitted when the valueMin property changes.
    void valueMinChanged();
    /// \brief Emitted when the valueMaxSource property changes.
    void valueMaxSourceChanged();
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
    GradientValueSource valueMinSource_{GradientValueSource::DataRange};
    qreal valueMin_{0.0};
    GradientValueSource valueMaxSource_{GradientValueSource::DataRange};
    qreal valueMax_{0.0};
    GradientColorPayload payload_;
};

} // namespace QAccelPlot
