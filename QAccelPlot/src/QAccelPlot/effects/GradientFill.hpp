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

/// \brief A LineCurve effect that fills the area under the curve with a color gradient.
///
/// Attach to \c LineCurve::effects to render a shaded fill between the curve and a configurable
/// baseline. The color stops come from \c colormap when set, otherwise from a Qt \c Gradient
/// assigned to \c gradient. \c direction sets the axis the colors vary along.
///
/// \sa GradientStroke, Colormap, LineCurveEffect, LineCurve
class GradientFill : public LineCurveEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientFill)

    /// \brief Axis along which the gradient color varies (Horizontal or Vertical).
    Q_PROPERTY(GradientDirection direction READ direction WRITE setDirection NOTIFY directionChanged)
    /// \brief A Qt \c Gradient (or compatible) object supplying the color stops.
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    /// \brief Colormap supplying the color stops. Overrides \c gradient when set.
    ///
    /// Only the ramp is used. \c Colormap::min, \c Colormap::max, and \c Colormap::norm are ignored;
    /// \c gradientValueMin and \c gradientValueMax place the ramp instead.
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    /// \brief How the gradient normalization minimum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource gradientValueMinSource READ gradientValueMinSource WRITE setGradientValueMinSource NOTIFY gradientValueMinSourceChanged)
    /// \brief Data-space value that maps to gradient position 0.0. Only used when gradientValueMinSource is Fixed.
    Q_PROPERTY(qreal gradientValueMin READ gradientValueMin WRITE setGradientValueMin NOTIFY gradientValueMinChanged)
    /// \brief How the gradient normalization maximum is determined; see \c GradientValueSource.
    Q_PROPERTY(GradientValueSource gradientValueMaxSource READ gradientValueMaxSource WRITE setGradientValueMaxSource NOTIFY gradientValueMaxSourceChanged)
    /// \brief Data-space value that maps to gradient position 1.0. Only used when gradientValueMaxSource is Fixed.
    Q_PROPERTY(qreal gradientValueMax READ gradientValueMax WRITE setGradientValueMax NOTIFY gradientValueMaxChanged)
    /// \brief Where the filled area's baseline starts (axis minimum or a fixed value).
    Q_PROPERTY(GradientFillBaseline baseline READ baseline WRITE setBaseline NOTIFY baselineChanged)
    /// \brief Fixed baseline data value used when \c baseline is \c GradientFillBaseline.Value.
    Q_PROPERTY(qreal baselineValue READ baselineValue WRITE setBaselineValue NOTIFY baselineValueChanged)
    /// \brief Overall opacity of the fill area in [0, 1]. Default: 0.35.
    ///
    /// Applies to the fill only. The curve's \c opacity and the \c opacity of its parent
    /// items also apply, so the rendered fill alpha is the product of all of them.
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)

public:
    /// \brief Constructs a GradientFill with the given \a parent.
    explicit GradientFill(QObject* parent = nullptr);

    /// \brief Returns the gradient direction.
    GradientDirection direction() const;
    /// \brief Sets the gradient direction to \a direction.
    void setDirection(GradientDirection direction);

    /// \brief Returns the Qt Gradient object supplying color stops.
    QObject* gradient() const;
    /// \brief Sets the Qt Gradient object to \a gradient.
    void setGradient(QObject* gradient);

    /// \brief Returns the colormap supplying color stops, or \c nullptr when \c gradient supplies them.
    Colormap* colormap() const;
    /// \brief Sets the colormap to \a colormap. Pass \c nullptr to take the stops from \c gradient.
    void setColormap(Colormap* colormap);

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

    /// \brief Returns the baseline mode.
    GradientFillBaseline baseline() const;
    /// \brief Sets the baseline mode to \a baseline.
    void setBaseline(GradientFillBaseline baseline);

    /// \brief Returns the fixed baseline data value.
    qreal baselineValue() const;
    /// \brief Sets the fixed baseline data value to \a value.
    void setBaselineValue(qreal value);

    /// \brief Returns the fill opacity.
    qreal opacity() const;
    /// \brief Sets the fill opacity to \a value.
    void setOpacity(qreal value);

    /// \brief Returns a render-thread-safe snapshot of all fill parameters.
    GradientFillPayload payload() const;

signals:
    /// \brief Emitted when the direction property changes.
    void directionChanged();
    /// \brief Emitted when the gradient property changes.
    void gradientChanged();
    /// \brief Emitted when the colormap property changes.
    void colormapChanged();
    /// \brief Emitted when the gradientValueMinSource property changes.
    void gradientValueMinSourceChanged();
    /// \brief Emitted when the gradientValueMin property changes.
    void gradientValueMinChanged();
    /// \brief Emitted when the gradientValueMaxSource property changes.
    void gradientValueMaxSourceChanged();
    /// \brief Emitted when the gradientValueMax property changes.
    void gradientValueMaxChanged();
    /// \brief Emitted when the baseline property changes.
    void baselineChanged();
    /// \brief Emitted when the baselineValue property changes.
    void baselineValueChanged();
    /// \brief Emitted when the opacity property changes.
    void opacityChanged();

private:
    void reconnectGradientSignals();
    void disconnectGradientSignals();
    Q_SLOT void onGradientObjectChanged();
    void reconnectColormapSignals();
    void disconnectColormapSignals();

    GradientDirection direction_{GradientDirection::Horizontal};
    QObject* gradient_{nullptr};
    QVector<QMetaObject::Connection> gradientConnections_;
    Colormap* colormap_{nullptr};
    QVector<QMetaObject::Connection> colormapConnections_;
    GradientValueSource gradientValueMinSource_{GradientValueSource::DataRange};
    qreal gradientValueMin_{0.0};
    GradientValueSource gradientValueMaxSource_{GradientValueSource::DataRange};
    qreal gradientValueMax_{0.0};
    GradientFillBaseline baseline_{GradientFillBaseline::AxisMinimum};
    qreal baselineValue_{0.0};
    qreal opacity_{0.35};
};

} // namespace QAccelPlot
