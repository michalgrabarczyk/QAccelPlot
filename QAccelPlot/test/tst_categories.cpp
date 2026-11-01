//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/Category.hpp"
#include "QAccelPlot/series/internal/CategoryList.hpp"
#include "QAccelPlot/series/internal/CategoryTable.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

#include <limits>
#include <memory>

namespace QAccelPlot {

using Internal::CategoryList;
using Internal::CategoryTable;

class CategoriesTest : public QObject {
    Q_OBJECT

private slots:
    void categoryNotifiesItsProperties();
    void tableLooksUpColorsAndNamesByValue();
    void tableFindsValuesFarApart();
    void tableKeepsTheFirstOfSharedValuesAndNames();
    void tableResolvesDataPointValuesAndNames();
    void tableGivesOneLegendEntryPerCategory();
    void listFollowsEditsAndDestruction();
    void listReportsProblemsOnceBindingsSettle();
};

void CategoriesTest::categoryNotifiesItsProperties()
{
    auto category = Category{};
    QCOMPARE(category.value(), 0);
    QVERIFY(category.name().isEmpty());
    QVERIFY(!category.color().isValid());

    auto valueSpy = QSignalSpy{&category, &Category::valueChanged};
    auto nameSpy = QSignalSpy{&category, &Category::nameChanged};
    auto colorSpy = QSignalSpy{&category, &Category::colorChanged};
    category.setValue(3);
    category.setValue(3);
    category.setName(QStringLiteral("Idle"));
    category.setColor(Qt::gray);
    QCOMPARE(valueSpy.count(), 1);
    QCOMPARE(nameSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 1);
}

void CategoriesTest::tableLooksUpColorsAndNamesByValue()
{
    auto idle = Category{3, QStringLiteral("Idle"), Qt::gray};
    auto alarm = Category{-2, {}, Qt::red};
    const auto table = CategoryTable::fromCategories({&idle, nullptr, &alarm});

    QVERIFY(!table.empty());
    QCOMPARE(table.entries().size(), std::size_t{2});
    QVERIFY(table.problems().isEmpty());
    QCOMPARE(table.color(3), QColor{Qt::gray});
    QCOMPARE(table.name(3), QStringLiteral("Idle"));
    QCOMPARE(table.color(-2), QColor{Qt::red});
    QVERIFY(table.name(-2).isEmpty());
    QVERIFY(!table.color(0).isValid());
    QVERIFY(CategoryTable{}.empty());
    QVERIFY(!CategoryTable{}.color(0).isValid());
}

void CategoriesTest::tableFindsValuesFarApart()
{
    constexpr auto kLowest = std::numeric_limits<int>::min();
    constexpr auto kHighest = std::numeric_limits<int>::max();
    auto lowest = Category{kLowest, {}, Qt::red};
    auto highest = Category{kHighest, {}, Qt::blue};
    const auto table = CategoryTable::fromCategories({&lowest, &highest});

    QCOMPARE(table.color(kLowest), QColor{Qt::red});
    QCOMPARE(table.color(kHighest), QColor{Qt::blue});
    QVERIFY(!table.color(0).isValid());
}

void CategoriesTest::tableKeepsTheFirstOfSharedValuesAndNames()
{
    auto run = Category{0, QStringLiteral("Run"), Qt::green};
    auto stop = Category{0, QStringLiteral("Stop"), Qt::red};
    auto idle = Category{1, QStringLiteral("Idle"), {}};
    auto idleAgain = Category{2, QStringLiteral("Idle"), {}};
    const auto table = CategoryTable::fromCategories({&run, &stop, &idle, &idleAgain});

    QCOMPARE(table.entries().size(), std::size_t{3});
    QCOMPARE(table.color(0), QColor{Qt::green});
    QCOMPARE(table.name(2), QStringLiteral("Idle"));
    QCOMPARE(table.dataPointCategory(QStringLiteral("Idle"), "Series", 0), std::optional<int>{1});
    QCOMPARE(table.problems().size(), 2);
    QCOMPARE(table.problems()[0], QStringLiteral("skips category 1: an earlier category has the value 0"));
    QVERIFY(table.problems()[1].startsWith(QStringLiteral("finds the category name \"Idle\" more than once")));
}

void CategoriesTest::tableResolvesDataPointValuesAndNames()
{
    auto alarm = Category{7, QStringLiteral("Alarm"), {}};
    const auto table = CategoryTable::fromCategories({&alarm});

    QCOMPARE(table.dataPointCategory(QStringLiteral("Alarm"), "Series", 0), std::optional<int>{7});
    QCOMPARE(table.dataPointCategory(12, "Series", 0), std::optional<int>{12});
    QCOMPARE(table.dataPointCategory(-1.0, "Series", 0), std::optional<int>{-1});
    QCOMPARE(table.dataPointCategory(QVariant{}, "Series", 0), std::optional<int>{});
    QCOMPARE(table.dataPointCategory(QVariant::fromValue(nullptr), "Series", 0), std::optional<int>{});

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Series ignores the category of data point 4: no category is named \"Idle\""));
    QCOMPARE(table.dataPointCategory(QStringLiteral("Idle"), "Series", 4), std::optional<int>{});
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Series ignores the category .*2\\.5.* of data point 5"));
    QCOMPARE(table.dataPointCategory(2.5, "Series", 5), std::optional<int>{});
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Series ignores the category .* of data point 6"));
    QCOMPARE(table.dataPointCategory(1e12, "Series", 6), std::optional<int>{});
}

void CategoriesTest::tableGivesOneLegendEntryPerCategory()
{
    auto run = Category{5, QStringLiteral("Run"), Qt::green};
    auto unnamed = Category{9, {}, {}};
    const auto table = CategoryTable::fromCategories({&run, &unnamed});
    auto seriesEntry = LegendEntry{};
    seriesEntry.symbol = PlotSeries::LegendSymbol::Fill;
    seriesEntry.color = Qt::gray;

    const auto entries = table.legendEntries(seriesEntry);
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries[0].name, QStringLiteral("Run"));
    QCOMPARE(entries[0].color, QColor{Qt::green});
    QCOMPARE(entries[0].symbol, PlotSeries::LegendSymbol::Fill);
    QCOMPARE(entries[1].name, QStringLiteral("9"));
    QCOMPARE(entries[1].color, QColor{Qt::gray});
}

void CategoriesTest::listFollowsEditsAndDestruction()
{
    auto list = CategoryList{"Series"};
    auto changed = QSignalSpy{&list, &CategoryList::changed};
    auto kept = Category{0, QStringLiteral("Run"), Qt::green};
    auto removed = std::make_unique<Category>(1, QStringLiteral("Stop"), Qt::red);

    auto property = list.listProperty(&list);
    property.append(&property, &kept);
    property.append(&property, removed.get());
    QCOMPARE(property.count(&property), 2);
    QCOMPARE(property.at(&property, 1), removed.get());
    QCOMPARE(changed.count(), 2);

    kept.setColor(Qt::yellow);
    QCOMPARE(list.table().color(0), QColor{Qt::yellow});
    QCOMPARE(changed.count(), 3);

    removed.reset();
    QCOMPARE(property.count(&property), 1);
    QVERIFY(!list.table().color(1).isValid());
    QCOMPARE(changed.count(), 4);

    property.clear(&property);
    QVERIFY(list.table().empty());
    kept.setValue(4);
    QCOMPARE(changed.count(), 5);

    list.setCategories({&kept});
    QCOMPARE(list.table().name(4), QStringLiteral("Run"));
    QCOMPARE(changed.count(), 6);
}

void CategoriesTest::listReportsProblemsOnceBindingsSettle()
{
    auto list = CategoryList{"Series"};
    auto first = Category{};
    auto second = Category{};
    // Like QML bindings that have not run yet: both categories still have value 0.
    list.setCategories({&first, &second});
    second.setValue(1);
    static auto warnings = QStringList{};
    warnings.clear();
    const auto previousHandler = qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& message) { warnings.append(message); });
    QCoreApplication::processEvents();
    qInstallMessageHandler(previousHandler);
    QVERIFY(warnings.isEmpty());

    second.setValue(0);
    QTest::ignoreMessage(QtWarningMsg, "Series skips category 1: an earlier category has the value 0");
    QCoreApplication::processEvents();
}

} // namespace QAccelPlot

QTEST_GUILESS_MAIN(QAccelPlot::CategoriesTest)
#include "tst_categories.moc"
