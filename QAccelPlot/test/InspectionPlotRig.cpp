//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InspectionPlotRig.hpp"

#include <QHoverEvent>
#include <QMouseEvent>

#include <utility>

namespace InspectionTest {

PlotRig::PlotRig(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    plot.setSize({800, 400});
    x.setViewportMin(xMin);
    x.setViewportMax(xMax);
    y.setViewportMin(yMin);
    y.setViewportMax(yMax);
    plot.setXAxis(&x);
    plot.setYAxis(&y);
}

LineCurve* PlotRig::addCurve(std::vector<double> data, const QString& name)
{
    auto* curve = new LineCurve(&plot);
    curve->setName(name);
    curve->setXAxis(&x);
    curve->setYAxis(&y);
    const auto count = static_cast<int>(data.size() / 2);
    curve->setData(std::move(data), count);
    return curve;
}

QPointF PlotRig::pixel(const qreal dataX, const qreal dataY) const
{
    const auto area = plot.plotRect();
    return {area.x() + x.coordToPixel(dataX, area.width()), area.y() + y.coordToPixel(dataY, area.height())};
}

void PlotRig::hover(const QPointF& point)
{
    auto event = QHoverEvent{QEvent::HoverMove, point, point, Qt::NoModifier};
    plot.hoverMoveEvent(&event);
}

void PlotRig::leave()
{
    auto event = QHoverEvent{QEvent::HoverLeave, {}, {}, Qt::NoModifier};
    plot.hoverLeaveEvent(&event);
}

void PlotRig::drag(const QPointF& first, const QPointF& last, const Qt::KeyboardModifiers modifiers)
{
    auto press = QMouseEvent{QEvent::MouseButtonPress, first, first, Qt::LeftButton, Qt::LeftButton, modifiers};
    plot.mousePressEvent(&press);
    auto move = QMouseEvent{QEvent::MouseMove, last, last, Qt::NoButton, Qt::LeftButton, modifiers};
    plot.mouseMoveEvent(&move);
    auto release = QMouseEvent{QEvent::MouseButtonRelease, last, last, Qt::LeftButton, Qt::NoButton, modifiers};
    plot.mouseReleaseEvent(&release);
}

} // namespace InspectionTest
