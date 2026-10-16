

# File DataPointStore.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**DataPointStore.hpp**](DataPointStore_8hpp.md)

[Go to the documentation of this file](DataPointStore_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <cstdint>
#include <optional>
#include <vector>

namespace QAccelPlot {

class DataPointStore {
public:
    enum class Dimension { X, Y };

    enum class RangeRule {
        EachCoordinate, 
        WholeDataPoint, 
        SkipOnNaN,      
    };

    struct Layout {
        std::vector<Dimension> columns{Dimension::X, Dimension::Y}; 
        RangeRule rangeRule{RangeRule::EachCoordinate};             
        int attributeCount{0}; 
    };

    struct Viewport {
        double min; 
        double max; 
    };

    struct DimensionState {
        bool logScale{false};             
        std::optional<Viewport> viewport; 
    };

    struct Extent {
        double min; 
        double max; 
    };

    struct Extents {
        std::optional<Extent> x; 
        std::optional<Extent> y; 
    };

    struct View {
        const double* precise{nullptr}; 
        const float* floats{nullptr};   
        int stride{0};                  
        int count{0};                   

        [[nodiscard]] double value(int index, int column) const noexcept;
    };

    struct AttributeView {
        const float* data{nullptr}; 
        int stride{0};              
    };

    explicit DataPointStore(Layout layout);

    [[nodiscard]] const Layout& layout() const noexcept;
    void setLayout(Layout layout);
    void remapColumns(std::vector<Dimension> columns);
    [[nodiscard]] int columnCount() const noexcept;

    void setPrecise(std::vector<double>&& coordinates, int count, std::vector<float>&& attributes = {});
    void assignPrecise(const double* coordinates, int count);
    void setFloat(std::vector<float>&& coordinates, int count, std::vector<float>&& attributes = {});
    void assignFloat(const float* coordinates, int count);
    void setAttributes(std::vector<float>&& attributes);
    void setCategories(std::vector<int>&& categories);
    void append(const double* coordinates);
    void promoteToPrecise();
    void clear();

    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] bool isPrecise() const noexcept;
    [[nodiscard]] bool hasAttributes() const noexcept;
    [[nodiscard]] bool hasCategories() const noexcept;
    [[nodiscard]] View view() const noexcept;
    [[nodiscard]] double value(int index, int column) const noexcept;
    [[nodiscard]] AttributeView attributes() const noexcept;
    [[nodiscard]] float attribute(int index, int which = 0) const noexcept;
    [[nodiscard]] const std::vector<int>& categories() const noexcept;

    [[nodiscard]] Extents extents(bool logScaleX, bool logScaleY, int first = 0, int last = -1) const;

    std::vector<double> setDrawn(std::vector<double>&& dataPoints, int count);
    void clearDrawn();
    [[nodiscard]] bool hasDrawn() const noexcept;
    [[nodiscard]] View drawnView() const;
    [[nodiscard]] View renderedView() const;

    void setSkipInvalid(bool skip);
    [[nodiscard]] bool skipInvalid() const noexcept;

    void setDimensionState(Dimension dimension, const DimensionState& state);
    [[nodiscard]] const DimensionState& dimensionState(Dimension dimension) const noexcept;

    [[nodiscard]] const std::vector<float>& renderData() const;
    [[nodiscard]] int renderStride() const noexcept;
    [[nodiscard]] int renderCount() const;
    [[nodiscard]] double renderOrigin(Dimension dimension) const;
    [[nodiscard]] std::uint64_t renderRevision() const noexcept;

private:
    struct Origin {
        double value{0.0};
        bool settled{false};
    };

    // Valid data points of the drawn or stored ones, kept only while some are invalid.
    struct Compacted {
        std::vector<double> precise;
        std::vector<float> floats;
        std::vector<float> attributes;
        int count{0};
        bool active{false};
        bool valid{false};
    };

    [[nodiscard]] View sourceView() const noexcept;
    [[nodiscard]] bool usesOrigin() const noexcept;
    [[nodiscard]] bool dataPointValid(const View& view, int index) const noexcept;
    [[nodiscard]] Origin& origin(Dimension dimension) const noexcept;
    [[nodiscard]] double originValue(Dimension dimension) const noexcept;
    void storeFloatCoordinates(std::vector<float>&& coordinates, std::vector<float>&& attributes);
    void dataPointsChanged();
    void invalidateRender();
    void ensureCompacted() const;
    void ensureRenderData() const;
    void chooseOrigins(const View& source) const;
    void writeRenderData(const View& source, const float* attributes, int attributeStride) const;

    Layout layout_;
    int count_{0};
    bool isPrecise_{false};
    bool hasAttributes_{false};
    std::vector<double> precise_;   // columnCount() doubles per data point
    std::vector<float> floats_;     // renderStride() floats per data point; the render data of float input
    std::vector<float> attributes_; // attributeCount floats per data point of double input
    std::vector<int> categories_;

    std::vector<double> drawn_; // renderStride() doubles per data point
    int drawnCount_{0};
    bool hasDrawn_{false};

    bool skipInvalid_{false};
    // Lazily derived from the members above, also by const readers.
    mutable Compacted compacted_;

    DimensionState x_;
    DimensionState y_;
    mutable Origin originX_;
    mutable Origin originY_;
    mutable bool renderLogScaleX_{false};
    mutable bool renderLogScaleY_{false};
    mutable std::vector<float> renderData_;
    mutable bool renderValid_{false};
    std::uint64_t renderRevision_{0};
};

} // namespace QAccelPlot
```


