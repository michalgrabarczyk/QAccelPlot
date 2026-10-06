

# File RectangleSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**RectangleSeries.hpp**](RectangleSeries_8hpp.md)

[Go to the documentation of this file](RectangleSeries_8hpp.md)


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
#include "QAccelPlot/series/RectVertexCache.hpp"
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <QPointF>
#include <QSizeF>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

namespace Internal {
class HoverIndexBudget;
}
class RectMaterial;

class RectangleSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleSeries)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(qreal minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumHeightChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit RectangleSeries(QQuickItem* parent = nullptr);
    ~RectangleSeries() override;

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

    void setData(const double* data, int rectCount) override;

    void setData(std::vector<double>&& data, int rectCount) override;

    void setData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    void setDataNoRange(const double* data, int rectCount) override;
    void setDataNoRange(std::vector<double>&& data, int rectCount) override;

    void setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    void setDataF(const float* data, int rectCount) override;

    void setDataF(std::vector<float>&& data, int rectCount) override;

    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    void setDataFNoRange(const float* data, int rectCount) override;
    void setDataFNoRange(std::vector<float>&& data, int rectCount) override;

    void setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    void postData(std::vector<double>&& data, int rectCount) override;

    void postData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    void postData(std::vector<float>&& data, int rectCount) override;

    void postData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    Q_INVOKABLE void clearData() override;

    Q_INVOKABLE void setCategories(const QList<int>& categories);

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
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void onAxisScaleChanged() override;

private:
    bool validateRawDataArguments(const void* data, int rectCount) const;
    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int rectCount) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount, bool reportRanges);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount, bool reportRanges);
    void setDataFFromArray(const float* data, int rectCount, bool reportRanges);
    void finishDataChange(std::vector<int>&& categories, int rectCount, bool reportRanges);
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    // Returns edge \a component (0 = x1, 1 = y1, 2 = x2, 3 = y2) of rectangle \a index.
    double coordinate(int index, int component) const;
    bool hasCategories() const;
    QColor rectangleColor(int index) const;
    // Tests rectangle \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    void updateMaterial(RectMaterial& material) const;
    // Returns true when the spatial grid answers the next query, building it once scanning has cost as much.
    bool spatialGridReady() const;

    // Everything a hit test reads besides the rectangles, so equal inputs give an equal answer.
    struct HitTestInputs {
        QPointF position;
        QSizeF itemSize;
        QSizeF minimumSize;
        // The axis mappings, pinned down by each axis' scale and the coordinates at its two ends.
        double xAtLeft;
        double xAtRight;
        double yAtTop;
        double yAtBottom;
        bool logScaleX;
        bool logScaleY;

        bool operator==(const HitTestInputs& other) const;
    };

    HitTestInputs hitTestInputs(const QPointF& position) const;
    int topmostRectangleAt(const QPointF& position) const;
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision
    // data_, so the GPU upload stays accurate for large coordinates
    // (e.g. modern Unix-epoch timestamps) without needing double-precision textures.
    void rebuildRenderData(bool logScaleX, bool logScaleY);

    QColor color_;
    QList<QColor> categoryColors_;
    RectangleBorder* border_{new RectangleBorder{this}};
    QColor hoverColor_;
    qreal minimumWidth_{1.0};
    qreal minimumHeight_{1.0};
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision. Empty when setDataF() supplied the data.
    std::vector<double> data_;
    // One category per rect, or empty when no rectangle has one.
    std::vector<int> categories_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    int rectCount_{0};
    bool dataChanged_{false};
    // Vertex colors are category colors when categories are set; otherwise \c color is a uniform.
    RectVertexCache vertexCache_;
    // Built once scanning the same data has cost as much as the build, so streaming skips it.
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
    std::unique_ptr<Internal::HoverIndexBudget> spatialGridBudget_;
    // The last hit test and its answer. Qt tests contains() several times per hover event before
    // the handler asks again, each time at the same position.
    mutable std::optional<HitTestInputs> lastHitTest_;
    mutable int lastHitIndex_{-1};
};

} // namespace QAccelPlot
```


