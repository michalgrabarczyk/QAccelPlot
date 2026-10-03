//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QAccelPlot/inspection/InspectionRowModel.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <QSignalSpy>
#include <QtTest>

#include <memory>

using namespace QAccelPlot;

namespace {

InspectionRow rowFor(PlotSeries* series, const int index)
{
    auto row = InspectionRow{};
    row.series = series;
    row.sample.status = InspectionStatus::Ready;
    row.sample.index = index;
    row.sample.position = {1.5, 2.5};
    row.xText = QStringLiteral("1.5");
    return row;
}

} // namespace

class TestInspectionRowModel : public QObject {
    Q_OBJECT
private slots:
    void get_returnsEveryRoleByName();
    void setRows_updatesInPlaceWhileSeriesAreUnchanged();
    void retainSeries_removesMissingAndDestroyedSeries();
};

void TestInspectionRowModel::get_returnsEveryRoleByName()
{
    auto curve = LineCurve{};
    curve.setName(QStringLiteral("signal"));
    curve.setColor(QColor{Qt::red});
    auto model = InspectionRowModel{};
    auto row = rowFor(&curve, 7);
    row.hasSummary = true;
    row.summary.status = InspectionStatus::Ready;
    row.summary.count = 3;
    row.summary.maximum = 9.0;
    model.setRows({row});

    const auto values = model.get(0);
    QCOMPARE(values.size(), model.roleNames().size());
    QCOMPARE(values.value("series").value<PlotSeries*>(), &curve);
    QCOMPARE(values.value("seriesName").toString(), QStringLiteral("signal"));
    QCOMPARE(values.value("seriesColor").value<QColor>(), QColor{Qt::red});
    QVERIFY(values.value("valid").toBool());
    QCOMPARE(values.value("sampleIndex").toInt(), 7);
    QCOMPARE(values.value("sampleX").toDouble(), 1.5);
    QCOMPARE(values.value("sampleY").toDouble(), 2.5);
    QCOMPARE(values.value("xText").toString(), QStringLiteral("1.5"));
    QVERIFY(values.value("hasSummary").toBool());
    QCOMPARE(values.value("summaryCount").toInt(), 3);
    QCOMPARE(values.value("maximum").toDouble(), 9.0);
    QCOMPARE(model.data(model.index(0), InspectionRowModel::SampleIndexRole).toInt(), 7);
    // A series without a name falls back to its object name.
    curve.setName({});
    curve.setObjectName(QStringLiteral("fallback"));
    QCOMPARE(model.get(0).value("seriesName").toString(), QStringLiteral("fallback"));
    QVERIFY(model.get(1).isEmpty());
    QVERIFY(model.get(-1).isEmpty());
}

void TestInspectionRowModel::setRows_updatesInPlaceWhileSeriesAreUnchanged()
{
    auto first = LineCurve{};
    auto second = LineCurve{};
    auto model = InspectionRowModel{};
    auto resets = QSignalSpy{&model, &QAbstractItemModel::modelReset};
    auto changes = QSignalSpy{&model, &QAbstractItemModel::dataChanged};
    auto counts = QSignalSpy{&model, &InspectionRowModel::countChanged};

    model.setRows({rowFor(&first, 1), rowFor(&second, 2)});
    QCOMPARE(model.count(), 2);
    QCOMPARE(resets.count(), 1);
    QCOMPARE(counts.count(), 1);

    // The same series in the same order keep their rows, so delegates are reused.
    model.setRows({rowFor(&first, 5), rowFor(&second, 6)});
    QCOMPARE(resets.count(), 1);
    QCOMPARE(changes.count(), 1);
    QCOMPARE(changes.first().at(0).toModelIndex().row(), 0);
    QCOMPARE(changes.first().at(1).toModelIndex().row(), 1);
    QCOMPARE(model.get(1).value("sampleIndex").toInt(), 6);

    // A different set of series replaces the rows.
    model.setRows({rowFor(&second, 6), rowFor(&first, 5)});
    QCOMPARE(resets.count(), 2);
    QCOMPARE(counts.count(), 1);
    model.setRows({});
    QCOMPARE(model.count(), 0);
    QCOMPARE(counts.count(), 2);
}

void TestInspectionRowModel::retainSeries_removesMissingAndDestroyedSeries()
{
    auto kept = LineCurve{};
    auto dropped = LineCurve{};
    auto destroyed = std::make_unique<LineCurve>();
    auto model = InspectionRowModel{};
    model.setRows({rowFor(&kept, 0), rowFor(&dropped, 1), rowFor(destroyed.get(), 2)});
    auto removals = QSignalSpy{&model, &QAbstractItemModel::rowsRemoved};
    auto resets = QSignalSpy{&model, &QAbstractItemModel::modelReset};

    destroyed.reset();
    model.retainSeries({&kept});
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.get(0).value("series").value<PlotSeries*>(), &kept);
    QCOMPARE(removals.count(), 2);
    QCOMPARE(resets.count(), 0);
    model.retainSeries({&kept});
    QCOMPARE(removals.count(), 2);
}

QTEST_MAIN(TestInspectionRowModel)
#include "tst_inspection_row_model.moc"
