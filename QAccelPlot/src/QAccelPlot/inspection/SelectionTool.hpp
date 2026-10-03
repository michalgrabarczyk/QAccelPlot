//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/PlotDragRect.hpp"
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/inspection/InspectionRowModel.hpp"

#include <limits>

namespace QAccelPlot {

class SelectionRectangle;

/// \brief Selects a data-space region by dragging, without changing the plot viewport.
///
/// The tool draws the gesture and the selected region in the plot's overlay, below the overlay's
/// other children.
/// The region is kept in data coordinates, so it follows panning and zooming and survives data
/// updates: the model rows are recomputed whenever a series changes. An infinite limit leaves
/// that side of the region unbounded, as a range selection does for its other dimension.
///
/// \sa SeriesInspection, InspectionRowModel
class SelectionTool : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SelectionTool)
    /// \brief Plot receiving the selection gestures.
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    /// \brief Axis the X limits refer to; unset uses the plot's primary X axis.
    Q_PROPERTY(::QAccelPlot::Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    /// \brief Axis the Y limits refer to; unset uses the plot's primary Y axis.
    Q_PROPERTY(::QAccelPlot::Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    /// \brief Enables gestures; disabling cancels a gesture in progress and keeps the selection. Default: true.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// \brief Region a gesture selects: a box, an X range, or a Y range. Default: Box.
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    /// \brief Mouse button that starts a gesture. Default: Qt.LeftButton.
    Q_PROPERTY(int button READ button WRITE setButton NOTIFY buttonChanged)
    /// \brief Exact Qt::KeyboardModifiers required at press time. Default: Qt.ShiftModifier.
    Q_PROPERTY(int modifiers READ modifiers WRITE setModifiers NOTIFY modifiersChanged)
    /// \brief Smallest gesture in logical pixels that selects; a smaller one only clears the selection. Default: 6.
    Q_PROPERTY(qreal minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    /// \brief Fill color of the rectangle. Default: \c Colors.dark.selectionFill.
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    /// \brief Outline color of the rectangle. Default: \c Colors.dark.selectionBorder.
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    /// \brief Draws the gesture and the selected region. Disable it to draw \c pixelRect yourself. Default: true.
    Q_PROPERTY(bool rectangleVisible READ rectangleVisible WRITE setRectangleVisible NOTIFY rectangleVisibleChanged)
    /// \brief True while a gesture is in progress.
    Q_PROPERTY(bool selecting READ selecting NOTIFY selectingChanged)
    /// \brief True while a region is selected.
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    /// \brief Gesture or selected region in plot-local logical pixels; empty without either.
    Q_PROPERTY(QRectF pixelRect READ pixelRect NOTIFY pixelRectChanged)
    /// \brief Lower X limit of the selection; -Infinity when unbounded, NaN without a selection.
    Q_PROPERTY(qreal xMin READ xMin NOTIFY selectionChanged)
    /// \brief Upper X limit of the selection; Infinity when unbounded, NaN without a selection.
    Q_PROPERTY(qreal xMax READ xMax NOTIFY selectionChanged)
    /// \brief Lower Y limit of the selection; -Infinity when unbounded, NaN without a selection.
    Q_PROPERTY(qreal yMin READ yMin NOTIFY selectionChanged)
    /// \brief Upper Y limit of the selection; Infinity when unbounded, NaN without a selection.
    Q_PROPERTY(qreal yMax READ yMax NOTIFY selectionChanged)
    /// \brief Read-only constant: one row of region statistics per selected series.
    Q_PROPERTY(::QAccelPlot::InspectionRowModel* model READ model CONSTANT)

public:
    /// \brief Region selected by a gesture.
    enum Mode {
        Box,    ///< \brief The dragged rectangle.
        XRange, ///< \brief The dragged X interval, unbounded in Y.
        YRange  ///< \brief The dragged Y interval, unbounded in X.
    };
    Q_ENUM(Mode)

    /// \brief Constructs a tool without a plot.
    explicit SelectionTool(QObject* parent = nullptr);

    /// \brief Returns the attached plot.
    ::QAccelPlot::QAccelPlot* plot() const;
    /// \brief Attaches to \a plot and clears the selection.
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    /// \brief Returns the axis the X limits refer to.
    Axis* xAxis() const;
    /// \brief Sets the X axis and clears the selection; null uses the plot's primary X axis.
    void setXAxis(Axis* axis);
    /// \brief Returns the axis the Y limits refer to.
    Axis* yAxis() const;
    /// \brief Sets the Y axis and clears the selection; null uses the plot's primary Y axis.
    void setYAxis(Axis* axis);
    /// \brief Returns whether gestures are enabled.
    bool enabled() const;
    /// \brief Enables or disables gestures.
    void setEnabled(bool enabled);
    /// \brief Returns the region a gesture selects.
    Mode mode() const;
    /// \brief Sets the region a gesture selects and cancels a gesture in progress.
    void setMode(Mode mode);
    /// \brief Returns the mouse button that starts a gesture.
    int button() const;
    /// \brief Sets the mouse button that starts a gesture.
    void setButton(int button);
    /// \brief Returns the required keyboard modifiers.
    int modifiers() const;
    /// \brief Sets the exact keyboard modifiers required at press time.
    void setModifiers(int modifiers);
    /// \brief Returns the smallest selecting gesture in logical pixels.
    qreal minimumSize() const;
    /// \brief Sets the smallest selecting gesture, clamping negative values to zero and ignoring nonfinite values.
    void setMinimumSize(qreal size);
    /// \brief Returns the rectangle's fill color.
    QColor fillColor() const;
    /// \brief Sets the rectangle's fill color.
    void setFillColor(const QColor& color);
    /// \brief Returns the rectangle's outline color.
    QColor borderColor() const;
    /// \brief Sets the rectangle's outline color.
    void setBorderColor(const QColor& color);
    /// \brief Returns whether the tool draws its rectangle.
    bool rectangleVisible() const;
    /// \brief Shows or hides the tool's rectangle.
    void setRectangleVisible(bool visible);
    /// \brief Returns true while a gesture is in progress.
    bool selecting() const;
    /// \brief Returns true while a region is selected.
    bool hasSelection() const;
    /// \brief Returns the gesture or selected region in plot-local logical pixels.
    QRectF pixelRect() const;
    /// \brief Returns the lower X limit of the selection.
    qreal xMin() const;
    /// \brief Returns the upper X limit of the selection.
    qreal xMax() const;
    /// \brief Returns the lower Y limit of the selection.
    qreal yMin() const;
    /// \brief Returns the upper Y limit of the selection.
    qreal yMax() const;
    /// \brief Returns the model of per-series region statistics.
    InspectionRowModel* model() const;

    /// \brief Selects the region with the given limits; returns false when a limit is NaN.
    Q_INVOKABLE bool select(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    /// \brief Returns one page of \a series' source indices inside the selection, against its current data.
    Q_INVOKABLE ::QAccelPlot::InspectionPage indices(::QAccelPlot::PlotSeries* series, int offset = 0, int limit = 4096);
    /// \brief Cancels a gesture in progress and clears the selection.
    Q_INVOKABLE void clear();

signals:
    /// \brief Emitted when the plot property changes.
    void plotChanged();
    /// \brief Emitted when the axis the X limits refer to changes.
    void xAxisChanged();
    /// \brief Emitted when the axis the Y limits refer to changes.
    void yAxisChanged();
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when the mode property changes.
    void modeChanged();
    /// \brief Emitted when the button property changes.
    void buttonChanged();
    /// \brief Emitted when the modifiers property changes.
    void modifiersChanged();
    /// \brief Emitted when the minimumSize property changes.
    void minimumSizeChanged();
    /// \brief Emitted when the fillColor property changes.
    void fillColorChanged();
    /// \brief Emitted when the borderColor property changes.
    void borderColorChanged();
    /// \brief Emitted when the rectangleVisible property changes.
    void rectangleVisibleChanged();
    /// \brief Emitted when a gesture starts or ends.
    void selectingChanged();
    /// \brief Emitted when the selected region is set, changed, or cleared.
    void selectionChanged();
    /// \brief Emitted when the pixelRect property changes.
    void pixelRectChanged();
    /// \brief Emitted when a gesture selected a region. Rows of series that are still preparing follow later.
    void completed();

private:
    struct Region {
        qreal xMin{std::numeric_limits<qreal>::quiet_NaN()};
        qreal xMax{std::numeric_limits<qreal>::quiet_NaN()};
        qreal yMin{std::numeric_limits<qreal>::quiet_NaN()};
        qreal yMax{std::numeric_limits<qreal>::quiet_NaN()};
    };

    void reconnect();
    void connectSeries(PlotSeries* series);
    // The limits are meaningless on a different axis, so a change of the effective axes drops the selection.
    void resetAxes();
    void press(PlotMouseEvent* event);
    void move(PlotMouseEvent* event);
    void release(PlotMouseEvent* event);
    void cancelGesture();
    void setRegion(const Region& region);
    void refreshRows();
    bool selects(const PlotSeries* series) const;
    QRectF gestureRect() const;
    QRectF regionRect() const;
    Region regionFromGesture(const QRectF& pixels) const;

    QPointer<::QAccelPlot::QAccelPlot> plot_;
    QPointer<Axis> xAxis_;
    QPointer<Axis> yAxis_;
    bool enabled_{true};
    Mode mode_{Box};
    int button_{Qt::LeftButton};
    int modifiers_{Qt::ShiftModifier};
    qreal minimumSize_{6.0};
    QColor fillColor_{ColorPalette::dark().selectionFill};
    QColor borderColor_{ColorPalette::dark().selectionBorder};
    bool rectangleVisible_{true};
    PlotDragRect drag_;
    bool hasSelection_{false};
    Region region_;
    quint64 rowsGeneration_{0};
    InspectionRowModel* model_;
    QList<QMetaObject::Connection> connections_;
    // Owned as a QObject child.
    SelectionRectangle* rectangle_;
};

} // namespace QAccelPlot
