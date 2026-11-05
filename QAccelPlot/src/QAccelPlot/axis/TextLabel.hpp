//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

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
    /// \brief Title color. Default: \c Colors.dark.axisLine on a ColorBar; on an Axis, an invalid color that follows \c baselineColor.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Gap in pixels between the title and the axis or color strip. Default: 30 on an Axis, 6 on a ColorBar.
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)

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

    /// \brief Returns the title color.
    QColor color() const;
    /// \brief Sets the title color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the gap between the title and the axis or color strip in pixels.
    qreal padding() const;
    /// \brief Sets the gap between the title and the axis or color strip to \a padding pixels.
    void setPadding(qreal padding);

signals:
    /// \brief Emitted when the text property changes.
    void textChanged();
    /// \brief Emitted when the font property changes.
    void fontChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the padding property changes.
    void paddingChanged();

private:
    QString text_;
    QFont font_;
    QColor color_;
    qreal padding_;
};

} // namespace QAccelPlot
