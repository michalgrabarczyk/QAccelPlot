//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
// Palette coordinates of gradients along a log axis. Pixels are linear in log10
// there, so shaders interpolate log10(value / reference) and convert it per
// fragment. Matches QAccelPlot::Internal::LogGradientMapping.

// Returns the positive value that log ratios of a gradient range are taken against.
float logGradientReference(float valueMin, float valueMax) {
    float largest = max(abs(valueMin), abs(valueMax));
    return largest > 0.0 ? largest : 1.0;
}

// Returns scale * (10^logRatio - 1) + offset. The series keeps precision for
// values near the reference, where exp(y) - 1 would cancel.
float logGradientCoordinate(float logRatio, float scale, float offset) {
    float y = logRatio * 2.302585093;
    float ratioMinusOne = abs(y) < 0.01 ? y * (1.0 + y * (0.5 + y / 6.0)) : exp(y) - 1.0;
    return scale * ratioMinusOne + offset;
}
