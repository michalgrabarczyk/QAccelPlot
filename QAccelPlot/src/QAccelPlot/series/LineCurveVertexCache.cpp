//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/LineCurveVertexCache.hpp"

#include <algorithm>

namespace QAccelPlot {

void LineCurveVertexCache::update(const Layout layout, const int pointCount, const std::size_t bytesPerPoint, const Writer& writer)
{
    if (layout == Layout::None || pointCount <= 0 || bytesPerPoint == 0) {
        invalidate();
        return;
    }

    const auto keptPoints = (valid_ && layout_ == layout && bytesPerPoint_ == bytesPerPoint) ? std::min(pointCount_, pointCount) : 0;
    bytes_.resize(static_cast<std::size_t>(pointCount) * bytesPerPoint);
    if (keptPoints < pointCount) {
        writer(bytes_, keptPoints);
    }
    layout_ = layout;
    pointCount_ = pointCount;
    bytesPerPoint_ = bytesPerPoint;
    valid_ = true;
}

bool LineCurveVertexCache::isReusableForDataChange(const Layout requiredLayout, const int pointCount) const noexcept
{
    return valid_ && requiredLayout != Layout::None && layout_ == requiredLayout && pointCount_ == pointCount;
}

void LineCurveVertexCache::invalidate() noexcept
{
    valid_ = false;
    layout_ = Layout::None;
    pointCount_ = 0;
    bytesPerPoint_ = 0;
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
