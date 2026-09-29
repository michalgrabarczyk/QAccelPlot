//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

namespace QAccelPlotExample {

/// \brief One RectangleSeries dataset.
struct PlasmaBatch {
    std::vector<float> rects; // 4 floats per rectangle (x1, y1, x2, y2)
    int rectangleCount{0};
};

/// \brief Generates a grid of square tiles sized by a drifting plasma field.
///
/// The field is a sum of sine waves along x, y, and both diagonals. The waves are evaluated once
/// per row and per column, and the diagonal ones are combined with angle-sum identities, so a
/// tile costs a few multiply-adds. Tiles grow beyond their grid cell at the crests, so with a
/// translucent fill the crests stay brighter than the troughs even when tiles are smaller than
/// a pixel.
class PlasmaGenerator final {
public:
    using Batch = PlasmaBatch;

    /// \brief X extent of the tile grid, starting at 0.
    static constexpr auto kDomainWidth = 160.0f;
    /// \brief Y extent of the tile grid, starting at 0.
    static constexpr auto kDomainHeight = 100.0f;

    /// \brief Fills \a batch with \a rectangleCount tiles sized for \a timeSeconds.
    void generate(Batch& batch, int rectangleCount, double timeSeconds);

    /// \brief Returns the plasma value, in [0, 1], at (\a x, \a y) and \a timeSeconds.
    static float valueAt(float x, float y, double timeSeconds);
    /// \brief Returns the side length of a tile with plasma \a value on a grid of \a pitch.
    static float tileSide(float value, float pitch);

private:
    std::vector<float> columnTerms_; // per column: x wave, sin and cos of both diagonal x phases
    std::vector<float> rowTerms_;    // per row: y wave, sin and cos of both diagonal y phases
};

} // namespace QAccelPlotExample
