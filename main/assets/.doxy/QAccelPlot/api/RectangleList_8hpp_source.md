

# File RectangleList.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**RectangleList.hpp**](RectangleList_8hpp.md)

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

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <vector>

namespace QAccelPlot {

class RectMaterial;

class RectangleList : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleList)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(qreal minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumHeightChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit RectangleList(QQuickItem* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    QList<QColor> categoryColors() const;
    void setCategoryColors(const QList<QColor>& colors);

    RectangleBorder* border() const;

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    qreal minimumWidth() const;
    void setMinimumWidth(qreal width);

    qreal minimumHeight() const;
    void setMinimumHeight(qreal height);

    int count() const;
    int hoveredIndex() const;

    Q_INVOKABLE void setData(const QVariantList& rects);

    void setData(const float* data, int rectCount);

    void setData(const double* data, int rectCount);

    void setData(std::vector<double>&& data, int rectCount);

    void setData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    void postData(std::vector<double>&& data, int rectCount);

    void postData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    Q_INVOKABLE void setCategories(const QList<int>& categories);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE QVariantMap rectangleAt(int index) const;

    int rectangleIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void categoryColorsChanged();
    void hoverColorChanged();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void countChanged();
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void onAxisScaleChanged() override;

private:
    bool validateRawDataArguments(const void* data, int rectCount) const;
    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int rectCount) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);
    bool hasCategories() const;
    QColor rectangleColor(int index) const;
    // Tests rectangle \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    void updateMaterial(RectMaterial& material) const;
    void buildSpatialGrid();
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision
    // data_, so the GPU upload stays accurate for large coordinates
    // (e.g. modern Unix-epoch timestamps) without needing double-precision textures.
    void rebuildRenderData(bool logScaleX, bool logScaleY);

    // Vertex cache: 6 vertices per rect, 12 bytes each. The color is the rectangle's category
    // color when categories are set; otherwise it is unused and \c color is a uniform.
    struct RectVertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    QColor color_;
    QList<QColor> categoryColors_;
    RectangleBorder* border_{new RectangleBorder{this}};
    QColor hoverColor_;
    qreal minimumWidth_{1.0};
    qreal minimumHeight_{1.0};
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision.
    std::vector<double> data_;
    // One category per rect, or empty when no rectangle has one.
    std::vector<int> categories_;
    // Origin-relative float mirror of data_, uploaded to the GPU.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    int rectCount_{0};
    bool dataChanged_{false};
    std::vector<RectVertex> vertexCache_;
    bool vertexCacheValid_{false};
    SpatialGrid spatialGrid_;
};

} // namespace QAccelPlot
```


