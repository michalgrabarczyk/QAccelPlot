

# File InspectionTypes.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**InspectionTypes.hpp**](InspectionTypes_8hpp.md)

[Go to the documentation of this file](InspectionTypes_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/AxisMapping.hpp"

#include <QPointF>

#include <limits>

namespace QAccelPlot {

enum class InspectionAxis { X, Y };

struct InspectionSource {
    const float* floats{nullptr};   
    int floatStride{2};             
    const double* doubles{nullptr}; 
    const float* values{nullptr};   
    int valueStride{1};             
    int count{-1};                  
    bool logX{false};               
    bool logY{false};               

    [[nodiscard]] bool supported() const noexcept;
    [[nodiscard]] double x(int index) const noexcept;
    [[nodiscard]] double y(int index) const noexcept;
    [[nodiscard]] double coordinate(InspectionAxis axis, int index) const noexcept;
    [[nodiscard]] double value(int index) const noexcept;
    [[nodiscard]] bool valid(int index) const noexcept;
};

struct InspectionMetric {
    AxisMapping x;
    AxisMapping y;
    double width{0.0};
    double height{0.0};

    [[nodiscard]] double pixelX(double value) const noexcept;
    [[nodiscard]] double pixelY(double value) const noexcept;
    [[nodiscard]] double pixel(InspectionAxis axis, double value) const noexcept;
    [[nodiscard]] double coord(InspectionAxis axis, double pixel) const noexcept;
};

struct InspectionBounds {
    double xMin;
    double xMax;
    double yMin;
    double yMax;

    [[nodiscard]] bool contains(double x, double y) const noexcept;
};

struct InspectionHit {
    int index{-1};
    double distance{std::numeric_limits<double>::infinity()};

    void consider(int candidate, double candidateDistance) noexcept;
};

struct InspectionNeighbors {
    int left{-1};
    int right{-1};
};

struct SummaryAccumulator {
    int count{0};
    int minimumIndex{-1};
    int maximumIndex{-1};
    double minimum{std::numeric_limits<double>::quiet_NaN()};
    double maximum{std::numeric_limits<double>::quiet_NaN()};
    double mean{0.0};
    double m2{0.0}; // Sum of squared deviations from the mean.

    void add(double y, int index) noexcept;
    void merge(const SummaryAccumulator& other) noexcept;
    [[nodiscard]] double standardDeviation() const noexcept;
};

[[nodiscard]] double distanceToInterval(double position, double a, double b) noexcept;

} // namespace QAccelPlot
```


