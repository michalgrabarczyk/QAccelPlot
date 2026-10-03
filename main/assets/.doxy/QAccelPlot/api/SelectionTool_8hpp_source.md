

# File SelectionTool.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**SelectionTool.hpp**](SelectionTool_8hpp.md)

[Go to the documentation of this file](SelectionTool_8hpp.md)


```C++
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

class SelectionTool : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SelectionTool)
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    Q_PROPERTY(::QAccelPlot::Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(::QAccelPlot::Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int button READ button WRITE setButton NOTIFY buttonChanged)
    Q_PROPERTY(int modifiers READ modifiers WRITE setModifiers NOTIFY modifiersChanged)
    Q_PROPERTY(qreal minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(bool rectangleVisible READ rectangleVisible WRITE setRectangleVisible NOTIFY rectangleVisibleChanged)
    Q_PROPERTY(bool selecting READ selecting NOTIFY selectingChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(QRectF pixelRect READ pixelRect NOTIFY pixelRectChanged)
    Q_PROPERTY(qreal xMin READ xMin NOTIFY selectionChanged)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY selectionChanged)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY selectionChanged)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY selectionChanged)
    Q_PROPERTY(::QAccelPlot::InspectionRowModel* model READ model CONSTANT)

public:
    enum Mode {
        Box,    
        XRange, 
        YRange  
    };
    Q_ENUM(Mode)

    
    explicit SelectionTool(QObject* parent = nullptr);

    ::QAccelPlot::QAccelPlot* plot() const;
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    Axis* xAxis() const;
    void setXAxis(Axis* axis);
    Axis* yAxis() const;
    void setYAxis(Axis* axis);
    bool enabled() const;
    void setEnabled(bool enabled);
    Mode mode() const;
    void setMode(Mode mode);
    int button() const;
    void setButton(int button);
    int modifiers() const;
    void setModifiers(int modifiers);
    qreal minimumSize() const;
    void setMinimumSize(qreal size);
    QColor fillColor() const;
    void setFillColor(const QColor& color);
    QColor borderColor() const;
    void setBorderColor(const QColor& color);
    bool rectangleVisible() const;
    void setRectangleVisible(bool visible);
    bool selecting() const;
    bool hasSelection() const;
    QRectF pixelRect() const;
    qreal xMin() const;
    qreal xMax() const;
    qreal yMin() const;
    qreal yMax() const;
    InspectionRowModel* model() const;

    Q_INVOKABLE bool select(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    Q_INVOKABLE ::QAccelPlot::InspectionPage indices(::QAccelPlot::PlotSeries* series, int offset = 0, int limit = 4096);
    Q_INVOKABLE void clear();

signals:
    void plotChanged();
    void xAxisChanged();
    void yAxisChanged();
    void enabledChanged();
    void modeChanged();
    void buttonChanged();
    void modifiersChanged();
    void minimumSizeChanged();
    void fillColorChanged();
    void borderColorChanged();
    void rectangleVisibleChanged();
    void selectingChanged();
    void selectionChanged();
    void pixelRectChanged();
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
```


