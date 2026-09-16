

# File GridNode.hpp

[**File List**](files.md) **>** [**grid**](dir_786c72ef44ba61f7b8903e40bd644b8e.md) **>** [**GridNode.hpp**](GridNode_8hpp.md)

[Go to the documentation of this file](GridNode_8hpp.md)


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
#include <QRectF>
#include <QSGNode>
#include <QVector>

#include <functional>

namespace QAccelPlot {

class Axis;
class Grid;

class GridNode : public QSGNode {
public:
    explicit GridNode() = default;

    void update(const Grid* grid, const Axis* xAxis, const Axis* yAxis, const QRectF& plotRect);

private:
    // Groups the orientation-specific inputs so the shared loop logic in
    // collectAxisGridLines() can handle both vertical and horizontal grid lines.
    struct GridLineCollectionParams {
        const Axis* axis;
        bool gridLinesVisible;
        bool subGridLinesVisible;
        // makeRect(dataValue, halfLineWidth) -> the QRectF to add for that grid line
        std::function<QRectF(qreal dataValue, qreal halfWidth)> makeRect;
    };

    void collectAxisGridLines(QVector<QRectF>& mainRects, QVector<QRectF>& subRects, const Grid* grid, const GridLineCollectionParams& p) const;

    QRectF plotRect_;
};

} // namespace QAccelPlot
```


