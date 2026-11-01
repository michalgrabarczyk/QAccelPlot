

# File CategoryTable.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**CategoryTable.hpp**](CategoryTable_8hpp.md)

[Go to the documentation of this file](CategoryTable_8hpp.md)


```C++
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

class CategoryTable {
public:
    struct Entry {
        int value{0}; 
        QString name; 
        QColor color; 
    };

    static CategoryTable fromCategories(const QList<Category*>& categories);

    bool empty() const;
    const std::vector<Entry>& entries() const;
    const QStringList& problems() const;

    QColor color(int value) const;
    QString name(int value) const;

    std::optional<int> dataPointCategory(const QVariant& value, const char* seriesName, qsizetype index) const;

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
```


