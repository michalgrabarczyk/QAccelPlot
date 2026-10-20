//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QTimer>

#include <utility>
#include <vector>

namespace QAccelPlot {
class BarSeries;
class HeatmapSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Keeps a BarSeries showing the histogram of the heatmap cells in view.
///
/// Counts the cells whose centers lie inside the heatmap's viewport into bins over the full value
/// range, so the bin edges never move, and loads the counts as ranged bars. Viewport changes are
/// collapsed into one recount per event-loop pass.
class VisibleHistogram final {
public:
    /// \brief Starts showing the histogram of \a values, the cells of the linearly spaced \a heatmap, in \a bars.
    VisibleHistogram(QAccelPlot::HeatmapSeries* heatmap, QAccelPlot::BarSeries* bars, std::vector<float> values, int binCount);

private:
    // Returns the first and last of count cells between min and max whose centers lie between
    // viewportMin and viewportMax. The first is past the last when none does.
    static std::pair<int, int> visibleCells(int count, double min, double max, double viewportMin, double viewportMax);

    void recount();

    QAccelPlot::HeatmapSeries* heatmap_;
    QAccelPlot::BarSeries* bars_;
    std::vector<float> values_;
    std::vector<double> edges_;
    // The values of the cells in view, reused between recounts.
    std::vector<float> visibleValues_;
    QTimer recountTimer_;
};

} // namespace QAccelPlotExample
