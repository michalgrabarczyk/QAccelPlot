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

/// \brief Text, font, color, and padding of an axis label, grouped as the \c label property of ColorBar and Axis.
///
/// \sa AxisLabel
class TextLabel : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Axis label text. Default: empty, which draws \c unitText alone, or no axis label without unit text.
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /// \brief Axis label font. Default: application default font.
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    /// \brief Axis label color. Default: \c Colors.dark.axisLine on a ColorBar, the axis \c baselineColor on an Axis.
    /// Assign \c undefined to return to it.
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)
    /// \brief Gap in pixels between the axis label and the axis or color strip. Default: 30 on an Axis, 6 on a ColorBar.
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)
    /// \brief Prefixed unit the tick label formatter chose for the current ticks, such as "mm". Read-only.
    ///
    /// The owning Axis or ColorBar updates it whenever it computes its ticks. It is empty when the formatter shows no
    /// unit, as with every built-in formatter except \c UnitTickLabelFormatter. While it is set, the owner paints its
    /// axis label with \c unitFormat, such as "Distance (mm)". An Axis \c delegate can bind to it.
    Q_PROPERTY(QString unitText READ unitText NOTIFY unitTextChanged)
    /// \brief How the painted axis label combines \c text (\c %1) and \c unitText (\c %2). Default: "%1 (%2)".
    ///
    /// For example, "%1 [%2]" paints "Distance [mm]" and "%1 / %2" paints "Distance / mm". It applies only while
    /// both \c text and \c unitText are set: an empty \c text paints \c unitText alone, and an empty \c unitText
    /// paints \c text alone. Assign \c undefined to return to the default.
    Q_PROPERTY(QString unitFormat READ unitFormat WRITE setUnitFormat RESET resetUnitFormat NOTIFY unitFormatChanged)

public:
    /// \brief Constructs a label with the default \a color and \a padding, owned by \a parent.
    explicit TextLabel(const QColor& color = {}, qreal padding = 0.0, QObject* parent = nullptr);

    /// \brief Returns the axis label text.
    QString text() const;
    /// \brief Sets the axis label text to \a text.
    void setText(const QString& text);

    /// \brief Returns the axis label font.
    QFont font() const;
    /// \brief Sets the axis label font to \a font.
    void setFont(const QFont& font);

    /// \brief Returns the axis label color in effect.
    QColor color() const;
    /// \brief Sets the axis label color to \a color. An invalid color restores the default.
    void setColor(const QColor& color);
    /// \brief Restores the default axis label color.
    void resetColor();
    /// \brief Sets the color used while none is set to \a color. Called by the owning Axis or ColorBar.
    void setDefaultColor(const QColor& color);

    /// \brief Returns the gap between the axis label and the axis or color strip in pixels.
    qreal padding() const;
    /// \brief Sets the gap between the axis label and the axis or color strip to \a padding pixels.
    void setPadding(qreal padding);

    /// \brief Returns the prefixed unit chosen for the current ticks, or an empty string for none.
    QString unitText() const;
    /// \brief Sets the prefixed unit to \a unitText. Called by the owning Axis or ColorBar when its ticks change.
    void setUnitText(const QString& unitText);

    /// \brief Returns how the painted axis label combines the text and the unit text.
    QString unitFormat() const;
    /// \brief Sets how the painted axis label combines the text (\c %1) and the unit text (\c %2) to \a format.
    void setUnitFormat(const QString& format);
    /// \brief Restores the default unit format "%1 (%2)".
    void resetUnitFormat();

    /// \brief Returns the axis label text combined with the unit text by unitFormat(), such as "Distance (mm)".
    ///
    /// Returns the unit text alone for an empty text, and the text alone without unit text.
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
    /// \brief Emitted when the unitFormat property changes.
    void unitFormatChanged();

private:
    QString text_;
    QFont font_;
    Internal::ResettableValue<QColor> color_;
    qreal padding_;
    QString unitText_;
    Internal::ResettableValue<QString> unitFormat_;
};

} // namespace QAccelPlot
