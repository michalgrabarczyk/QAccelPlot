//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/legend/LegendEntry.hpp"

#include <QColor>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <optional>
#include <vector>

namespace QAccelPlot {

class Category;

namespace Internal {

/// \brief A snapshot of the categories of a series, looked up by value or name.
class CategoryTable {
public:
    /// \brief One category: the value data points carry, an optional name, and an optional color.
    struct Entry {
        int value{0}; ///< \brief Value that data points carry.
        QString name; ///< \brief Name, or empty.
        QColor color; ///< \brief Fill color, or invalid to use the series color.
    };

    /// \brief Takes a snapshot of \a categories, skipping null entries.
    ///
    /// Keeps the first of categories that share a value or a name, and describes each later one in
    /// \c problems().
    static CategoryTable fromCategories(const QList<Category*>& categories);

    /// \brief Returns true when there are no categories.
    bool empty() const;
    /// \brief Returns the categories in list order.
    const std::vector<Entry>& entries() const;
    /// \brief Returns a description of each category the snapshot skipped or could not name.
    const QStringList& problems() const;

    /// \brief Returns the color of the category with \a value, or an invalid color when it has none.
    QColor color(int value) const;
    /// \brief Returns the name of the category with \a value, or an empty string when it has none.
    QString name(int value) const;

    /// \brief Returns the category in \a value, the \c category of the QML data point at \a index.
    ///
    /// A number must be a whole number; a string is a category name. Returns \c std::nullopt for a
    /// missing or null \a value, and for any other value, which logs a warning naming \a seriesName.
    std::optional<int> dataPointCategory(const QVariant& value, const char* seriesName, qsizetype index) const;

    /// \brief Returns one legend entry per category: \a seriesEntry with the category's name and color.
    ///
    /// A category without a name is labeled with its value; one without a color keeps the series color.
    QList<LegendEntry> legendEntries(const LegendEntry& seriesEntry) const;

private:
    // Returns the category with \a value, or nullptr.
    const Entry* find(int value) const;
    // Indexes the categories for lookups by value: densely when their values are close, otherwise hashed.
    void buildValueIndex();

    std::vector<Entry> entries_;
    QStringList problems_;
    // Index into entries_ per value from denseMin_, or -1; empty when the values are too far apart.
    std::vector<int> denseIndex_;
    int denseMin_{0};
    QHash<int, int> hashedIndex_;
    QHash<QString, int> valueByName_;
};

} // namespace Internal
} // namespace QAccelPlot
