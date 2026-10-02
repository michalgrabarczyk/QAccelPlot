//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QAccelPlot/QAccelPlot.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <vector>

/// Test fixtures shared by the plot inspector and selection tool tests.
namespace InspectionTest {

using namespace QAccelPlot;

// Exposes the event handlers of PlotView, so tests can deliver events without a window.
class InspectionPlot : public ::QAccelPlot::QAccelPlot {
public:
    using ::QAccelPlot::QAccelPlot::hoverLeaveEvent;
    using ::QAccelPlot::QAccelPlot::hoverMoveEvent;
    using ::QAccelPlot::QAccelPlot::keyPressEvent;
    using ::QAccelPlot::QAccelPlot::mouseMoveEvent;
    using ::QAccelPlot::QAccelPlot::mousePressEvent;
    using ::QAccelPlot::QAccelPlot::mouseReleaseEvent;
    using ::QAccelPlot::QAccelPlot::mouseUngrabEvent;
};

// A windowless plot with primary axes and helpers to drive its pointer.
struct PlotRig {
    InspectionPlot plot;
    Axis x;
    Axis y;

    PlotRig(qreal xMin, qreal xMax, qreal yMin, qreal yMax);

    // Adds a curve with interleaved XY data, bound to the primary axes.
    LineCurve* addCurve(std::vector<double> data, const QString& name = {});
    // Returns the plot-local pixel of a data position.
    QPointF pixel(qreal dataX, qreal dataY) const;
    void hover(const QPointF& point);
    void leave();
    void drag(const QPointF& first, const QPointF& last, Qt::KeyboardModifiers modifiers = Qt::ShiftModifier);
};

} // namespace InspectionTest
