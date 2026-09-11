

# File RectangleList.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**shapes**](dir_bdc19b5408ae96d0ef2607428fba59ea.md) **>** [**RectangleList.hpp**](RectangleList_8hpp.md)

[Go to the documentation of this file](RectangleList_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "series/PlotSeries.hpp"
#include "series/SpatialGrid.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <vector>

namespace QAccelPlot {

class RectangleList : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleList)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit RectangleList(QQuickItem* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    int count() const;
    int hoveredIndex() const;

    Q_INVOKABLE void setData(const QVariantList& rects);

    void setRawData(const float* data, int rectCount);

signals:
    void colorChanged();
    void countChanged();
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

private:
    bool validateRawDataArguments(const float* data, int rectCount) const;
    void buildSpatialGrid();
    void buildVertexCache();
    void updateDataRanges();

    // Vertex cache: 6 vertices per rect, 12 bytes each.
    // Rebuilt only when rectCount_ changes — vertex data is deterministic from count alone.
    struct RectVertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    QColor color_{QColor(0, 0, 255, 50)};
    int hoveredIndex_{-1};
    // Data: 4 floats per rect (x1, y1, x2, y2)
    std::vector<float> data_;
    int rectCount_{0};
    bool dataChanged_{false};
    std::vector<RectVertex> vertexCache_;
    bool vertexCacheValid_{false};
    SpatialGrid spatialGrid_;
};

} // namespace QAccelPlot
```


