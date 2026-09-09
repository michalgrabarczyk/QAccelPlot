//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "grid/Grid.hpp"

#include "MathUtils.hpp"

namespace QAccelPlot {

Grid::Grid(QObject* parent)
    : QObject(parent)
{
}

qreal Grid::lineWidth() const
{
    return lineWidth_;
}

void Grid::setLineWidth(qreal w)
{
    if (nearly_equal(lineWidth_, w)) {
        return;
    }
    lineWidth_ = w;
    emit lineWidthChanged();
}

qreal Grid::subGridLineWidth() const
{
    return subGridLineWidth_;
}

void Grid::setSubGridLineWidth(qreal w)
{
    if (nearly_equal(subGridLineWidth_, w)) {
        return;
    }
    subGridLineWidth_ = w;
    emit subGridLineWidthChanged();
}

QColor Grid::gridColor() const
{
    return gridColor_;
}

void Grid::setGridColor(const QColor& c)
{
    if (gridColor_ == c) {
        return;
    }
    gridColor_ = c;
    emit gridColorChanged();
}

QColor Grid::subGridColor() const
{
    return subGridColor_;
}

void Grid::setSubGridColor(const QColor& c)
{
    if (subGridColor_ == c) {
        return;
    }
    subGridColor_ = c;
    emit subGridColorChanged();
}

bool Grid::gridVisible() const
{
    return gridVisible_;
}

void Grid::setGridVisible(const bool on)
{
    if (gridVisible_ == on) {
        return;
    }
    gridVisible_ = on;
    emit gridVisibleChanged();
}

bool Grid::subGridVisible() const
{
    return subGridVisible_;
}

void Grid::setSubGridVisible(const bool on)
{
    if (subGridVisible_ == on) {
        return;
    }
    subGridVisible_ = on;
    emit subGridVisibleChanged();
}

bool Grid::gridHorizontalLinesVisible() const
{
    return gridHorizontalLinesVisible_;
}

void Grid::setGridHorizontalLinesVisible(const bool on)
{
    if (gridHorizontalLinesVisible_ == on) {
        return;
    }
    gridHorizontalLinesVisible_ = on;
    emit gridHorizontalLinesVisibleChanged();
}

bool Grid::gridVerticalLinesVisible() const
{
    return gridVerticalLinesVisible_;
}

void Grid::setGridVerticalLinesVisible(const bool on)
{
    if (gridVerticalLinesVisible_ == on) {
        return;
    }
    gridVerticalLinesVisible_ = on;
    emit gridVerticalLinesVisibleChanged();
}

bool Grid::subGridHorizontalLinesVisible() const
{
    return subGridHorizontalLinesVisible_;
}

void Grid::setSubGridHorizontalLinesVisible(const bool on)
{
    if (subGridHorizontalLinesVisible_ == on) {
        return;
    }
    subGridHorizontalLinesVisible_ = on;
    emit subGridHorizontalLinesVisibleChanged();
}

bool Grid::subGridVerticalLinesVisible() const
{
    return subGridVerticalLinesVisible_;
}

void Grid::setSubGridVerticalLinesVisible(const bool on)
{
    if (subGridVerticalLinesVisible_ == on) {
        return;
    }
    subGridVerticalLinesVisible_ = on;
    emit subGridVerticalLinesVisibleChanged();
}

} // namespace QAccelPlot
