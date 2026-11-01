//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/CategoryTable.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/series/Category.hpp"

#include <QMetaType>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot::Internal {

namespace {

// The widest span of values indexed densely, at one int per value in the span.
constexpr auto kMaxDenseSpan = qint64{65536};

bool isString(const QVariant& value)
{
    return value.metaType().id() == QMetaType::QString;
}

// Returns \a value as an int when it is a number with no fractional part in int range.
std::optional<int> wholeNumber(const QVariant& value)
{
    if (isString(value)) {
        return std::nullopt;
    }
    auto isNumber = false;
    const auto number = value.toDouble(&isNumber);
    const auto inRange = number >= std::numeric_limits<int>::min() && number <= std::numeric_limits<int>::max();
    if (!isNumber || !inRange || std::trunc(number) != number) {
        return std::nullopt;
    }
    return static_cast<int>(number);
}

bool isSet(const QVariant& value)
{
    return value.isValid() && !value.isNull();
}

} // namespace

CategoryTable CategoryTable::fromCategories(const QList<Category*>& categories)
{
    auto table = CategoryTable{};
    auto values = QSet<int>{};
    for (qsizetype i = 0; i < categories.size(); ++i) {
        const auto* category = categories[i];
        if (!category) {
            continue;
        }
        if (values.contains(category->value())) {
            table.problems_.append(QStringLiteral("skips category %1: an earlier category has the value %2").arg(i).arg(category->value()));
            continue;
        }
        values.insert(category->value());
        if (!category->name().isEmpty()) {
            if (table.valueByName_.contains(category->name())) {
                table.problems_.append(
                    QStringLiteral("finds the category name \"%1\" more than once; data points that use it get the first category").arg(category->name()));
            } else {
                table.valueByName_.insert(category->name(), category->value());
            }
        }
        table.entries_.push_back(Entry{category->value(), category->name(), category->color()});
    }
    table.buildValueIndex();
    return table;
}

bool CategoryTable::empty() const
{
    return entries_.empty();
}

const std::vector<CategoryTable::Entry>& CategoryTable::entries() const
{
    return entries_;
}

const QStringList& CategoryTable::problems() const
{
    return problems_;
}

QColor CategoryTable::color(const int value) const
{
    const auto* category = find(value);
    return category ? category->color : QColor{};
}

QString CategoryTable::name(const int value) const
{
    const auto* category = find(value);
    return category ? category->name : QString{};
}

std::optional<int> CategoryTable::dataPointCategory(const QVariant& value, const char* seriesName, const qsizetype index) const
{
    if (!isSet(value)) {
        return std::nullopt;
    }
    if (isString(value)) {
        const auto name = value.toString();
        const auto found = valueByName_.constFind(name);
        if (found != valueByName_.cend()) {
            return *found;
        }
        qCWarning(lcQAccelPlot).nospace() << seriesName << " ignores the category of data point " << index << ": no category is named " << name;
        return std::nullopt;
    }
    if (const auto number = wholeNumber(value)) {
        return number;
    }
    qCWarning(lcQAccelPlot).nospace() << seriesName << " ignores the category " << value << " of data point " << index
                                      << ": a category is a whole-number value or a category name";
    return std::nullopt;
}

QList<LegendEntry> CategoryTable::legendEntries(const LegendEntry& seriesEntry) const
{
    auto entries = QList<LegendEntry>{};
    entries.reserve(static_cast<qsizetype>(entries_.size()));
    for (const auto& category : entries_) {
        auto entry = seriesEntry;
        entry.name = category.name.isEmpty() ? QString::number(category.value) : category.name;
        if (category.color.isValid()) {
            entry.color = category.color;
        }
        entries.append(entry);
    }
    return entries;
}

const CategoryTable::Entry* CategoryTable::find(const int value) const
{
    auto index = -1;
    if (!denseIndex_.empty()) {
        const auto offset = qint64{value} - denseMin_;
        if (offset >= 0 && offset < static_cast<qint64>(denseIndex_.size())) {
            index = denseIndex_[static_cast<std::size_t>(offset)];
        }
    } else {
        index = hashedIndex_.value(value, -1);
    }
    return index < 0 ? nullptr : &entries_[static_cast<std::size_t>(index)];
}

void CategoryTable::buildValueIndex()
{
    if (entries_.empty()) {
        return;
    }
    const auto [minIt, maxIt]
        = std::minmax_element(entries_.cbegin(), entries_.cend(), [](const Entry& left, const Entry& right) { return left.value < right.value; });
    const auto span = qint64{maxIt->value} - minIt->value + 1;
    if (span <= kMaxDenseSpan) {
        denseMin_ = minIt->value;
        denseIndex_.assign(static_cast<std::size_t>(span), -1);
        for (std::size_t i = 0; i < entries_.size(); ++i) {
            denseIndex_[static_cast<std::size_t>(entries_[i].value - denseMin_)] = static_cast<int>(i);
        }
        return;
    }
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        hashedIndex_.insert(entries_[i].value, static_cast<int>(i));
    }
}

} // namespace QAccelPlot::Internal
