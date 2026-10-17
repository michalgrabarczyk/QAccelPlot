

# File LegendEntry.hpp

[**File List**](files.md) **>** [**legend**](dir_5fa9a39c33e751e7d5879623bc62e73e.md) **>** [**LegendEntry.hpp**](LegendEntry_8hpp.md)

[Go to the documentation of this file](LegendEntry_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QColor>
#include <QObject>

namespace QAccelPlot {

struct LegendEntry {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::PlotSeries::LegendSymbol symbol MEMBER symbol CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
    Q_PROPERTY(qreal lineWidth MEMBER lineWidth CONSTANT)
    Q_PROPERTY(::QAccelPlot::LineStyle* lineStyle MEMBER lineStyle CONSTANT)
    Q_PROPERTY(::QAccelPlot::PlotSeries::MarkerShape markerShape MEMBER markerShape CONSTANT)
    Q_PROPERTY(qreal markerSize MEMBER markerSize CONSTANT)
    Q_PROPERTY(bool markerFilled MEMBER markerFilled CONSTANT)
    Q_PROPERTY(qreal markerStrokeWidth MEMBER markerStrokeWidth CONSTANT)
    Q_PROPERTY(bool antialiasingEnabled MEMBER antialiasingEnabled CONSTANT)
    Q_PROPERTY(qreal antialiasingFeather MEMBER antialiasingFeather CONSTANT)

public:
    PlotSeries::LegendSymbol symbol{PlotSeries::LegendSymbol::Line};    
    QColor color;                                                       
    qreal lineWidth{1.0};                                               
    LineStyle* lineStyle{nullptr};                                      
    PlotSeries::MarkerShape markerShape{PlotSeries::MarkerShape::None}; 
    qreal markerSize{0.0};                                              
    bool markerFilled{true};                                            
    qreal markerStrokeWidth{1.0};                                       
    bool antialiasingEnabled{true};                                     
    qreal antialiasingFeather{1.0};                                     
};

} // namespace QAccelPlot
```


