//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"

#include <QString>

#include <utility>

namespace QAccelPlot {
class PlotSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Series API that receives the generated records.
enum class Ingestion {
    FloatMove,  ///< \brief <tt>setDataF(std::vector<float>&&, int)</tt>.
    FloatCopy,  ///< \brief <tt>setDataF(const float*, int)</tt>.
    DoubleMove, ///< \brief <tt>setData(std::vector<double>&&, int)</tt>.
    FloatPost,  ///< \brief <tt>postData(std::vector<float>&&, int)</tt>.
};

/// \brief Returns the ingestion named \a name in the QML settings, or \c FloatMove for an unknown name.
Ingestion ingestionFromName(const QString& name);

/// \brief Returns \c true when \a ingestion takes double records.
bool usesDoubles(Ingestion ingestion);

/// \brief Returns \c true when \a part holds its records in the precision \a ingestion takes.
bool hasPrecisionFor(const SeriesPart& part, Ingestion ingestion);

/// \brief Hands the records of \a part to \a series through \a ingestion. An empty part clears the series.
void applyRecords(QAccelPlot::PlotSeries& series, SeriesPart& part, Ingestion ingestion);

/// \brief Like \c applyRecords() and also hands the categories of \a part to \a series.
///
/// \a Series has the category overloads of the vector setters, and \a Part derives from
/// \c SeriesPart and adds a \c categories vector. The raw-array setters take no categories.
template <typename Series, typename Part> void applyCategorizedRecords(Series& series, Part& part, const Ingestion ingestion)
{
    if (part.count == 0 || part.categories.empty()) {
        applyRecords(series, part, ingestion);
        return;
    }
    switch (ingestion) {
    case Ingestion::FloatMove:
        series.setDataF(std::move(part.floats), std::move(part.categories), part.count);
        break;
    case Ingestion::FloatCopy:
        applyRecords(series, part, ingestion);
        break;
    case Ingestion::DoubleMove:
        series.setData(std::move(part.doubles), std::move(part.categories), part.count);
        break;
    case Ingestion::FloatPost:
        series.postData(std::move(part.floats), std::move(part.categories), part.count);
        break;
    }
}

} // namespace QAccelPlotExample
