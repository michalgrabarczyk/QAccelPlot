//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

struct RollingStats {
    std::vector<float> samples; // x, y
    std::vector<float> mean;    // x, mean
    std::vector<float> band;    // x, mean - 2σ, mean + 2σ
};

// Simulates ten minutes of a vibration sensor with sampleCount samples (at least 2) and
// computes its rolling mean and mean ± 2σ over a centered 10 s window.
RollingStats computeRollingStats(int sampleCount);
