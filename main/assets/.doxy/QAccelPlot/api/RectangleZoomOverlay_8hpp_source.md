

# File RectangleZoomOverlay.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**RectangleZoomOverlay.hpp**](RectangleZoomOverlay_8hpp.md)

[Go to the documentation of this file](RectangleZoomOverlay_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QQuickItem>

namespace QAccelPlot {

class PlotRectangleZoom;

class RectangleZoomOverlay final : public QQuickItem {
public:
    RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    void syncSelection();

    PlotRectangleZoom* configuration_;
};

} // namespace QAccelPlot
```


