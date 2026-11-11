//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DatasetParts.hpp"

namespace QAccelPlotExample {

std::size_t SeriesPart::byteSize() const
{
    return floats.size() * sizeof(float) + doubles.size() * sizeof(double);
}

bool operator==(const DatasetParameters& a, const DatasetParameters& b)
{
    return a.count == b.count && a.seriesCount == b.seriesCount && a.doublePrecision == b.doublePrecision;
}

int partCount(const int total, const int seriesCount, const int index)
{
    // The first total % seriesCount series get one record more.
    return total / seriesCount + (index < total % seriesCount ? 1 : 0);
}

void widenToDoubles(SeriesPart& part)
{
    part.doubles.assign(part.floats.begin(), part.floats.end());
    part.floats.clear();
}

} // namespace QAccelPlotExample
