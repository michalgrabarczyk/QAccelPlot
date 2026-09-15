

# File LineCurveGapFilter.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**series**](dir_d1bb17d10be635dda10fdf13c9e6bbc5.md) **>** [**LineCurveGapFilter.hpp**](LineCurveGapFilter_8hpp.md)

[Go to the documentation of this file](LineCurveGapFilter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "renderers/CurveRendererParams.hpp"

#include <vector>

namespace QAccelPlot {

struct SampleRun {
    int start; 
    int count; 
};

namespace LineCurveGapFilter {

[[nodiscard]] bool isValidPoint(const CurveDataView& data, int index, bool logScaleX, bool logScaleY);

[[nodiscard]] int countInvalidPoints(const CurveDataView& data, int pointCount, bool logScaleX, bool logScaleY);

[[nodiscard]] std::vector<SampleRun> findValidRuns(const CurveDataView& data, int pointCount, bool logScaleX, bool logScaleY);

[[nodiscard]] std::vector<int> planRunSampling(const std::vector<SampleRun>& runs, int maxSampledPoints);

[[nodiscard]] int sampledSourceIndex(const SampleRun& run, int sampleIndex, int sampledCount);

int compactValidPoints(const std::vector<float>& data, int pointCount, bool logScaleX, bool logScaleY, std::vector<float>& output);

int compactValidPoints(const std::vector<double>& data, const std::vector<float>& renderData, int pointCount, bool logScaleX, bool logScaleY,
    std::vector<double>& output, std::vector<float>& renderOutput);

} // namespace LineCurveGapFilter

} // namespace QAccelPlot
```


