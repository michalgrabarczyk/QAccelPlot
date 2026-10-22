

# File HeatmapSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**HeatmapSeries.hpp**](HeatmapSeries_8hpp.md)

[Go to the documentation of this file](HeatmapSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/ColormappedSeries.hpp"

#include <QList>
#include <QPointF>

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

namespace Internal {
struct HeatmapGridDimension;
}
class HeatmapMaterial;

class HeatmapSeries : public ColormappedSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(HeatmapSeries)

    
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(qreal xMin READ xMin WRITE setXMin NOTIFY xMinChanged)
    Q_PROPERTY(qreal xMax READ xMax WRITE setXMax NOTIFY xMaxChanged)
    Q_PROPERTY(qreal yMin READ yMin WRITE setYMin NOTIFY yMinChanged)
    Q_PROPERTY(qreal yMax READ yMax WRITE setYMax NOTIFY yMaxChanged)
    Q_PROPERTY(Spacing xSpacing READ xSpacing WRITE setXSpacing NOTIFY xSpacingChanged)
    Q_PROPERTY(Spacing ySpacing READ ySpacing WRITE setYSpacing NOTIFY ySpacingChanged)

public:
    enum class Spacing {
        Linear, 
        Log     
    };
    Q_ENUM(Spacing)

    
    explicit HeatmapSeries(QQuickItem* parent = nullptr);

    int columns() const;
    void setColumns(int columns);

    int rows() const;

    qreal xMin() const;
    void setXMin(qreal value);

    qreal xMax() const;
    void setXMax(qreal value);

    qreal yMin() const;
    void setYMin(qreal value);

    qreal yMax() const;
    void setYMax(qreal value);

    Spacing xSpacing() const;
    void setXSpacing(Spacing spacing);

    Spacing ySpacing() const;
    void setYSpacing(Spacing spacing);

    Q_INVOKABLE void setValues(const QList<qreal>& values, int columns);
    void setValues(std::vector<float>&& values, int columns, int rows);
    void postValues(std::vector<float>&& values, int columns, int rows);
    Q_INVOKABLE qreal valueAt(int row, int column) const;
    Q_INVOKABLE void clearData() override;

signals:
    void columnsChanged();
    void rowsChanged();
    void xMinChanged();
    void xMaxChanged();
    void yMinChanged();
    void yMaxChanged();
    void xSpacingChanged();
    void ySpacingChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

    void applyDataPoints(DataPointInput&& input) override;
    int dataPointValueCount() const override;
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    bool positionInspectionSupported() const override;
    InspectionDataPoint inspectionDataPointAt(const QPointF& position) const override;
    DataRanges computeDataRanges() const override;
    void onAxisScaleChanged() override;
    std::optional<ValueSpan> scanValueRange() const override;

private:
    struct Extent {
        qreal xMin;
        qreal xMax;
        qreal yMin;
        qreal yMax;

        bool isFinite() const;
    };

    Extent resolvedExtent() const;
    Internal::HeatmapGridDimension xDimension() const;
    Internal::HeatmapGridDimension yDimension() const;
    int cellCount() const;
    bool validateShape(std::size_t valueCount, int columns, int rows) const;
    void storeValues(std::vector<float>&& values, int columns);
    void applyShape(int columns);
    void setBound(qreal& bound, qreal value, void (HeatmapSeries::*changed)());
    void setSpacing(Spacing& current, Spacing spacing, void (HeatmapSeries::*changed)());
    std::optional<std::pair<int, int>> cellAt(const QPointF& position) const;
    QPointF cellCenter(int row, int column) const;
    bool canDraw(QQuickWindow* window);
    void updateMaterial(HeatmapMaterial& material, QQuickWindow* window, bool newNode);

    std::vector<float> values_;
    int columns_{0};
    int rows_{0};
    Extent edges_;
    Spacing xSpacing_{Spacing::Linear};
    Spacing ySpacing_{Spacing::Linear};
    // Incremented by every value change; the GPU texture holds the values of uploadedRevision_.
    std::uint64_t valuesRevision_{1};
    std::uint64_t uploadedRevision_{0};
    bool invalidLogSpacing_{false};
    bool overPrecision_{false};
};

} // namespace QAccelPlot
```


