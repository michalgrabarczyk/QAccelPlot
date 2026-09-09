//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"

#include <QQuickItem>
#include <QRectF>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

/// \brief A QQuickItem that tracks a data-coordinate rectangle in pixel space.
///
/// Declare DataAnchor as a direct child of PlotView and nest any visual QML items
/// (Rectangle, Text, HoverHandler, TapHandler …) inside it — they will follow pan
/// and zoom automatically as the axes change.
///
/// \par Usage patterns
/// - **Point annotation**: set \c dataX1 == \c dataX2 and \c dataY1 == \c dataY2.
///   The item has zero size but contains() still responds within hoverThreshold.
/// - **Vertical line**: set \c dataX1 == \c dataX2 and different Y values.
///   The item has zero width but a non-zero height.
/// - **Rectangle / range band**: supply four distinct data coordinates.
///
/// The contains() override expands the logical hit area by hoverThreshold in every
/// direction, so HoverHandler and TapHandler work correctly on zero-size anchors.
///
/// \sa Axis
class DataAnchor : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(DataAnchor)

    /// \brief The horizontal axis used to map data X coordinates to pixel positions.
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)

    /// \brief The vertical axis used to map data Y coordinates to pixel positions.
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)

    /// \brief The pixel rectangle of the plot canvas, used together with the axes to
    ///        compute the item's position and size.
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)

    /// \brief X data coordinate of the first (left) edge of the anchor.
    Q_PROPERTY(qreal dataX1 READ dataX1 WRITE setDataX1 NOTIFY dataX1Changed)

    /// \brief Y data coordinate of the first (bottom or top) edge of the anchor.
    Q_PROPERTY(qreal dataY1 READ dataY1 WRITE setDataY1 NOTIFY dataY1Changed)

    /// \brief X data coordinate of the second (right) edge of the anchor.
    Q_PROPERTY(qreal dataX2 READ dataX2 WRITE setDataX2 NOTIFY dataX2Changed)

    /// \brief Y data coordinate of the second (top or bottom) edge of the anchor.
    Q_PROPERTY(qreal dataY2 READ dataY2 WRITE setDataY2 NOTIFY dataY2Changed)

    /// \brief Radius in pixels by which contains() expands the logical hit area.
    ///
    /// Useful for zero-size anchors (point or line) where the item's bounding rect
    /// would otherwise have no area to interact with.
    Q_PROPERTY(qreal hoverThreshold READ hoverThreshold WRITE setHoverThreshold NOTIFY hoverThresholdChanged)

public:
    /// \brief Constructs a DataAnchor with the given \a parent.
    explicit DataAnchor(QQuickItem* parent = nullptr);

    /// \brief Returns the horizontal axis.
    Axis* xAxis() const;
    /// \brief Sets the horizontal axis to \a axis, reconnecting range-change tracking.
    void setXAxis(Axis* axis);

    /// \brief Returns the vertical axis.
    Axis* yAxis() const;
    /// \brief Sets the vertical axis to \a axis, reconnecting range-change tracking.
    void setYAxis(Axis* axis);

    /// \brief Returns the plot canvas pixel rectangle.
    QRectF plotRect() const;
    /// \brief Sets the plot canvas pixel rectangle to \a rect, triggering a geometry update.
    void setPlotRect(const QRectF& rect);

    /// \brief Returns the X data coordinate of the first edge.
    qreal dataX1() const;
    /// \brief Sets the X data coordinate of the first edge to \a value.
    void setDataX1(qreal value);

    /// \brief Returns the Y data coordinate of the first edge.
    qreal dataY1() const;
    /// \brief Sets the Y data coordinate of the first edge to \a value.
    void setDataY1(qreal value);

    /// \brief Returns the X data coordinate of the second edge.
    qreal dataX2() const;
    /// \brief Sets the X data coordinate of the second edge to \a value.
    void setDataX2(qreal value);

    /// \brief Returns the Y data coordinate of the second edge.
    qreal dataY2() const;
    /// \brief Sets the Y data coordinate of the second edge to \a value.
    void setDataY2(qreal value);

    /// \brief Returns the hover threshold in pixels.
    qreal hoverThreshold() const;
    /// \brief Sets the hover threshold to \a threshold pixels.
    void setHoverThreshold(qreal threshold);

    /// \brief Returns \c true if \a point falls within the item's bounding rect
    ///        expanded by hoverThreshold on all sides.
    /// \param point Point in item-local coordinates.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the xAxis property changes.
    void xAxisChanged();
    /// \brief Emitted when the yAxis property changes.
    void yAxisChanged();
    /// \brief Emitted when the plotRect property changes.
    void plotRectChanged();
    /// \brief Emitted when the dataX1 property changes.
    void dataX1Changed();
    /// \brief Emitted when the dataY1 property changes.
    void dataY1Changed();
    /// \brief Emitted when the dataX2 property changes.
    void dataX2Changed();
    /// \brief Emitted when the dataY2 property changes.
    void dataY2Changed();
    /// \brief Emitted when the hoverThreshold property changes.
    void hoverThresholdChanged();

private:
    void updateGeometry();

    Axis* xAxis_{nullptr};
    Axis* yAxis_{nullptr};
    QRectF plotRect_;
    qreal dataX1_{0.0};
    qreal dataY1_{0.0};
    qreal dataX2_{0.0};
    qreal dataY2_{0.0};
    qreal hoverThreshold_{8.0};
};

} // namespace QAccelPlot
