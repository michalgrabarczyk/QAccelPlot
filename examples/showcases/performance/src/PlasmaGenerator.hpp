//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"

#include <cstddef>
#include <vector>

namespace QAccelPlotExample {

/// \brief One series' share of a RectangleSeries dataset: 4 floats per rectangle (x1, y1, x2, y2).
struct PlasmaPart : SeriesPart {
    /// \brief One category per rectangle; empty when categories are off.
    std::vector<int> categories;

    /// \brief Returns the bytes held by the rectangles and categories.
    std::size_t byteSize() const;
};

/// \brief One RectangleSeries dataset, split into consecutive runs of grid rows.
struct PlasmaBatch {
    std::vector<PlasmaPart> parts;

    /// \brief Returns the bytes held by the batch's records.
    std::size_t byteSize() const;
};

/// \brief Settings of a PlasmaGenerator batch.
struct PlasmaParameters {
    DatasetParameters dataset;
    /// \brief Multiplies the side of every tile.
    float tileScale{1.0f};
    /// \brief Number of categories the plasma value of a tile is binned into; 0 turns categories off.
    int categoryCount{0};
};

/// \brief Returns \c true when \a a and \a b describe the same batch, apart from its time.
bool operator==(const PlasmaParameters& a, const PlasmaParameters& b);

/// \brief Generates a grid of square tiles sized by a drifting plasma field.
///
/// The field is a sum of sine waves along x, y, and both diagonals. The waves are evaluated once
/// per row and per column, and the diagonal ones are combined with angle-sum identities, so a
/// tile costs a few multiply-adds. Tiles grow beyond their grid cell at the crests, so with a
/// translucent fill the crests stay brighter than the troughs even when tiles are smaller than
/// a pixel. Large datasets are written by several threads.
class PlasmaGenerator final {
public:
    using Batch = PlasmaBatch;
    using Parameters = PlasmaParameters;

    /// \brief X extent of the tile grid, starting at 0.
    static constexpr auto kDomainWidth = 160.0f;
    /// \brief Y extent of the tile grid, starting at 0.
    static constexpr auto kDomainHeight = 100.0f;

    /// \brief Fills \a batch with the tiles described by \a parameters, sized for \a timeSeconds.
    void generate(Batch& batch, const Parameters& parameters, double timeSeconds);

    /// \brief Returns the plasma value, in [0, 1], at (\a x, \a y) and \a timeSeconds.
    static float valueAt(float x, float y, double timeSeconds);
    /// \brief Returns the side length of a tile with plasma \a value on a grid of \a pitch.
    static float tileSide(float value, float pitch);

private:
    struct Grid {
        int columns;
        int rows;
        float pitch;
        float originX;
        float originY;
    };

    void updateWaveTerms(const Grid& grid, double timeSeconds);
    // Writes tiles [first, last) of part; partBegin is the grid index of the part's first tile.
    void writeTiles(PlasmaPart& part, int first, int last, int partBegin, const Grid& grid, const Parameters& parameters) const;

    std::vector<float> columnTerms_; // per column: x wave, sin and cos of both diagonal x phases
    std::vector<float> rowTerms_;    // per row: y wave, sin and cos of both diagonal y phases
};

} // namespace QAccelPlotExample
