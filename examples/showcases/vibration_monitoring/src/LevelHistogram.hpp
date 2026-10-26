//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QTimer>

#include <utility>
#include <vector>

namespace QAccelPlot {
class BarSeries;
class HeatmapSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Counts the spectrogram cells in view by level and shows the counts as the spectrogram's color key.
///
/// Recounts on the UI thread when the spectrogram's data or view changes, at most once per
/// event-loop pass, so zooming into part of the spectrogram shows the levels of that part.
class LevelHistogram final : public QObject {
public:
    /// \brief Starts counting the cells of \a spectrogram into \a bars, ranged bars along the level axis.
    LevelHistogram(QAccelPlot::HeatmapSeries* spectrogram, QAccelPlot::BarSeries* bars, QObject* parent = nullptr);

private:
    // Returns the first and last of count cells between min and max whose centers lie between
    // viewportMin and viewportMax. The first is past the last when none does.
    static std::pair<int, int> visibleCells(int count, double min, double max, double viewportMin, double viewportMax);

    void recount();

    QAccelPlot::HeatmapSeries* spectrogram_;
    QAccelPlot::BarSeries* bars_;
    // The levels of the cells in view, reused between recounts.
    std::vector<float> visibleLevels_;
    QTimer recountTimer_;
};

} // namespace QAccelPlotExample
