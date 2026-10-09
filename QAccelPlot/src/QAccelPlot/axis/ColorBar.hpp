//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/axis/AxisTicks.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QColor>
#include <QFont>
#include <QList>
#include <QMetaObject>
#include <QPointF>
#include <QPointer>
#include <QQuickPaintedItem>

#include <vector>

namespace QAccelPlot {

/// \brief A continuous key that shows how a series' \c Colormap maps values to colors.
///
/// Draws the series' color ramp as a strip, with ticks and labels for the value range the series
/// resolved (\c PointCloud::dataValueMin and \c PointCloud::dataValueMax). Ticks follow the
/// colormap's \c norm, so a \c Log colormap gets decade ticks. The bar redraws when the colormap, the
/// resolved range, or the assigned colormap changes. Nothing is drawn while the series has no
/// colormap.
///
/// A vertical bar has its minimum at the bottom, with ticks on the right and \c label beside them,
/// reading top to bottom.
/// A horizontal bar has its minimum on the left, with ticks and \c label below. Position the bar
/// like any item, for example inside the plot area:
///
/// \code
/// QAccelPlot.ColorBar {
///     series: cloud
///     label: "Intensity"
///     x: plot.plotRect.right - width - 8
///     y: plot.plotRect.y + 8
/// }
/// \endcode
///
/// The implicit size fits the strip, ticks, tick labels, and \c label, with a length of 160 pixels
/// along the ramp. A vertical bar gives its tick labels the fixed \c tickLabelWidth and clips wider
/// ones. Tick labels at the ends are kept inside the item.
///
/// With \c interactive set, a left drag along the ramp pans the colormap's value range, the mouse
/// wheel zooms it around the value under the cursor, and a double-click calls \c rescaleToData().
/// The gestures write \c Colormap::min and \c Colormap::max, so every series sharing the colormap
/// follows. A QML binding on either bound overrides them when it is next evaluated.
///
/// \sa PointCloud, Colormap, AxisTicker
class ColorBar : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(ColorBar)

    /// \brief Series whose colormap and resolved value range are shown. Default: null.
    Q_PROPERTY(PointCloud* series READ series WRITE setSeries NOTIFY seriesChanged)
    /// \brief Direction of the ramp: \c ColorBar.Vertical or \c ColorBar.Horizontal. Default: \c Vertical.
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    /// \brief Optional title drawn beside the tick labels. Default: empty.
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    /// \brief Font of the title. Default: application default font.
    Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont NOTIFY labelFontChanged)
    /// \brief Color of the title. Default: \c Colors.dark.axisLine.
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged)
    /// \brief Gap in pixels between the tick labels and the title. Default: 6.
    Q_PROPERTY(qreal labelPadding READ labelPadding WRITE setLabelPadding NOTIFY labelPaddingChanged)
    /// \brief Thickness in pixels of the color strip across the ramp direction. Default: 12.
    Q_PROPERTY(qreal barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    /// \brief Color of the outline around the strip. Default: \c Colors.dark.axisLine.
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    /// \brief Width in pixels of the outline around the strip. 0 draws no outline. Default: 1.
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    /// \brief Width in pixels reserved for the tick labels of a vertical bar. Default: 16.
    ///
    /// The width is fixed, so the implicit width and the title stay in place while the labels
    /// change. Labels wider than it are clipped. Ignored by horizontal bars.
    Q_PROPERTY(qreal tickLabelWidth READ tickLabelWidth WRITE setTickLabelWidth NOTIFY tickLabelWidthChanged)
    /// \brief Whether dragging, the mouse wheel, and double-clicking change the colormap's value range. Default: \c false.
    ///
    /// While \c false, the bar leaves mouse and wheel events to the items below it.
    Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY interactiveChanged)
    /// \brief Fraction by which the value range is multiplied on each inward mouse-wheel step. Must be in (0, 1). Default: 0.9.
    ///
    /// On a zoom-out step the range is multiplied by the reciprocal. Used only while \c interactive is set.
    Q_PROPERTY(double zoomScaleFactor READ zoomScaleFactor WRITE setZoomScaleFactor NOTIFY zoomScaleFactorChanged)
    /// \brief Read-only: \c true while the mouse cursor is over the bar. Reported only while \c interactive is set.
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    /// \brief Read-only constant: tick appearance, count, and label formatter.
    ///
    /// Defaults differ from \c Axis: \c tickLengthIn 0, \c tickLengthOut 4, \c subtickLengthIn 0,
    /// \c subtickLengthOut 2, \c subtickCount 0, and \c tickWidth 1.
    Q_PROPERTY(AxisTicker* ticker READ ticker CONSTANT)

public:
    /// \brief Direction of the color ramp.
    enum Orientation {
        Horizontal, ///< \brief Minimum on the left, ticks below.
        Vertical    ///< \brief Minimum at the bottom, ticks on the right.
    };
    Q_ENUM(Orientation)

    /// \brief Constructs a ColorBar with the given \a parent.
    explicit ColorBar(QQuickItem* parent = nullptr);

    /// \brief Returns the series whose colormap is shown, or \c nullptr.
    PointCloud* series() const;
    /// \brief Sets the series whose colormap is shown to \a series.
    void setSeries(PointCloud* series);

    /// \brief Returns the ramp direction.
    Orientation orientation() const;
    /// \brief Sets the ramp direction to \a orientation.
    void setOrientation(Orientation orientation);

    /// \brief Returns the title.
    QString label() const;
    /// \brief Sets the title to \a label.
    void setLabel(const QString& label);

    /// \brief Returns the title font.
    QFont labelFont() const;
    /// \brief Sets the title font to \a font.
    void setLabelFont(const QFont& font);

    /// \brief Returns the title color.
    QColor labelColor() const;
    /// \brief Sets the title color to \a color.
    void setLabelColor(const QColor& color);

    /// \brief Returns the gap between the tick labels and the title in pixels.
    qreal labelPadding() const;
    /// \brief Sets the gap between the tick labels and the title to \a padding pixels. Negative values are clamped to 0.
    void setLabelPadding(qreal padding);

    /// \brief Returns the strip thickness in pixels.
    qreal barThickness() const;
    /// \brief Sets the strip thickness to \a thickness pixels. Negative values are clamped to 0.
    void setBarThickness(qreal thickness);

    /// \brief Returns the strip outline color.
    QColor borderColor() const;
    /// \brief Sets the strip outline color to \a color.
    void setBorderColor(const QColor& color);

    /// \brief Returns the strip outline width in pixels.
    qreal borderWidth() const;
    /// \brief Sets the strip outline width to \a width pixels. Negative values are clamped to 0.
    void setBorderWidth(qreal width);

    /// \brief Returns the width reserved for tick labels in pixels.
    qreal tickLabelWidth() const;
    /// \brief Sets the width reserved for tick labels to \a width pixels. Negative values are clamped to 0.
    void setTickLabelWidth(qreal width);

    /// \brief Returns whether the gestures that change the colormap's value range are enabled.
    bool interactive() const;
    /// \brief Sets whether the gestures that change the colormap's value range are enabled to \a interactive.
    void setInteractive(bool interactive);

    /// \brief Returns the zoom scale factor.
    double zoomScaleFactor() const;
    /// \brief Sets the zoom scale factor to \a factor (clamped to the range (0, 1)).
    void setZoomScaleFactor(double factor);

    /// \brief Returns \c true while the mouse cursor is over an interactive bar.
    bool hovered() const;

    /// \brief Returns the tick configuration object. The object is owned by the color bar.
    AxisTicker* ticker() const;

    /// \brief Unsets the colormap's \c min and \c max, so the range is resolved from the data again.
    Q_INVOKABLE void rescaleToData();

    /// \brief Maps \a value to a pixel position along a strip of \a length pixels.
    ///
    /// Horizontal bars measure from the left edge, vertical bars from the top edge. Uses the range and
    /// normalization captured at the last polish, so it matches what \c paint() draws.
    qreal valueToPixel(qreal value, qreal length) const;
    /// \brief Maps a \a pixel position along a strip of \a length pixels back to a value.
    ///
    /// The inverse of \c valueToPixel(). Returns the range minimum when \a length is 0.
    qreal pixelToValue(qreal pixel, qreal length) const;

    /// \brief Paints the strip, ticks, tick labels, and title computed in \c updatePolish().
    void paint(QPainter* painter) override;

signals:
    /// \brief Emitted when the series property changes.
    void seriesChanged();
    /// \brief Emitted when the orientation property changes.
    void orientationChanged();
    /// \brief Emitted when the label property changes.
    void labelChanged();
    /// \brief Emitted when the labelFont property changes.
    void labelFontChanged();
    /// \brief Emitted when the labelColor property changes.
    void labelColorChanged();
    /// \brief Emitted when the labelPadding property changes.
    void labelPaddingChanged();
    /// \brief Emitted when the barThickness property changes.
    void barThicknessChanged();
    /// \brief Emitted when the borderColor property changes.
    void borderColorChanged();
    /// \brief Emitted when the borderWidth property changes.
    void borderWidthChanged();
    /// \brief Emitted when the tickLabelWidth property changes.
    void tickLabelWidthChanged();
    /// \brief Emitted when the interactive property changes.
    void interactiveChanged();
    /// \brief Emitted when the zoomScaleFactor property changes.
    void zoomScaleFactorChanged();
    /// \brief Emitted when the hovered property changes.
    void hoveredChanged();

protected:
    /// \brief Marks the bar as hovered.
    void hoverEnterEvent(QHoverEvent* event) override;
    /// \brief Clears the hovered state.
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \brief Starts panning the value range on a left-button press while \c interactive is set.
    void mousePressEvent(QMouseEvent* event) override;
    /// \brief Pans the value range by the distance dragged along the ramp.
    void mouseMoveEvent(QMouseEvent* event) override;
    /// \brief Ends panning on a left-button release.
    void mouseReleaseEvent(QMouseEvent* event) override;
    /// \brief Calls \c rescaleToData() on a left-button double-click while \c interactive is set.
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    /// \brief Ends panning when the mouse grab is lost.
    void mouseUngrabEvent() override;
    /// \brief Zooms the value range around the value under the cursor while \c interactive is set.
    void wheelEvent(QWheelEvent* event) override;
    /// \brief Captures the colormap, formats tick labels, and lays out the bar on the GUI thread, ahead of \c paint().
    void updatePolish() override;
    /// \brief Schedules a new layout when the item is resized.
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    struct Layout {
        QRectF strip;
        QRectF tickArea;
        QRectF title;
        // Area that the tick marks and labels of a vertical bar may paint in.
        QRectF tickClip;
        qreal endInset{0.0};
    };

    void setHovered(bool hovered);
    Colormap* seriesColormap() const;
    qreal stripLength() const;
    void panRange(qreal fraction);
    void zoomRange(qreal factor, qreal anchorRatio);
    void reconnectSeries();
    void reconnectColormap();
    void invalidate();
    void captureColormap();
    void updateImplicitSize();
    Layout computeLayout() const;
    qreal endInset() const;
    qreal tickLabelHeight() const;
    qreal tickLabelThickness() const;
    qreal titleThickness() const;
    qreal titleOffset() const;
    void paintStrip(QPainter* painter) const;
    void paintTicks(QPainter* painter) const;
    void paintTitle(QPainter* painter) const;

    QPointer<PointCloud> series_;
    QList<QMetaObject::Connection> seriesConnections_;
    QMetaObject::Connection colormapConnection_;
    Orientation orientation_{Vertical};
    QString label_;
    QFont labelFont_;
    QColor labelColor_{ColorPalette::dark().axisLine};
    qreal labelPadding_{6.0};
    qreal barThickness_{12.0};
    QColor borderColor_{ColorPalette::dark().axisLine};
    qreal borderWidth_{1.0};
    qreal tickLabelWidth_{16.0};
    bool interactive_{false};
    double zoomScaleFactor_{0.9};
    bool hovered_{false};
    AxisTicker* ticker_;
    bool isDragging_{false};
    QPointF lastMousePos_;

    // Captured on the GUI thread in updatePolish() and read by paint(), which may run on the render thread.
    std::vector<GradientStopData> stops_;
    qreal valueMin_{0.0};
    qreal valueMax_{1.0};
    bool logScale_{false};
    AxisTicks ticks_;
    Layout layout_;
};

} // namespace QAccelPlot
