

# File Axis.hpp

[**File List**](files.md) **>** [**axis**](dir_f07047c7c39e03c70b8bf3fe272880c9.md) **>** [**Axis.hpp**](Axis_8hpp.md)

[Go to the documentation of this file](Axis_8hpp.md)


```C++
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

class Axis : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(Axis)
    Q_PROPERTY(qreal viewportMin READ viewportMin WRITE setViewportMin NOTIFY viewportMinChanged)
    Q_PROPERTY(qreal viewportMax READ viewportMax WRITE setViewportMax NOTIFY viewportMaxChanged)
    Q_PROPERTY(qreal dataMin READ dataMin WRITE setDataMin NOTIFY dataMinChanged)
    Q_PROPERTY(qreal dataMax READ dataMax WRITE setDataMax NOTIFY dataMaxChanged)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont NOTIFY labelFontChanged)
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(QColor baselineColor READ baselineColor WRITE setBaselineColor NOTIFY baselineColorChanged)
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged)
    Q_PROPERTY(qreal baselineWidth READ baselineWidth WRITE setBaselineWidth NOTIFY baselineWidthChanged)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(Side side READ side WRITE setSide NOTIFY sideChanged)
    Q_PROPERTY(int axisTitlePadding READ axisTitlePadding WRITE setAxisTitlePadding NOTIFY axisTitlePaddingChanged)
    Q_PROPERTY(int axisLinePadding READ axisLinePadding WRITE setAxisLinePadding NOTIFY axisLinePaddingChanged)
    Q_PROPERTY(qreal layoutSize READ layoutSize WRITE setLayoutSize NOTIFY layoutSizeChanged)
    Q_PROPERTY(AxisTicker* ticker READ ticker CONSTANT)
    Q_PROPERTY(bool logScale READ logScale WRITE setLogScale NOTIFY logScaleChanged)
    Q_PROPERTY(double zoomScaleFactor READ zoomScaleFactor WRITE setZoomScaleFactor NOTIFY zoomScaleFactorChanged)

public:
    enum Orientation { Horizontal, Vertical };
    Q_ENUM(Orientation)

    
    enum Side { Left, Top, Right, Bottom };
    Q_ENUM(Side)

    
    explicit Axis(QQuickItem* parent = nullptr, Side side = Bottom);

    qreal viewportMin() const;
    void setViewportMin(qreal m);

    qreal viewportMax() const;
    void setViewportMax(qreal m);

    qreal dataMin() const;
    void setDataMin(qreal m);

    qreal dataMax() const;
    void setDataMax(qreal m);

    Orientation orientation() const;
    void setOrientation(Orientation o);

    QString label() const;
    void setLabel(const QString& t);

    QFont labelFont() const;
    void setLabelFont(const QFont& f);

    Side side() const;
    void setSide(Side s);

    bool hovered() const;

    QColor baselineColor() const;
    void setBaselineColor(const QColor& c);

    QColor labelColor() const;
    void setLabelColor(const QColor& c);

    qreal baselineWidth() const;
    void setBaselineWidth(qreal width);

    QColor hoverColor() const;
    void setHoverColor(const QColor& c);

    int axisTitlePadding() const;
    void setAxisTitlePadding(int padding);

    int axisLinePadding() const;
    void setAxisLinePadding(int padding);

    qreal layoutSize() const;
    void setLayoutSize(qreal size);

    AxisTicker* ticker() const;

    bool logScale() const;
    void setLogScale(bool on);
    double zoomScaleFactor() const;
    void setZoomScaleFactor(double factor);

    Q_INVOKABLE void toggleLogScale();

    Q_INVOKABLE void rescaleToData();

    void paint(QPainter* painter) override;

    qreal inwardTickOverlap() const;

    qreal labelOverflow() const;

    qreal coordToPixel(qreal value, qreal length) const;
    qreal pixelToCoord(qreal pos, qreal length) const;

public slots:
    void updateDataRange(qreal min, qreal max);

signals:
    void viewportMinChanged();
    void viewportMaxChanged();
    void dataMinChanged();
    void dataMaxChanged();
    void orientationChanged();
    void labelChanged();
    void labelFontChanged();
    void hoveredChanged();
    void rangeChanged();

    void baselineColorChanged();
    void labelColorChanged();
    void baselineWidthChanged();
    void hoverColorChanged();
    void sideChanged();
    void axisTitlePaddingChanged();
    void axisLinePaddingChanged();
    void layoutSizeChanged();
    void logScaleChanged();
    void zoomScaleFactorChanged();
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
```


