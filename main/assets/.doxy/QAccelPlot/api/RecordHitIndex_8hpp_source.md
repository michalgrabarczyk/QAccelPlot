

# File RecordHitIndex.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**RecordHitIndex.hpp**](RecordHitIndex_8hpp.md)

[Go to the documentation of this file](RecordHitIndex_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/series/SpatialGrid.hpp"

#include <QPointF>
#include <QSizeF>

#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

class RecordHitIndex {
public:
    struct Query {
        QPointF position;    
        qreal radius{0.0};   
        qreal overhang{0.0}; 
        int drawnCount{0};   
    };

    using Distance = std::function<std::optional<qreal>(int index)>;
    using SpanAt = std::function<std::optional<std::pair<double, double>>(int index)>;

    void invalidate();
    bool valid() const;

    void build(int count, Qt::Orientation positionAxis, const SpanAt& spanAt);

    int nearest(const Query& query, const Axis& xAxis, const Axis& yAxis, const QSizeF& size, const Distance& distance) const;

private:
    std::vector<double> rects_;
    SpatialGrid grid_;
    bool valid_{false};
};

} // namespace QAccelPlot::Internal
```


