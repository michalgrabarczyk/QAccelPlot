//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/LineCurveVertexCache.hpp"

#include <utility>

namespace QAccelPlot {

bool LineCurveVertexCache::install(std::vector<char>&& bytes, const Layout layout, const int pointCount, const std::size_t expectedSize)
{
    if (bytes.empty() || expectedSize == 0 || bytes.size() != expectedSize || layout == Layout::None || pointCount <= 0) {
        clear();
        return false;
    }

    bytes_ = std::move(bytes);
    layout_ = layout;
    pointCount_ = pointCount;
    valid_ = true;
    return true;
}

void LineCurveVertexCache::rebuild(const Layout layout, const int pointCount, const Builder& builder)
{
    invalidate();
    builder(bytes_);
    if (bytes_.empty() || layout == Layout::None || pointCount <= 0) {
        return;
    }

    layout_ = layout;
    pointCount_ = pointCount;
    valid_ = true;
}

bool LineCurveVertexCache::isReusableForDataChange(const Layout requiredLayout, const int pointCount) const noexcept
{
    return valid_ && requiredLayout == Layout::Line && layout_ == Layout::Line && pointCount_ == pointCount;
}

void LineCurveVertexCache::invalidate() noexcept
{
    valid_ = false;
    layout_ = Layout::None;
    pointCount_ = 0;
}

void LineCurveVertexCache::clear() noexcept
{
    bytes_.clear();
    invalidate();
}

const std::vector<char>* LineCurveVertexCache::data() const noexcept
{
    return valid_ ? &bytes_ : nullptr;
}

const std::vector<char>& LineCurveVertexCache::bytes() const noexcept
{
    return bytes_;
}

bool LineCurveVertexCache::valid() const noexcept
{
    return valid_;
}

LineCurveVertexCache::Layout LineCurveVertexCache::layout() const noexcept
{
    return layout_;
}

int LineCurveVertexCache::pointCount() const noexcept
{
    return pointCount_;
}

} // namespace QAccelPlot
