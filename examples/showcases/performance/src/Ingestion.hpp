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

namespace QAccelPlot {
class PlotSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Series API that receives the generated records.
enum class Ingestion {
    FloatNoRangeMove, ///< \brief <tt>setDataFNoRange(std::vector<float>&&, int)</tt>.
    FloatMove,        ///< \brief <tt>setDataF(std::vector<float>&&, int)</tt>, with a range scan.
    FloatNoRangeCopy, ///< \brief <tt>setDataFNoRange(const float*, int)</tt>.
    DoubleMove,       ///< \brief <tt>setData(std::vector<double>&&, int)</tt>, with a range scan.
    FloatPost,        ///< \brief <tt>postData(std::vector<float>&&, int)</tt>.
};

/// \brief Returns the ingestion named \a name in the QML settings, or \c FloatNoRangeMove for an unknown name.
Ingestion ingestionFromName(const QString& name);

/// \brief Returns \c true when \a ingestion takes double records.
bool usesDoubles(Ingestion ingestion);

/// \brief Returns \c true when \a part holds its records in the precision \a ingestion takes.
bool hasPrecisionFor(const SeriesPart& part, Ingestion ingestion);

/// \brief Hands the records of \a part to \a series through \a ingestion. An empty part clears the series.
void applyRecords(QAccelPlot::PlotSeries& series, SeriesPart& part, Ingestion ingestion);

} // namespace QAccelPlotExample
