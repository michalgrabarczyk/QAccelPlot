//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/CategoryList.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <utility>

namespace QAccelPlot::Internal {

CategoryList::CategoryList(const char* seriesName, QObject* parent)
    : QObject(parent)
    , seriesName_(seriesName)
{
}

QQmlListProperty<Category> CategoryList::listProperty(QObject* owner)
{
    return QQmlListProperty<Category>{
        owner, this, &CategoryList::appendCategory, &CategoryList::categoryCount, &CategoryList::categoryAt, &CategoryList::clearCategories};
}

void CategoryList::setCategories(const QList<Category*>& categories)
{
    auto listed = QList<QPointer<Category>>{};
    for (auto* category : categories) {
        if (category) {
            listed.append(category);
        }
    }
    if (listed == categories_) {
        return;
    }
    unwatchAll();
    categories_ = std::move(listed);
    for (const auto& category : std::as_const(categories_)) {
        watch(category.data());
    }
    onCategoriesChanged();
}

const CategoryTable& CategoryList::table() const
{
    return table_;
}

void CategoryList::append(Category* category)
{
    if (!category) {
        return;
    }
    watch(category);
    categories_.append(category);
    onCategoriesChanged();
}

void CategoryList::clear()
{
    if (categories_.isEmpty()) {
        return;
    }
    unwatchAll();
    categories_.clear();
    onCategoriesChanged();
}

void CategoryList::watch(Category* category) const
{
    connect(category, &Category::valueChanged, this, &CategoryList::onCategoriesChanged, Qt::UniqueConnection);
    connect(category, &Category::nameChanged, this, &CategoryList::onCategoriesChanged, Qt::UniqueConnection);
    connect(category, &Category::colorChanged, this, &CategoryList::onCategoriesChanged, Qt::UniqueConnection);
    connect(category, &QObject::destroyed, this, &CategoryList::onCategoryDestroyed, Qt::UniqueConnection);
}

void CategoryList::unwatchAll() const
{
    for (const auto& category : categories_) {
        if (category) {
            disconnect(category.data(), nullptr, this, nullptr);
        }
    }
}

void CategoryList::onCategoryDestroyed()
{
    categories_.removeAll(QPointer<Category>{});
    onCategoriesChanged();
}

void CategoryList::onCategoriesChanged()
{
    auto live = QList<Category*>{};
    for (const auto& category : std::as_const(categories_)) {
        live.append(category.data());
    }
    table_ = CategoryTable::fromCategories(live);
    if (!table_.problems().isEmpty() && !reportQueued_) {
        reportQueued_ = true;
        QMetaObject::invokeMethod(this, &CategoryList::reportProblems, Qt::QueuedConnection);
    }
    emit changed();
}

void CategoryList::reportProblems()
{
    reportQueued_ = false;
    for (const auto& problem : table_.problems()) {
        qCWarning(lcQAccelPlot).noquote() << seriesName_ << problem;
    }
}

void CategoryList::appendCategory(QQmlListProperty<Category>* list, Category* category)
{
    static_cast<CategoryList*>(list->data)->append(category);
}

qsizetype CategoryList::categoryCount(QQmlListProperty<Category>* list)
{
    return static_cast<CategoryList*>(list->data)->categories_.size();
}

Category* CategoryList::categoryAt(QQmlListProperty<Category>* list, const qsizetype index)
{
    return static_cast<CategoryList*>(list->data)->categories_.value(index).data();
}

void CategoryList::clearCategories(QQmlListProperty<Category>* list)
{
    static_cast<CategoryList*>(list->data)->clear();
}

} // namespace QAccelPlot::Internal
