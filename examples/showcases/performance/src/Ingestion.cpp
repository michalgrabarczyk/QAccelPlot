//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "Ingestion.hpp"

#include <QAccelPlot/series/PlotSeries.hpp>

#include <utility>

namespace QAccelPlotExample {

Ingestion ingestionFromName(const QString& name)
{
    if (name == QLatin1String("floatMove")) {
        return Ingestion::FloatMove;
    }
    if (name == QLatin1String("floatNoRangeCopy")) {
        return Ingestion::FloatNoRangeCopy;
    }
    if (name == QLatin1String("doubleMove")) {
        return Ingestion::DoubleMove;
    }
    if (name == QLatin1String("floatPost")) {
        return Ingestion::FloatPost;
    }
    return Ingestion::FloatNoRangeMove;
}

bool usesDoubles(const Ingestion ingestion)
{
    return ingestion == Ingestion::DoubleMove;
}

bool hasPrecisionFor(const SeriesPart& part, const Ingestion ingestion)
{
    return part.count == 0 || part.floats.empty() == usesDoubles(ingestion);
}

void applyRecords(QAccelPlot::PlotSeries& series, SeriesPart& part, const Ingestion ingestion)
{
    if (part.count == 0) {
        series.clearData();
        return;
    }
    switch (ingestion) {
    case Ingestion::FloatNoRangeMove:
        series.setDataFNoRange(std::move(part.floats), part.count);
        break;
    case Ingestion::FloatMove:
        series.setDataF(std::move(part.floats), part.count);
        break;
    case Ingestion::FloatNoRangeCopy:
        series.setDataFNoRange(part.floats.data(), part.count);
        break;
    case Ingestion::DoubleMove:
        series.setData(std::move(part.doubles), part.count);
        break;
    case Ingestion::FloatPost:
        series.postData(std::move(part.floats), part.count);
        break;
    }
}

} // namespace QAccelPlotExample
