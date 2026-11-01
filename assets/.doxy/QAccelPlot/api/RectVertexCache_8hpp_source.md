

# File RectVertexCache.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**RectVertexCache.hpp**](RectVertexCache_8hpp.md)

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

namespace QAccelPlot::Internal {

class RectVertexCache final {
public:
    using ColorFunction = std::function<QColor(int index)>;

    static constexpr int kVerticesPerRect{6};
    static constexpr int kBytesPerRect{72};

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

} // namespace QAccelPlot::Internal
```


