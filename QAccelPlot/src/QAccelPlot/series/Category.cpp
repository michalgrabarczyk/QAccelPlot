//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/Category.hpp"

#include <utility>

namespace QAccelPlot {

Category::Category(QObject* parent)
    : QObject(parent)
{
}

Category::Category(const int value, QString name, const QColor& color, QObject* parent)
    : QObject(parent)
    , value_(value)
    , name_(std::move(name))
    , color_(color)
{
}

int Category::value() const
{
    return value_;
}

void Category::setValue(const int value)
{
    if (value_ == value) {
        return;
    }
    value_ = value;
    emit valueChanged();
}

QString Category::name() const
{
    return name_;
}

void Category::setName(const QString& name)
{
    if (name_ == name) {
        return;
    }
    name_ = name;
    emit nameChanged();
}

QColor Category::color() const
{
    return color_;
}

void Category::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
}

} // namespace QAccelPlot
