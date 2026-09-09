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

/// \brief Owns a LineCurve's pre-built vertex bytes and the metadata required to use them safely.
///
/// A cache is valid only for the vertex layout and point count recorded when it
/// was installed or rebuilt. Invalidating a cache preserves its allocation for
/// reuse; clearing it also releases the stored bytes.
class LineCurveVertexCache final {
public:
    /// \brief Renderer geometry represented by the cached bytes.
    enum class Layout { None, Line, Points };

    /// \brief Function that writes a complete cache into the supplied reusable byte buffer.
    using Builder = std::function<void(std::vector<char>&)>;

    /// \brief Installs externally built cache \a bytes.
    ///
    /// Returns \c true when the byte count matches \a expectedSize and the
    /// supplied metadata describes a usable cache. On failure, the cache is cleared.
    [[nodiscard]] bool install(std::vector<char>&& bytes, Layout layout, int pointCount, std::size_t expectedSize);

    /// \brief Rebuilds the cache with \a builder while retaining reusable allocation capacity.
    ///
    /// The rebuilt cache is valid only when the builder produces bytes and the
    /// supplied \a layout and \a pointCount are usable.
    void rebuild(Layout layout, int pointCount, const Builder& builder);

    /// \brief Returns whether this cache can survive a data change with the requested layout.
    ///
    /// Solid-line cache vertices contain no XY values and can therefore be
    /// reused when the point count remains unchanged.
    [[nodiscard]] bool isReusableForDataChange(Layout requiredLayout, int pointCount) const noexcept;

    /// \brief Marks the cache unusable while retaining its byte allocation.
    void invalidate() noexcept;

    /// \brief Removes all cached bytes and resets the compatibility metadata.
    void clear() noexcept;

    /// \brief Returns the cached bytes when valid, or \c nullptr otherwise.
    [[nodiscard]] const std::vector<char>* data() const noexcept;

    /// \brief Returns the owned byte buffer, including retained invalid storage.
    [[nodiscard]] const std::vector<char>& bytes() const noexcept;

    /// \brief Returns whether the cache contains compatible, usable data.
    [[nodiscard]] bool valid() const noexcept;

    /// \brief Returns the renderer geometry represented by the cache.
    [[nodiscard]] Layout layout() const noexcept;

    /// \brief Returns the point count associated with the cache.
    [[nodiscard]] int pointCount() const noexcept;

private:
    std::vector<char> bytes_;
    bool valid_{false};
    Layout layout_{Layout::None};
    int pointCount_{0};
};

} // namespace QAccelPlot
