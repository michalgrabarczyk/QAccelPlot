//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"

#include <QObject>

namespace QAccelPlot {

/// \brief Controls the markers a series draws at its data points.
///
/// Accessible via the \c marker CONSTANT grouped property of \c LineCurve and \c PointCloud, for example
/// <tt>marker.shape: QAccelPlot.LineCurve.Diamond</tt>. Defaults depend on the owning series.
///
/// \sa LineCurve, PointCloud
class SeriesMarker : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Shape drawn at each data point. Series that always draw markers, such as \c PointCloud, ignore
    /// \c MarkerShape.None and log a warning. Default: \c None on \c LineCurve, \c Circle on \c PointCloud.
    Q_PROPERTY(PlotSeries::MarkerShape shape READ shape WRITE setShape NOTIFY shapeChanged)
    /// \brief Radius of each marker in pixels, clamped to at least 0. Default: 4 on \c LineCurve, 3 on \c PointCloud.
    Q_PROPERTY(qreal size READ size WRITE setSize NOTIFY sizeChanged)
    /// \brief Whether closed marker shapes are filled. When \c false they are drawn as outlines of \c strokeWidth
    /// inside the shape's edge. Line-like shapes (\c Cross, \c XCross, \c Asterisk, \c HLine, \c VLine) and \c Pixel
    /// are unaffected. Default: \c true.
    Q_PROPERTY(bool filled READ filled WRITE setFilled NOTIFY filledChanged)
    /// \brief Outline width in pixels of hollow markers, clamped to at least 0. Has effect only when \c filled is
    /// \c false. Default: 1.
    Q_PROPERTY(qreal strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)

public:
    /// \brief Whether \c MarkerShape.None is a valid shape for the owning series.
    enum class NoneShape { Accepted, Rejected };

    /// \brief Constructs a SeriesMarker with the default \a shape and \a size, owned by \a parent.
    SeriesMarker(PlotSeries::MarkerShape shape, qreal size, NoneShape noneShape, QObject* parent);

    /// \brief Returns the marker shape.
    PlotSeries::MarkerShape shape() const;
    /// \brief Sets the marker shape to \a shape. \c MarkerShape.None is ignored when the series rejects it.
    void setShape(PlotSeries::MarkerShape shape);

    /// \brief Returns the marker radius in pixels.
    qreal size() const;
    /// \brief Sets the marker radius to \a size pixels. Negative values are clamped to 0.
    void setSize(qreal size);

    /// \brief Returns \c true if closed marker shapes are filled.
    bool filled() const;
    /// \brief Sets whether closed marker shapes are filled (\a filled) or drawn as outlines.
    void setFilled(bool filled);

    /// \brief Returns the outline width of hollow markers in pixels.
    qreal strokeWidth() const;
    /// \brief Sets the outline width of hollow markers to \a width pixels. Negative values are clamped to 0.
    void setStrokeWidth(qreal width);

signals:
    /// \brief Emitted when the shape property changes.
    void shapeChanged();
    /// \brief Emitted when the size property changes.
    void sizeChanged();
    /// \brief Emitted when the filled property changes.
    void filledChanged();
    /// \brief Emitted when the strokeWidth property changes.
    void strokeWidthChanged();

private:
    PlotSeries::MarkerShape shape_;
    qreal size_;
    NoneShape noneShape_;
    bool filled_{true};
    qreal strokeWidth_{1.0};
};

} // namespace QAccelPlot
