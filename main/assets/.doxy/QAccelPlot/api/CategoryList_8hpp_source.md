

# File CategoryList.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**CategoryList.hpp**](CategoryList_8hpp.md)

[Go to the documentation of this file](CategoryList_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/Category.hpp"
#include "QAccelPlot/series/internal/CategoryTable.hpp"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

namespace QAccelPlot::Internal {

class CategoryList : public QObject {
    Q_OBJECT

public:
    explicit CategoryList(const char* seriesName, QObject* parent = nullptr);

    QQmlListProperty<Category> listProperty(QObject* owner);
    void setCategories(const QList<Category*>& categories);

    const CategoryTable& table() const;

signals:
    void changed();

private:
    void append(Category* category);
    void clear();
    // Follows the edits and the destruction of \a category.
    void watch(Category* category) const;
    // Stops following every listed category.
    void unwatchAll() const;
    // Drops the destroyed category, whose pointer is already null.
    void onCategoryDestroyed();
    void onCategoriesChanged();
    // Logs the problems of the current table naming the series.
    void reportProblems();

    static void appendCategory(QQmlListProperty<Category>* list, Category* category);
    static qsizetype categoryCount(QQmlListProperty<Category>* list);
    static Category* categoryAt(QQmlListProperty<Category>* list, qsizetype index);
    static void clearCategories(QQmlListProperty<Category>* list);

    const char* seriesName_;
    QList<QPointer<Category>> categories_;
    CategoryTable table_;
    bool reportQueued_{false};
};

} // namespace QAccelPlot::Internal
```


