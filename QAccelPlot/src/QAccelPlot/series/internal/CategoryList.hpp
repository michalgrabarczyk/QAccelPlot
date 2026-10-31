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
#include <QQmlListProperty>

namespace QAccelPlot::Internal {

/// \brief The \c categories list of a series: the \c Category objects and a lookup table that follows their edits.
///
/// The table updates on every change, quietly. Duplicate values and names are reported once the
/// event loop runs, after QML bindings of newly declared categories have been evaluated.
class CategoryList : public QObject {
    Q_OBJECT

public:
    /// \brief Constructs an empty list whose warnings name \a seriesName.
    explicit CategoryList(const char* seriesName, QObject* parent = nullptr);

    /// \brief Returns the list as a QML list property of \a owner.
    QQmlListProperty<Category> listProperty(QObject* owner);
    /// \brief Replaces the categories with \a categories. Does not take ownership.
    void setCategories(const QList<Category*>& categories);

    /// \brief Returns the lookup table of the current categories.
    const CategoryTable& table() const;

signals:
    /// \brief Emitted when a category is added, removed, or edited.
    void changed();

private:
    void append(Category* category);
    void clear();
    // Follows the edits and the destruction of \a category.
    void watch(Category* category) const;
    void onCategoryDestroyed(QObject* category);
    void onCategoriesChanged();
    // Logs the problems of the current table naming the series.
    void reportProblems();

    static void appendCategory(QQmlListProperty<Category>* list, Category* category);
    static qsizetype categoryCount(QQmlListProperty<Category>* list);
    static Category* categoryAt(QQmlListProperty<Category>* list, qsizetype index);
    static void clearCategories(QQmlListProperty<Category>* list);

    const char* seriesName_;
    QList<Category*> categories_;
    CategoryTable table_;
    bool reportQueued_{false};
};

} // namespace QAccelPlot::Internal
