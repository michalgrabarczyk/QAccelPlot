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

/// \brief A hardware-accelerated QML item that renders a large list of axis-aligned rectangles.
///
/// All rectangles are stored as interleaved floats (x1, y1, x2, y2) and uploaded to the GPU
/// as a data texture, making it suitable for tens of thousands of rectangles.
/// Hover detection uses an internal \c SpatialGrid for O(1) hit tests.
///
/// \sa LineCurve, Axis
class RectangleList : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleList)

    /// \brief Fill color applied to all rectangles. Default: semi-transparent blue.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Read-only: number of rectangles currently loaded.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: index of the rectangle under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief Constructs a RectangleList with the given \a parent.
    explicit RectangleList(QQuickItem* parent = nullptr);

    /// \brief Returns the rectangle fill color.
    QColor color() const;
    /// \brief Sets the fill color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the number of rectangles currently loaded.
    int count() const;
    /// \brief Returns the index of the hovered rectangle, or -1 if none.
    int hoveredIndex() const;

    /// \brief Loads rectangles from \a rects, a QML list of objects with \c x1, \c y1, \c x2, \c y2 properties.
    Q_INVOKABLE void setData(const QVariantList& rects);

    /// \brief Loads rectangles from a C++ raw float array (\a data must have \a rectCount × 4 floats: x1, y1, x2, y2).
    void setData(const float* data, int rectCount);

    /// \brief Loads rectangles from a C++ raw double array, preserving full precision for large
    /// coordinates (e.g. modern Unix-epoch timestamps). \a data must have \a rectCount × 4 doubles.
    void setData(const double* data, int rectCount);

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the rectangle count changes.
    void countChanged();
    /// \brief Emitted when the hovered rectangle index changes.
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

private:
    bool validateRawDataArguments(const void* data, int rectCount) const;
    void buildSpatialGrid();
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision
    // data_, so GPU upload and hover hit-testing stay accurate for large coordinates
    // (e.g. modern Unix-epoch timestamps) without needing double-precision textures.
    void rebuildRenderData(bool logScaleX, bool logScaleY);

    // Vertex cache: 6 vertices per rect, 12 bytes each.
    // Rebuilt only when rectCount_ changes — vertex data is deterministic from count alone.
    struct RectVertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    QColor color_{QColor(0, 0, 255, 50)};
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision.
    std::vector<double> data_;
    // Origin-relative float mirror of data_, uploaded to the GPU and used for hover hit-testing.
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
