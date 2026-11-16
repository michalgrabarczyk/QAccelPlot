

# File RectangleZoomOverlay.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**RectangleZoomOverlay.hpp**](RectangleZoomOverlay_8hpp.md)

[Go to the documentation of this file](RectangleZoomOverlay_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/internal/OutlinedRectangle.hpp"

namespace QAccelPlot {
class PlotRectangleZoom;
} // namespace QAccelPlot

namespace QAccelPlot::Internal {

class RectangleZoomOverlay final : public OutlinedRectangle {
public:
    RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration);

private:
    void syncSelection();
    void syncStyle();

    PlotRectangleZoom* configuration_;
};

} // namespace QAccelPlot::Internal
```


