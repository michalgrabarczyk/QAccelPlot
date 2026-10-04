//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PageScene.hpp"

#include <QQuickItem>

#include <algorithm>

namespace QAccelPlotExample {

DatasetParameters datasetParameters(const CommonOptions& options)
{
    auto dataset = DatasetParameters{};
    dataset.count = options.count;
    return dataset;
}

PageScene::PageScene(QObject* root, const char* settingsProperty, const QString& plotName)
    : root_(root)
    , settingsProperty_(settingsProperty)
    , plot_(root->findChild<QQuickItem*>(plotName))
{
}

QVariant PageScene::setting(const char* name) const
{
    // Read from the root on every call: resetting a page replaces its settings object.
    const auto* settings = root_->property(settingsProperty_).value<QObject*>();
    return settings ? settings->property(name) : QVariant{};
}

CommonOptions PageScene::commonOptions() const
{
    auto options = CommonOptions{};
    options.count = std::max(1, setting("count").toInt());
    return options;
}

QList<QAccelPlot::PlotSeries*> PageScene::allSeries() const
{
    return plot_ ? plot_->property("series").value<QList<QAccelPlot::PlotSeries*>>() : QList<QAccelPlot::PlotSeries*>{};
}

} // namespace QAccelPlotExample
