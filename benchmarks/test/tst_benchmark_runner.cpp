//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkRunner.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QMetaObject>
#include <QtTest/QtTest>

class TestBenchmarkRunner : public QObject {
    Q_OBJECT

private slots:
    void stopBeforeQueuedStartLeavesNoActiveScenario();
    void stopFromCompletionSuppressesAllCompleted();
};

void TestBenchmarkRunner::stopBeforeQueuedStartLeavesNoActiveScenario()
{
    auto runner = QAccelPlot::BenchmarkRunner{};
    const auto name = QStringLiteral("queued-stop");
    runner.addCustomScenario(name, name, QString{}, 2, 1, static_cast<int>(QAccelPlot::BenchmarkScenario::UpdateMode::Static), 10, 0);

    runner.runScenario(name);
    QVERIFY(runner.isRunning());
    runner.stop();
    QCoreApplication::sendPostedEvents(&runner, QEvent::MetaCall);

    QVERIFY(!runner.isRunning());
    QCOMPARE(runner.currentScenarioName(), QString{});
}

void TestBenchmarkRunner::stopFromCompletionSuppressesAllCompleted()
{
    auto runner = QAccelPlot::BenchmarkRunner{};
    const auto name = QStringLiteral("stop-on-completion");
    runner.addCustomScenario(name, name, QString{}, 2, 1, static_cast<int>(QAccelPlot::BenchmarkScenario::UpdateMode::Static), 10, 0);
    auto allCompletedSpy = QSignalSpy{&runner, &QAccelPlot::BenchmarkRunner::allCompleted};
    QObject::connect(&runner, &QAccelPlot::BenchmarkRunner::scenarioCompleted, &runner, &QAccelPlot::BenchmarkRunner::stop);

    runner.runScenario(name);
    QCoreApplication::sendPostedEvents(&runner, QEvent::MetaCall);
    QVERIFY(runner.isRunning());
    QVERIFY(QMetaObject::invokeMethod(&runner, "finishScenario", Qt::DirectConnection));
    QCoreApplication::sendPostedEvents(&runner, QEvent::MetaCall);

    QVERIFY(!runner.isRunning());
    QCOMPARE(allCompletedSpy.count(), 0);
}

QTEST_MAIN(TestBenchmarkRunner)
#include "tst_benchmark_runner.moc"
