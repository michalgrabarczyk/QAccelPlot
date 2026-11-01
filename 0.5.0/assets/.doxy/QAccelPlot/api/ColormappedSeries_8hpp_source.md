

# File ColormappedSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**ColormappedSeries.hpp**](ColormappedSeries_8hpp.md)

[Go to the documentation of this file](ColormappedSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QList>
#include <QMetaObject>
#include <QPointer>

#include <optional>
#include <vector>

namespace QAccelPlot {

class ColormappedSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(ColormappedSeries)
    QML_UNCREATABLE("ColormappedSeries is a base class for series colored by value.")

    
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    Colormap* colormap() const;
    void setColormap(Colormap* colormap);

    qreal dataValueMin() const;
    qreal dataValueMax() const;

signals:
    void colormapChanged();
    void valueRangeChanged();

protected:
    struct ValueSpan {
        qreal min; 
        qreal max; 
    };

    ColormappedSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent);

    virtual std::optional<ValueSpan> scanValueRange() const;
    void updateValueRange();
    const std::vector<GradientStopData>& colorStops() const;

private:
    Q_SLOT void onColormapUpdated();
    void reconnectColormapSignals();
    void refreshColorStops();

    QPointer<Colormap> colormap_;
    std::vector<GradientStopData> colorStops_;
    qreal dataValueMin_{0.0};
    qreal dataValueMax_{1.0};
    QList<QMetaObject::Connection> colormapConnections_;
};

} // namespace QAccelPlot
```


