

# File RectVertexCache.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**RectVertexCache.hpp**](RectVertexCache_8hpp.md)

[Go to the documentation of this file](RectVertexCache_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>

#include <functional>
#include <vector>

class QSGGeometry;

namespace QAccelPlot {

class RectVertexCache final {
public:
    using ColorFunction = std::function<QColor(int index)>;

    static constexpr int kVerticesPerRect{6};

    void rebuild(int rectCount, const ColorFunction& colorAt);

    void invalidate() noexcept;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] int vertexCount() const noexcept;

    void copyTo(QSGGeometry& geometry) const;

private:
    struct Vertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    std::vector<Vertex> vertices_;
    bool valid_{false};
};

} // namespace QAccelPlot
```


