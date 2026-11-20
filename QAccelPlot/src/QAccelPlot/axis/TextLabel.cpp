//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/TextLabel.hpp"
#include "QAccelPlot/MathUtils.hpp"

namespace QAccelPlot {

TextLabel::TextLabel(const QColor& color, const qreal padding, QObject* parent)
    : QObject(parent)
    , color_(color)
    , padding_(padding)
{
}

QString TextLabel::text() const
{
    return text_;
}

void TextLabel::setText(const QString& text)
{
    if (text_ == text) {
        return;
    }
    text_ = text;
    emit textChanged();
}

QFont TextLabel::font() const
{
    return font_;
}

void TextLabel::setFont(const QFont& font)
{
    if (font_ == font) {
        return;
    }
    font_ = font;
    emit fontChanged();
}

QColor TextLabel::color() const
{
    return color_.value();
}

void TextLabel::setColor(const QColor& color)
{
    if (!color.isValid()) {
        resetColor();
        return;
    }
    if (color_.set(color)) {
        emit colorChanged();
    }
}

void TextLabel::resetColor()
{
    if (color_.reset()) {
        emit colorChanged();
    }
}

void TextLabel::setDefaultColor(const QColor& color)
{
    if (color_.setFallback(color)) {
        emit colorChanged();
    }
}

qreal TextLabel::padding() const
{
    return padding_;
}

void TextLabel::setPadding(const qreal padding)
{
    if (nearlyEqual(padding_, padding)) {
        return;
    }
    padding_ = padding;
    emit paddingChanged();
}

} // namespace QAccelPlot
