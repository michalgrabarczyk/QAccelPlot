//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/AxisTicker.hpp"

#include <QColor>
#include <QFont>
#include <QHash>
#include <QQuickPaintedItem>

namespace QAccelPlot {

class PlotSeries;

/// \brief A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels.
///
/// Assign an Axis to the corresponding property of \c PlotView (\c xAxis, \c yAxis, etc.)
/// to connect it to the plot. Tick appearance is configured via the \c ticker property.
/// Supports linear and logarithmic scaling, mouse-driven pan/zoom, and custom label formatters.
/// Set the inherited \c visible property to \c false to hide the axis and reclaim its layout
/// space while preserving its range, coordinate mapping, and plot-wide navigation behavior.
///
/// \sa PlotView, AxisTicker, TickLabelFormatter
class Axis : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(Axis)
    /// \brief Lower bound of the visible range. Default: 0.
    Q_PROPERTY(qreal viewportMin READ viewportMin WRITE setViewportMin NOTIFY viewportMinChanged)
    /// \brief Upper bound of the visible range. Default: 1.
    Q_PROPERTY(qreal viewportMax READ viewportMax WRITE setViewportMax NOTIFY viewportMaxChanged)
    /// \brief Minimum data value seen by the curves bound to this axis. Default: 0.
    Q_PROPERTY(qreal dataMin READ dataMin WRITE setDataMin NOTIFY dataMinChanged)
    /// \brief Maximum data value seen by the curves bound to this axis. Default: 1.
    Q_PROPERTY(qreal dataMax READ dataMax WRITE setDataMax NOTIFY dataMaxChanged)
    /// \brief Axis orientation: \c Axis.Horizontal or \c Axis.Vertical.
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    /// \brief Optional axis label displayed alongside the axis line.
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    /// \brief Font used to render the axis label text. Default: application default font.
    Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont NOTIFY labelFontChanged)
    /// \brief Read-only: \c true while the mouse cursor is over the axis widget.
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    /// \brief Color of the axis baseline. Default: \c Qt::black.
    Q_PROPERTY(QColor baselineColor READ baselineColor WRITE setBaselineColor NOTIFY baselineColorChanged)
    /// \brief Optional axis-label color. When invalid, the baselineColor is used.
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged)
    /// \brief Width in pixels of the axis baseline. Default: 2.
    Q_PROPERTY(qreal baselineWidth READ baselineWidth WRITE setBaselineWidth NOTIFY baselineWidthChanged)
    /// \brief Tick and label color applied when the axis is hovered. Default: \c Qt::blue.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    /// \brief Side of the plot on which the axis is drawn: Left, Top, Right, or Bottom. Assigned automatically for xAxis, yAxis, x2Axis,
    /// and y2Axis; set explicitly for an axis placed in extraAxes.
    Q_PROPERTY(Side side READ side WRITE setSide NOTIFY sideChanged)
    /// \brief Pixels of padding between the axis line and the title text. Default: 30.
    Q_PROPERTY(int axisTitlePadding READ axisTitlePadding WRITE setAxisTitlePadding NOTIFY axisTitlePaddingChanged)
    /// \brief Pixels of padding between the plot area edge and the axis baseline. Default: 0.
    Q_PROPERTY(int axisLinePadding READ axisLinePadding WRITE setAxisLinePadding NOTIFY axisLinePaddingChanged)
    /// \brief Layout thickness in pixels: width for vertical axes and height for horizontal axes. Default: 50.
    Q_PROPERTY(qreal layoutSize READ layoutSize WRITE setLayoutSize NOTIFY layoutSizeChanged)
    /// \brief Read-only constant: tick appearance configuration object.
    Q_PROPERTY(AxisTicker* ticker READ ticker CONSTANT)
    /// \brief Whether the axis uses a base-10 logarithmic scale. Default: \c false.
    Q_PROPERTY(bool logScale READ logScale WRITE setLogScale NOTIFY logScaleChanged)
    /// \brief Fraction by which the visible range is multiplied on each inward mouse-wheel step. Must be in (0, 1). Default: 0.9.
    ///
    /// On a zoom-in step the range is multiplied by this value; on zoom-out by its reciprocal.
    /// Decrease the factor (e.g. 0.5) to zoom faster; increase it toward 1.0 (e.g. 0.95) to zoom more slowly.
    Q_PROPERTY(double zoomScaleFactor READ zoomScaleFactor WRITE setZoomScaleFactor NOTIFY zoomScaleFactorChanged)

public:
    /// \brief Orientation of an axis.
    enum Orientation { Horizontal, Vertical };
    Q_ENUM(Orientation)

    /// \brief Side of the plot on which the axis is positioned.
    enum Side { Left, Top, Right, Bottom };
    Q_ENUM(Side)

    /// \brief Constructs an Axis with the given \a parent and initial \a side.
    explicit Axis(QQuickItem* parent = nullptr, Side side = Bottom);

    /// \brief Returns the lower bound of the visible range.
    qreal viewportMin() const;
    /// \brief Sets the lower bound of the visible range to \a m.
    void setViewportMin(qreal m);

    /// \brief Returns the upper bound of the visible range.
    qreal viewportMax() const;
    /// \brief Sets the upper bound of the visible range to \a m.
    void setViewportMax(qreal m);

    /// \brief Returns the minimum data value tracked by bound curves.
    qreal dataMin() const;
    /// \brief Sets the tracked data minimum to \a m.
    void setDataMin(qreal m);

    /// \brief Returns the maximum data value tracked by bound curves.
    qreal dataMax() const;
    /// \brief Sets the tracked data maximum to \a m.
    void setDataMax(qreal m);

    /// \brief Returns the axis orientation.
    Orientation orientation() const;
    /// \brief Sets the axis orientation to \a o.
    void setOrientation(Orientation o);

    /// \brief Returns the axis label string.
    QString label() const;
    /// \brief Sets the axis label to \a t.
    void setLabel(const QString& t);

    /// \brief Returns the axis label font.
    QFont labelFont() const;
    /// \brief Sets the axis label font to \a f.
    void setLabelFont(const QFont& f);

    /// \brief Returns the axis side.
    Side side() const;
    /// \brief Sets the axis side to \a s.
    void setSide(Side s);

    /// \brief Returns \c true if the mouse is currently over the axis widget.
    bool hovered() const;

    /// \brief Returns the axis baseline color.
    QColor baselineColor() const;
    /// \brief Sets the axis baseline color to \a c.
    void setBaselineColor(const QColor& c);

    /// \brief Returns the axis-label color, or an invalid color when it follows baselineColor.
    QColor labelColor() const;
    /// \brief Sets the axis-label color to \a c. An invalid color restores the baselineColor fallback.
    void setLabelColor(const QColor& c);

    /// \brief Returns the axis baseline width in pixels.
    qreal baselineWidth() const;
    /// \brief Sets the axis baseline width to \a width pixels. Negative values are clamped to zero.
    void setBaselineWidth(qreal width);

    /// \brief Returns the hovered tick/label color.
    QColor hoverColor() const;
    /// \brief Sets the hovered color to \a c.
    void setHoverColor(const QColor& c);

    /// \brief Returns the axis title padding in pixels.
    int axisTitlePadding() const;
    /// \brief Sets the axis title padding to \a padding pixels.
    void setAxisTitlePadding(int padding);

    /// \brief Returns the axis line padding in pixels.
    int axisLinePadding() const;
    /// \brief Sets the axis line padding to \a padding pixels.
    void setAxisLinePadding(int padding);

    /// \brief Returns the layout thickness in pixels.
    qreal layoutSize() const;
    /// \brief Sets the layout thickness to \a size pixels. Negative values are clamped to zero.
    void setLayoutSize(qreal size);

    /// \brief Returns the tick configuration object.
    AxisTicker* ticker() const;

    /// \brief Returns \c true when log scale is active.
    bool logScale() const;
    /// \brief Sets log-scale mode to \a on.
    void setLogScale(bool on);
    /// \brief Returns the zoom scale factor.
    double zoomScaleFactor() const;
    /// \brief Sets the zoom scale factor to \a factor (clamped to the range (0, 1)).
    ///
    /// Smaller values (e.g. 0.5) produce faster zooming; values closer to 1.0 (e.g. 0.95) produce slower zooming.
    void setZoomScaleFactor(double factor);

    /// \brief Toggles the log-scale mode on or off.
    Q_INVOKABLE void toggleLogScale();

    /// \brief Sets \c viewportMin and \c viewportMax to the current \c dataMin / \c dataMax range.
    Q_INVOKABLE void rescaleToData();

    /// \brief Paints the axis widget (tick marks, labels, label text, background).
    void paint(QPainter* painter) override;

    /// \brief Returns the inward tick overlap beyond the axis line padding, used by PlotView to size the plot area.
    qreal inwardTickOverlap() const;

    /// \brief Returns extra width/height needed to accommodate edge tick labels that extend beyond the axis bounds.
    qreal labelOverflow() const;

    /// \brief Maps a data-space \a value to a pixel position along an axis of \a length pixels.
    qreal coordToPixel(qreal value, qreal length) const;
    /// \brief Maps a pixel \a pos along an axis of \a length pixels back to a data-space value.
    qreal pixelToCoord(qreal pos, qreal length) const;

public slots:
    /// \brief Replaces the tracked data range.
    /// \param min New minimum data value.
    /// \param max New maximum data value.
    void updateDataRange(qreal min, qreal max);

signals:
    /// \brief Emitted when the viewportMin property changes.
    void viewportMinChanged();
    /// \brief Emitted when the viewportMax property changes.
    void viewportMaxChanged();
    /// \brief Emitted when the dataMin property changes.
    void dataMinChanged();
    /// \brief Emitted when the dataMax property changes.
    void dataMaxChanged();
    /// \brief Emitted when the orientation property changes.
    void orientationChanged();
    /// \brief Emitted when the label property changes.
    void labelChanged();
    /// \brief Emitted when the labelFont property changes.
    void labelFontChanged();
    /// \brief Emitted when the hovered property changes.
    void hoveredChanged();
    /// \brief Emitted when either viewportMin or viewportMax changes.
    void rangeChanged();

    /// \brief Emitted when the baselineColor property changes.
    void baselineColorChanged();
    /// \brief Emitted when the labelColor property changes.
    void labelColorChanged();
    /// \brief Emitted when the baselineWidth property changes.
    void baselineWidthChanged();
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
    /// \brief Emitted when the side property changes.
    void sideChanged();
    /// \brief Emitted when the axisTitlePadding property changes.
    void axisTitlePaddingChanged();
    /// \brief Emitted when the axisLinePadding property changes.
    void axisLinePaddingChanged();
    /// \brief Emitted when the layoutSize property changes.
    void layoutSizeChanged();
    /// \brief Emitted when the logScale property changes.
    void logScaleChanged();
    /// \brief Emitted when the zoomScaleFactor property changes.
    void zoomScaleFactorChanged();
    /// \brief Emitted when the user double-clicks the axis widget.
    void doubleClicked();

protected:
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    friend class PlotSeries;

    struct DataRange {
        qreal min;
        qreal max;
    };

    void setSourceDataRange(const QObject* source, Orientation dimension, qreal min, qreal max);
    void clearSourceDataRange(const QObject* source, Orientation dimension);
    void recomputeSourceDataRange();
    void setDataRangeValues(qreal min, qreal max);
    void paintLabel(QPainter* painter, const QRectF& r, qreal axisX, qreal axisY) const;

    qreal viewportMin_{0.0};
    qreal viewportMax_{1.0};
    qreal dataMin_{0.0};
    qreal dataMax_{1.0};
    QHash<const QObject*, DataRange> horizontalDataRanges_;
    QHash<const QObject*, DataRange> verticalDataRanges_;
    QHash<const QObject*, QMetaObject::Connection> rangeSourceConnections_;
    QString label_;
    QFont labelFont_;
    bool hovered_{false};
    bool isDragging_{false};
    QPointF lastMousePos_;
    QColor baselineColor_{QColor(Qt::black)};
    QColor labelColor_;
    qreal baselineWidth_{2.0};
    QColor hoverColor_{QColor(Qt::blue)};
    int axisTitlePadding_{30};
    int axisLinePadding_{0};
    qreal layoutSize_{50.0};
    bool logScale_{false};
    double zoomScaleFactor_{0.9};
    bool extendWidgetForLabels_{true};
    bool clampEdgeLabels_{true};
    Orientation orientation_{Horizontal};
    Side side_{Bottom};
    AxisTicker* ticker_;
};

} // namespace QAccelPlot
