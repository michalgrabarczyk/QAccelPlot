//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

// Collects the warnings logged while it exists.
class WarningLog {
public:
    WarningLog()
    {
        messages().clear();
        previous_ = qInstallMessageHandler(&WarningLog::handle);
    }

    ~WarningLog()
    {
        qInstallMessageHandler(previous_);
    }

    WarningLog(const WarningLog&) = delete;
    WarningLog& operator=(const WarningLog&) = delete;

    static QStringList& messages()
    {
        static auto list = QStringList{};
        return list;
    }

private:
    static void handle(const QtMsgType type, [[maybe_unused]] const QMessageLogContext& context, const QString& message)
    {
        if (type == QtWarningMsg) {
            messages().append(message);
        }
    }

    QtMessageHandler previous_;
};

// Reports how many connections its effectChanged() signal has.
class ConnectionCountingFill : public GradientFill {
public:
    int effectChangedConnections() const
    {
        return receivers(SIGNAL(effectChanged()));
    }
};

void appendEffect(PlotSeries& series, SeriesEffect* effect)
{
    auto list = series.effects();
    list.append(&list, effect);
}

} // namespace

class SeriesEffectsTest : public QObject {
    Q_OBJECT

private slots:
    void supportedEffectsAreListed();
    void unsupportedEffectWarnsAndStaysListed();
    void repeatedEffectTypeWarns();
    void destroyedEffectLeavesTheList();
    void clearingTheListRemovesEveryEffect();
    void clearedEffectNoLongerRedrawsTheSeries();
    void seriesFromQmlNamesItselfInTheWarning();
};

void SeriesEffectsTest::supportedEffectsAreListed()
{
    auto curve = LineCurve{};
    auto fill = GradientFill{};
    auto stroke = GradientStroke{};
    const auto log = WarningLog{};

    appendEffect(curve, &fill);
    appendEffect(curve, &stroke);

    auto list = curve.effects();
    QCOMPARE(list.count(&list), 2);
    QCOMPARE(list.at(&list, 0), &fill);
    QVERIFY(WarningLog::messages().isEmpty());
}

void SeriesEffectsTest::unsupportedEffectWarnsAndStaysListed()
{
    auto bars = BarSeries{};
    bars.setObjectName(QStringLiteral("revenue"));
    auto fill = GradientFill{};
    const auto log = WarningLog{};

    appendEffect(bars, &fill);

    QCOMPARE(WarningLog::messages(), QStringList{QStringLiteral(R"(GradientFill is not supported by BarSeries "revenue"; the effect is ignored)")});
    auto list = bars.effects();
    QCOMPARE(list.count(&list), 1);
}

void SeriesEffectsTest::repeatedEffectTypeWarns()
{
    auto curve = LineCurve{};
    auto first = GradientFill{};
    auto second = GradientFill{};
    const auto log = WarningLog{};

    appendEffect(curve, &first);
    appendEffect(curve, &second);

    QCOMPARE(WarningLog::messages(),
        QStringList{QStringLiteral("GradientFill appears more than once in the effects of LineCurve; only the first enabled one applies")});
}

void SeriesEffectsTest::destroyedEffectLeavesTheList()
{
    auto curve = LineCurve{};
    auto fill = std::make_unique<GradientFill>();
    appendEffect(curve, fill.get());

    fill.reset();

    auto list = curve.effects();
    QCOMPARE(list.count(&list), 0);
}

void SeriesEffectsTest::clearingTheListRemovesEveryEffect()
{
    auto curve = LineCurve{};
    auto fill = GradientFill{};
    auto stroke = GradientStroke{};
    appendEffect(curve, &fill);
    appendEffect(curve, &stroke);

    auto list = curve.effects();
    list.clear(&list);

    QCOMPARE(list.count(&list), 0);
}

void SeriesEffectsTest::clearedEffectNoLongerRedrawsTheSeries()
{
    auto curve = LineCurve{};
    auto fill = ConnectionCountingFill{};
    const auto connectionsBefore = fill.effectChangedConnections();
    appendEffect(curve, &fill);
    QCOMPARE(fill.effectChangedConnections(), connectionsBefore + 1);

    auto list = curve.effects();
    list.clear(&list);
    QCOMPARE(fill.effectChangedConnections(), connectionsBefore);

    appendEffect(curve, &fill);
    QCOMPARE(fill.effectChangedConnections(), connectionsBefore + 1);
}

void SeriesEffectsTest::seriesFromQmlNamesItselfInTheWarning()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    // The objectName is assigned after the effects; the warning waits for it.
    component.setData(R"(
        import QAccelPlot
        BarSeries {
            effects: [GradientStroke {}]
            objectName: "profit"
        })",
        QUrl{});
    const auto log = WarningLog{};

    const auto bars = std::unique_ptr<QObject>{component.create()};

    QVERIFY2(bars, qPrintable(component.errorString()));
    QCOMPARE(WarningLog::messages(), QStringList{QStringLiteral(R"(GradientStroke is not supported by BarSeries "profit"; the effect is ignored)")});
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::SeriesEffectsTest)
#include "tst_series_effects.moc"
