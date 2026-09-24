

# File SeriesMarker.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**SeriesMarker.hpp**](SeriesMarker_8hpp.md)

[Go to the documentation of this file](SeriesMarker_8hpp.md)


```C++
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

class SeriesMarker : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(PlotSeries::MarkerShape shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(qreal size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool filled READ filled WRITE setFilled NOTIFY filledChanged)
    Q_PROPERTY(qreal strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)

public:
    enum class NoneShape { Accepted, Rejected };

    SeriesMarker(PlotSeries::MarkerShape shape, qreal size, NoneShape noneShape, QObject* parent);

    PlotSeries::MarkerShape shape() const;
    void setShape(PlotSeries::MarkerShape shape);

    qreal size() const;
    void setSize(qreal size);

    bool filled() const;
    void setFilled(bool filled);

    qreal strokeWidth() const;
    void setStrokeWidth(qreal width);

signals:
    void shapeChanged();
    void sizeChanged();
    void filledChanged();
    void strokeWidthChanged();

private:
    PlotSeries::MarkerShape shape_;
    qreal size_;
    NoneShape noneShape_;
    bool filled_{true};
    qreal strokeWidth_{1.0};
};

} // namespace QAccelPlot
```


