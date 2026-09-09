//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/LineCurveEffect.hpp"
#include "effects/GradientColorTypes.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief A LineCurve effect that replaces the solid line color with a color gradient.
///
/// Attach to \c LineCurve::effects to color the line with a gradient derived from a Qt
/// \c Gradient object. The gradient can run horizontally (along X data values) or
/// vertically (along Y data values).
///
/// \sa GradientFill, LineCurveEffect, LineCurve
class GradientStroke : public LineCurveEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientStroke)

    /// \brief Axis along which the gradient color varies (Horizontal or Vertical).
    Q_PROPERTY(GradientDirection direction READ direction WRITE setDirection NOTIFY directionChanged)
    /// \brief A Qt \c Gradient (or compatible) object supplying the color stops.
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    /// \brief How the gradient normalization minimum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource gradientValueMinSource READ gradientValueMinSource WRITE setGradientValueMinSource NOTIFY gradientValueMinSourceChanged)
    /// \brief Data-space value that maps to gradient position 0.0. Only used when gradientValueMinSource is Fixed.
    Q_PROPERTY(qreal gradientValueMin READ gradientValueMin WRITE setGradientValueMin NOTIFY gradientValueMinChanged)
    /// \brief How the gradient normalization maximum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource gradientValueMaxSource READ gradientValueMaxSource WRITE setGradientValueMaxSource NOTIFY gradientValueMaxSourceChanged)
    /// \brief Data-space value that maps to gradient position 1.0. Only used when gradientValueMaxSource is Fixed.
    Q_PROPERTY(qreal gradientValueMax READ gradientValueMax WRITE setGradientValueMax NOTIFY gradientValueMaxChanged)

public:
    /// \brief Constructs a GradientStroke with the given \a parent.
    explicit GradientStroke(QObject* parent = nullptr);

    /// \brief Returns the gradient direction.
    GradientDirection direction() const;
    /// \brief Sets the gradient direction to \a direction.
    void setDirection(GradientDirection direction);

    /// \brief Returns the Qt Gradient object supplying color stops.
    QObject* gradient() const;
    /// \brief Sets the Qt Gradient object to \a gradient.
    void setGradient(QObject* gradient);

    /// \brief Returns how the gradient normalization minimum is determined.
    GradientValueSource gradientValueMinSource() const;
    /// \brief Sets the gradient normalization minimum source to \a source.
    void setGradientValueMinSource(GradientValueSource source);

    /// \brief Returns the minimum data value for gradient normalization.
    qreal gradientValueMin() const;
    /// \brief Sets the minimum data value for gradient normalization to \a value.
    void setGradientValueMin(qreal value);

    /// \brief Returns how the gradient normalization maximum is determined.
    GradientValueSource gradientValueMaxSource() const;
    /// \brief Sets the gradient normalization maximum source to \a source.
    void setGradientValueMaxSource(GradientValueSource source);

    /// \brief Returns the maximum data value for gradient normalization.
    qreal gradientValueMax() const;
    /// \brief Sets the maximum data value for gradient normalization to \a value.
    void setGradientValueMax(qreal value);

    /// \brief Returns a render-thread-safe snapshot of all stroke parameters.
    GradientColorPayload payload() const;

signals:
    /// \brief Emitted when the direction property changes.
    void directionChanged();
    /// \brief Emitted when the gradient property changes.
    void gradientChanged();
    /// \brief Emitted when the gradientValueMinSource property changes.
    void gradientValueMinSourceChanged();
    /// \brief Emitted when the gradientValueMin property changes.
    void gradientValueMinChanged();
    /// \brief Emitted when the gradientValueMaxSource property changes.
    void gradientValueMaxSourceChanged();
    /// \brief Emitted when the gradientValueMax property changes.
    void gradientValueMaxChanged();

private:
    void reconnectGradientSignals();
    void disconnectGradientSignals();
    void onGradientObjectChanged();

    GradientDirection direction_{GradientDirection::Horizontal};
    QObject* gradient_{nullptr};
    QVector<QMetaObject::Connection> gradientConnections_;
    GradientValueSource gradientValueMinSource_{GradientValueSource::DataRange};
    qreal gradientValueMin_{0.0};
    GradientValueSource gradientValueMaxSource_{GradientValueSource::DataRange};
    qreal gradientValueMax_{0.0};
};

} // namespace QAccelPlot
