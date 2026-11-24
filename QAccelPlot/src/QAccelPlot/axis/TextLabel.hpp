//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/internal/ResettableValue.hpp"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Text, font, color, and padding of a title, grouped as the \c label property of ColorBar and Axis.
///
/// \sa AxisLabel
class TextLabel : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Title text. Default: empty, which draws no title.
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /// \brief Title font. Default: application default font.
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    /// \brief Title color. Default: \c Colors.dark.axisLine on a ColorBar, the axis \c baselineColor on an Axis.
    /// Assign \c undefined to return to it.
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)
    /// \brief Gap in pixels between the title and the axis or color strip. Default: 30 on an Axis, 6 on a ColorBar.
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)
    /// \brief Prefixed unit the tick label formatter chose for the visible range, such as "mm". Read-only.
    ///
    /// An Axis updates it whenever it computes its ticks. It is empty when the formatter shows no unit, as with every
    /// built-in formatter except \c UnitTickLabelFormatter. While it is set, the Axis paints its title as
    /// "text (unitText)", such as "Distance (mm)". A \c delegate can bind to it.
    Q_PROPERTY(QString unitText READ unitText NOTIFY unitTextChanged)

public:
    /// \brief Constructs a label with the default \a color and \a padding, owned by \a parent.
    explicit TextLabel(const QColor& color = {}, qreal padding = 0.0, QObject* parent = nullptr);

    /// \brief Returns the title text.
    QString text() const;
    /// \brief Sets the title text to \a text.
    void setText(const QString& text);

    /// \brief Returns the title font.
    QFont font() const;
    /// \brief Sets the title font to \a font.
    void setFont(const QFont& font);

    /// \brief Returns the title color in effect.
    QColor color() const;
    /// \brief Sets the title color to \a color. An invalid color restores the default.
    void setColor(const QColor& color);
    /// \brief Restores the default title color.
    void resetColor();
    /// \brief Sets the color used while none is set to \a color. Called by the owning Axis or ColorBar.
    void setDefaultColor(const QColor& color);

    /// \brief Returns the gap between the title and the axis or color strip in pixels.
    qreal padding() const;
    /// \brief Sets the gap between the title and the axis or color strip to \a padding pixels.
    void setPadding(qreal padding);

    /// \brief Returns the prefixed unit chosen for the visible range, or an empty string for none.
    QString unitText() const;
    /// \brief Sets the prefixed unit to \a unitText. Called by the owning Axis when its ticks change.
    void setUnitText(const QString& unitText);

    /// \brief Returns the title text combined with the unit text, such as "Distance (mm)", or \c text() without unit text.
    QString paintedText() const;

signals:
    /// \brief Emitted when the text property changes.
    void textChanged();
    /// \brief Emitted when the font property changes.
    void fontChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the padding property changes.
    void paddingChanged();
    /// \brief Emitted when the unitText property changes.
    void unitTextChanged();

private:
    QString text_;
    QFont font_;
    Internal::ResettableValue<QColor> color_;
    qreal padding_;
    QString unitText_;
};

} // namespace QAccelPlot
