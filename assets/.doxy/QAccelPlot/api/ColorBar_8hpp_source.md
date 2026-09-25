

# File ColorBar.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**ColorBar.hpp**](ColorBar_8hpp.md)

[Go to the documentation of this file](ColorBar_8hpp.md)


```C++
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
#include <QPointer>
#include <QQuickPaintedItem>

#include <vector>

namespace QAccelPlot {

class ColorBar : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(ColorBar)

    
    Q_PROPERTY(PointCloud* series READ series WRITE setSeries NOTIFY seriesChanged)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont NOTIFY labelFontChanged)
    Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor NOTIFY labelColorChanged)
    Q_PROPERTY(qreal labelPadding READ labelPadding WRITE setLabelPadding NOTIFY labelPaddingChanged)
    Q_PROPERTY(qreal barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(AxisTicker* ticker READ ticker CONSTANT)

public:
    enum Orientation {
        Horizontal, 
        Vertical    
    };
    Q_ENUM(Orientation)

    
    explicit ColorBar(QQuickItem* parent = nullptr);

    PointCloud* series() const;
    void setSeries(PointCloud* series);

    Orientation orientation() const;
    void setOrientation(Orientation orientation);

    QString label() const;
    void setLabel(const QString& label);

    QFont labelFont() const;
    void setLabelFont(const QFont& font);

    QColor labelColor() const;
    void setLabelColor(const QColor& color);

    qreal labelPadding() const;
    void setLabelPadding(qreal padding);

    qreal barThickness() const;
    void setBarThickness(qreal thickness);

    QColor borderColor() const;
    void setBorderColor(const QColor& color);

    qreal borderWidth() const;
    void setBorderWidth(qreal width);

    AxisTicker* ticker() const;

    qreal valueToPixel(qreal value, qreal length) const;

    void paint(QPainter* painter) override;

signals:
    void seriesChanged();
    void orientationChanged();
    void labelChanged();
    void labelFontChanged();
    void labelColorChanged();
    void labelPaddingChanged();
    void barThicknessChanged();
    void borderColorChanged();
    void borderWidthChanged();

protected:
    void updatePolish() override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    struct Layout {
        QRectF strip;
        QRectF tickArea;
        QRectF title;
        qreal endInset{0.0};
    };

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
    AxisTicker* ticker_;

    // Captured on the GUI thread in updatePolish() and read by paint(), which may run on the render thread.
    std::vector<GradientStopData> stops_;
    qreal valueMin_{0.0};
    qreal valueMax_{1.0};
    bool logScale_{false};
    AxisTicks ticks_;
    qreal maxTickLabelWidth_{0.0};
    Layout layout_;
};

} // namespace QAccelPlot
```


