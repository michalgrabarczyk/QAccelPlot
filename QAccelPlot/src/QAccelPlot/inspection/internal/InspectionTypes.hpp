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

/// \brief Data dimension a query measures along, or along which a series' records are ordered.
enum class InspectionAxis { X, Y };

/// \brief Read-only view over the XY records a series exposes to inspection queries.
///
/// The pointers refer to the series' own buffers and are only valid until its next data change.
struct InspectionSource {
    const float* floats{nullptr};   ///< \brief Interleaved single-precision records; X and Y lead each record.
    int floatStride{2};             ///< \brief Floats per record in \c floats.
    const double* doubles{nullptr}; ///< \brief Interleaved double-precision XY pairs; takes precedence over \c floats.
    const float* values{nullptr};   ///< \brief Optional per-record scalar.
    int valueStride{1};             ///< \brief Floats between consecutive scalars in \c values.
    int count{-1};                  ///< \brief Number of records, or -1 when the series has no XY records.
    bool logX{false};               ///< \brief Whether X is on a logarithmic axis.
    bool logY{false};               ///< \brief Whether Y is on a logarithmic axis.

    [[nodiscard]] bool supported() const noexcept;
    [[nodiscard]] double x(int index) const noexcept;
    [[nodiscard]] double y(int index) const noexcept;
    /// \brief Returns the coordinate of record \a index along \a axis.
    [[nodiscard]] double coordinate(InspectionAxis axis, int index) const noexcept;
    [[nodiscard]] double value(int index) const noexcept;
    /// \brief Applies the invalid-sample contract shared with rendering.
    [[nodiscard]] bool valid(int index) const noexcept;
};

/// \brief Pixel mapping of a series' plot area, used to measure on-screen distances.
struct InspectionMetric {
    AxisMapping x;
    AxisMapping y;
    double width{0.0};
    double height{0.0};

    [[nodiscard]] double pixelX(double value) const noexcept;
    [[nodiscard]] double pixelY(double value) const noexcept;
    /// \brief Maps a data \a value on \a axis to a series-local pixel.
    [[nodiscard]] double pixel(InspectionAxis axis, double value) const noexcept;
    /// \brief Maps a series-local \a pixel on \a axis to a data value.
    [[nodiscard]] double coord(InspectionAxis axis, double pixel) const noexcept;
};

/// \brief Inclusive data-space region; infinite limits leave a dimension unbounded.
struct InspectionBounds {
    double xMin;
    double xMax;
    double yMin;
    double yMax;

    [[nodiscard]] bool contains(double x, double y) const noexcept;
};

/// \brief Nearest-sample result: source index and pixel distance, or index -1.
struct InspectionHit {
    int index{-1};
    double distance{std::numeric_limits<double>::infinity()};

    /// \brief Keeps the closer candidate; equal distances resolve to the highest index, the sample drawn last.
    void consider(int candidate, double candidateDistance) noexcept;
};

/// \brief Source indices of the valid samples on either side of an X value, or -1.
struct InspectionNeighbors {
    int left{-1};
    int right{-1};
};

/// \brief Running sample-weighted Y statistics that can be merged across disjoint sample sets.
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

/// \brief Returns the signed distance from \a position to the closed interval between \a a and \a b.
[[nodiscard]] double distanceToInterval(double position, double a, double b) noexcept;

} // namespace QAccelPlot
