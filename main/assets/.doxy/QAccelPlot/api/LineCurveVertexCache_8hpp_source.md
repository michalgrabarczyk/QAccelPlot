

# File LineCurveVertexCache.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**series**](dir_d1bb17d10be635dda10fdf13c9e6bbc5.md) **>** [**LineCurveVertexCache.hpp**](LineCurveVertexCache_8hpp.md)

[Go to the documentation of this file](LineCurveVertexCache_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstddef>
#include <functional>
#include <vector>

namespace QAccelPlot {

class LineCurveVertexCache final {
public:
    enum class Layout { None, Line, Points };

    using Builder = std::function<void(std::vector<char>&)>;

    [[nodiscard]] bool install(std::vector<char>&& bytes, Layout layout, int pointCount, std::size_t expectedSize);

    void rebuild(Layout layout, int pointCount, const Builder& builder);

    [[nodiscard]] bool isReusableForDataChange(Layout requiredLayout, int pointCount) const noexcept;

    void invalidate() noexcept;

    void clear() noexcept;

    [[nodiscard]] const std::vector<char>* data() const noexcept;

    [[nodiscard]] const std::vector<char>& bytes() const noexcept;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] Layout layout() const noexcept;

    [[nodiscard]] int pointCount() const noexcept;

private:
    std::vector<char> bytes_;
    bool valid_{false};
    Layout layout_{Layout::None};
    int pointCount_{0};
};

} // namespace QAccelPlot
```


