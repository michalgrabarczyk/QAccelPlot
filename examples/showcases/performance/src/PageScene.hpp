//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"
#include "Ingestion.hpp"

#include <QAccelPlot/series/PlotSeries.hpp>

#include <QList>
#include <QPointer>
#include <QString>
#include <QVariant>

#include <vector>

class QObject;
class QQuickItem;

namespace QAccelPlotExample {

/// \brief Options every showcase page offers.
struct CommonOptions {
    /// \brief Total number of records across the page's series.
    int count{1};
    /// \brief Number of series the records are split across.
    int seriesCount{1};
    /// \brief Series API that receives the records.
    Ingestion ingestion{Ingestion::FloatNoRangeMove};
    /// \brief Whether the series take part in hover hit-testing.
    bool hoverEnabled{true};
};

/// \brief Returns the dataset size and layout that \a options ask for.
DatasetParameters datasetParameters(const CommonOptions& options);

/// \brief Reads one showcase page's settings and series from the QML scene.
class PageScene final {
public:
    /// \brief Looks up the plot named \a plotName below \a root.
    ///
    /// \a settingsProperty names the property of \a root that holds the page's settings object.
    PageScene(QObject* root, const char* settingsProperty, const QString& plotName);

    /// \brief Returns the page setting \a name, or an invalid variant when there is none.
    QVariant setting(const char* name) const;
    /// \brief Returns the options every page offers.
    CommonOptions commonOptions() const;

    /// \brief Returns the plot's series of type \a Series, in creation order.
    template <typename Series> std::vector<Series*> series() const
    {
        auto result = std::vector<Series*>{};
        for (auto* plotSeries : allSeries()) {
            if (auto* series = qobject_cast<Series*>(plotSeries)) {
                result.push_back(series);
            }
        }
        return result;
    }

private:
    QList<QAccelPlot::PlotSeries*> allSeries() const;

    QObject* root_;
    const char* settingsProperty_;
    QPointer<QQuickItem> plot_;
};

} // namespace QAccelPlotExample
