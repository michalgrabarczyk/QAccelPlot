

# File Grid.hpp

[**File List**](files.md) **>** [**grid**](dir_786c72ef44ba61f7b8903e40bd644b8e.md) **>** [**Grid.hpp**](Grid_8hpp.md)

[Go to the documentation of this file](Grid_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class Grid : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(qreal subGridLineWidth READ subGridLineWidth WRITE setSubGridLineWidth NOTIFY subGridLineWidthChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)
    Q_PROPERTY(QColor subGridColor READ subGridColor WRITE setSubGridColor NOTIFY subGridColorChanged)
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)
    Q_PROPERTY(bool subGridVisible READ subGridVisible WRITE setSubGridVisible NOTIFY subGridVisibleChanged)
    Q_PROPERTY(bool gridHorizontalLinesVisible READ gridHorizontalLinesVisible WRITE setGridHorizontalLinesVisible NOTIFY gridHorizontalLinesVisibleChanged)
    Q_PROPERTY(bool gridVerticalLinesVisible READ gridVerticalLinesVisible WRITE setGridVerticalLinesVisible NOTIFY gridVerticalLinesVisibleChanged)
    Q_PROPERTY(bool subGridHorizontalLinesVisible READ subGridHorizontalLinesVisible WRITE setSubGridHorizontalLinesVisible NOTIFY
            subGridHorizontalLinesVisibleChanged)
    Q_PROPERTY(bool subGridVerticalLinesVisible READ subGridVerticalLinesVisible WRITE setSubGridVerticalLinesVisible NOTIFY subGridVerticalLinesVisibleChanged)

public:
    explicit Grid(QObject* parent = nullptr);

    qreal lineWidth() const;
    void setLineWidth(qreal w);

    qreal subGridLineWidth() const;
    void setSubGridLineWidth(qreal w);

    QColor gridColor() const;
    void setGridColor(const QColor& c);

    QColor subGridColor() const;
    void setSubGridColor(const QColor& c);

    bool gridVisible() const;
    void setGridVisible(bool on);

    bool subGridVisible() const;
    void setSubGridVisible(bool on);

    bool gridHorizontalLinesVisible() const;
    void setGridHorizontalLinesVisible(bool on);

    bool gridVerticalLinesVisible() const;
    void setGridVerticalLinesVisible(bool on);

    bool subGridHorizontalLinesVisible() const;
    void setSubGridHorizontalLinesVisible(bool on);

    bool subGridVerticalLinesVisible() const;
    void setSubGridVerticalLinesVisible(bool on);

signals:
    void lineWidthChanged();
    void subGridLineWidthChanged();
    void gridColorChanged();
    void subGridColorChanged();
    void gridVisibleChanged();
    void subGridVisibleChanged();
    void gridHorizontalLinesVisibleChanged();
    void gridVerticalLinesVisibleChanged();
    void subGridHorizontalLinesVisibleChanged();
    void subGridVerticalLinesVisibleChanged();

private:
    qreal lineWidth_{2.0};
    qreal subGridLineWidth_{1.0};
    QColor gridColor_{"#8c8c8c"};
    QColor subGridColor_{"#b4b4b4"};
    bool gridVisible_{true};
    bool subGridVisible_{true};
    bool gridHorizontalLinesVisible_{true};
    bool gridVerticalLinesVisible_{true};
    bool subGridHorizontalLinesVisible_{true};
    bool subGridVerticalLinesVisible_{true};
};

} // namespace QAccelPlot
```


