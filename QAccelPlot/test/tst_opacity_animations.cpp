//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "OpacityAnimations.hpp"
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/internal/OpacityAnimations.hpp"
#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest/QtTest>

#include <cmath>
#include <memory>

namespace QAccelPlot {

using Internal::OpacitySegment;
using Internal::OpacityTimeline;

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

bool near(const double actual, const double expected)
{
    return std::abs(actual - expected) < 1e-6;
}

// Reads the animation that `qml` declares, collecting the problems into `problems`.
OpacityTimeline read(const QString& qml, QStringList* problems = nullptr)
{
    auto parent = QObject{};
    auto* animation = Test::createQuickObject(qml, &parent);
    return Internal::readOpacityAnimations({animation}, problems);
}

} // namespace

class OpacityAnimationsTest : public QObject {
    Q_OBJECT

private slots:
    // Timeline
    void timeline_withoutSegments_isOpaque();
    void timeline_followsTheEasing();
    void timeline_missingFrom_continuesFromTheValueReached();
    void timeline_beforeTheFirstSegment_showsItsStart();
    void timeline_lengthIncludesATrailingPause();

    // Reading Qt Quick animations
    void numberAnimation_isRead();
    void missingTo_isOpaque();
    void sequentialAnimation_playsInTurn();
    void parallelAnimation_lastsAsLongAsItsLongestChild();
    void loops_repeatTheAnimation();
    void loops_repeatASequence();
    void propertyAction_setsTheOpacityAtOnce();
    void properties_canNameOpacityAmongOthers();
    void otherProperty_onlyAddsItsDuration();
    void unsupportedAnimations_areReported_data();
    void unsupportedAnimations_areReported();
    void infiniteLoops_playOnce();

    // DataTransition
    void transitionFromQml_readsItsAnimations();
    void transitionFromQml_warnsOnceItLoaded();
    void transitionFromQml_rejectsAnimationOnOpacity();
    void animationAddedLater_isReported();
    void destroyedAnimation_leavesTheTransition();
};

void OpacityAnimationsTest::timeline_withoutSegments_isOpaque()
{
    const auto timeline = OpacityTimeline{};

    QVERIFY(timeline.empty());
    QCOMPARE(timeline.length(), 0.0);
    QCOMPARE(timeline.valueAt(100.0), 1.0);
}

void OpacityAnimationsTest::timeline_followsTheEasing()
{
    const auto timeline = OpacityTimeline{{OpacitySegment{0.0, 100.0, 0.0, 1.0, QEasingCurve{QEasingCurve::InQuad}}}, 0.0};

    QCOMPARE(timeline.length(), 100.0);
    QVERIFY(near(timeline.valueAt(50.0), 0.25));
    QCOMPARE(timeline.valueAt(500.0), 1.0);
}

void OpacityAnimationsTest::timeline_missingFrom_continuesFromTheValueReached()
{
    const auto timeline = OpacityTimeline{{OpacitySegment{0.0, 100.0, std::nullopt, 0.4, {}}, OpacitySegment{100.0, 100.0, std::nullopt, 1.0, {}}}, 0.0};

    QVERIFY(near(timeline.valueAt(50.0), 0.7));
    QVERIFY(near(timeline.valueAt(150.0), 0.7));
    QCOMPARE(timeline.valueAt(200.0), 1.0);
}

void OpacityAnimationsTest::timeline_beforeTheFirstSegment_showsItsStart()
{
    const auto timeline = OpacityTimeline{{OpacitySegment{200.0, 100.0, 0.0, 1.0, {}}}, 0.0};

    QCOMPARE(timeline.valueAt(100.0), 0.0);
}

void OpacityAnimationsTest::timeline_lengthIncludesATrailingPause()
{
    QCOMPARE((OpacityTimeline{{OpacitySegment{0.0, 100.0, 0.0, 1.0, {}}}, 500.0}.length()), 500.0);
}

void OpacityAnimationsTest::numberAnimation_isRead()
{
    const auto timeline = read(R"(NumberAnimation { property: "opacity"; from: 0; to: 0.5; duration: 200; easing.type: Easing.InQuad })");

    QCOMPARE(timeline.length(), 200.0);
    QVERIFY(near(timeline.valueAt(100.0), 0.125));
    QCOMPARE(timeline.valueAt(200.0), 0.5);
}

void OpacityAnimationsTest::missingTo_isOpaque()
{
    const auto timeline = read(R"(NumberAnimation { property: "opacity"; from: 0; duration: 100 })");

    QCOMPARE(timeline.valueAt(0.0), 0.0);
    QCOMPARE(timeline.valueAt(100.0), 1.0);
}

void OpacityAnimationsTest::sequentialAnimation_playsInTurn()
{
    const auto timeline = read(R"(SequentialAnimation {
        NumberAnimation { property: "opacity"; to: 0.4; duration: 200 }
        PauseAnimation { duration: 100 }
        NumberAnimation { property: "opacity"; to: 1; duration: 100 }
    })");

    QCOMPARE(timeline.length(), 400.0);
    QVERIFY(near(timeline.valueAt(100.0), 0.7));
    QVERIFY(near(timeline.valueAt(250.0), 0.4));
    QVERIFY(near(timeline.valueAt(350.0), 0.7));
}

void OpacityAnimationsTest::parallelAnimation_lastsAsLongAsItsLongestChild()
{
    const auto timeline = read(R"(ParallelAnimation {
        NumberAnimation { property: "opacity"; from: 0; duration: 100 }
        PauseAnimation { duration: 300 }
    })");

    QCOMPARE(timeline.length(), 300.0);
    QCOMPARE(timeline.valueAt(100.0), 1.0);
}

void OpacityAnimationsTest::loops_repeatTheAnimation()
{
    const auto timeline = read(R"(NumberAnimation { property: "opacity"; from: 0; duration: 100; loops: 3 })");

    QCOMPARE(timeline.length(), 300.0);
    QVERIFY(near(timeline.valueAt(150.0), 0.5));
}

void OpacityAnimationsTest::loops_repeatASequence()
{
    // Blinks twice.
    const auto timeline = read(R"(SequentialAnimation {
        loops: 2
        NumberAnimation { property: "opacity"; to: 0; duration: 100 }
        NumberAnimation { property: "opacity"; to: 1; duration: 100 }
    })");

    QCOMPARE(timeline.length(), 400.0);
    QVERIFY(near(timeline.valueAt(150.0), 0.5));
    QVERIFY(near(timeline.valueAt(250.0), 0.5));
    QCOMPARE(timeline.valueAt(300.0), 0.0);
}

void OpacityAnimationsTest::propertyAction_setsTheOpacityAtOnce()
{
    const auto timeline = read(R"(SequentialAnimation {
        PropertyAction { property: "opacity"; value: 0 }
        PauseAnimation { duration: 100 }
        NumberAnimation { property: "opacity"; duration: 100 }
    })");

    QCOMPARE(timeline.valueAt(50.0), 0.0);
    QVERIFY(near(timeline.valueAt(150.0), 0.5));
}

void OpacityAnimationsTest::properties_canNameOpacityAmongOthers()
{
    QVERIFY(!read(R"(NumberAnimation { properties: "x, opacity"; from: 0; duration: 100 })").empty());
}

void OpacityAnimationsTest::otherProperty_onlyAddsItsDuration()
{
    auto problems = QStringList{};
    const auto timeline = read(R"(SequentialAnimation {
        NumberAnimation { property: "x"; duration: 100 }
        NumberAnimation { property: "opacity"; from: 0; duration: 100 }
    })",
        &problems);

    QVERIFY(near(timeline.valueAt(150.0), 0.5));
    QCOMPARE(problems,
        QStringList{QStringLiteral(R"(NumberAnimation does not animate "opacity", the only property a transition animates; it only adds its duration)")});
}

void OpacityAnimationsTest::unsupportedAnimations_areReported_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("problem");

    QTest::newRow("spring") << R"(SpringAnimation { property: "opacity" })" << "SpringAnimation cannot play in a transition and is ignored";
    QTest::newRow("smoothed") << R"(SmoothedAnimation { property: "opacity" })" << "SmoothedAnimation cannot play in a transition and is ignored";
    QTest::newRow("script") << R"(ScriptAction { script: {} })" << "ScriptAction cannot play in a transition and is ignored";
    QTest::newRow("not an animation") << "QtObject {}"
                                      << "QObject is not an animation and is ignored";
    QTest::newRow("color") << R"(ColorAnimation { property: "opacity"; from: "red" })"
                           << R"(ColorAnimation of "opacity" needs numbers for from and to; it only adds its duration)";
}

void OpacityAnimationsTest::unsupportedAnimations_areReported()
{
    QFETCH(QString, qml);
    QFETCH(QString, problem);
    auto problems = QStringList{};

    QVERIFY(read(qml, &problems).empty());
    QCOMPARE(problems, QStringList{problem});
}

void OpacityAnimationsTest::infiniteLoops_playOnce()
{
    auto problems = QStringList{};

    const auto timeline = read(R"(NumberAnimation { property: "opacity"; from: 0; duration: 100; loops: Animation.Infinite })", &problems);

    QCOMPARE(timeline.length(), 100.0);
    QCOMPARE(problems, QStringList{QStringLiteral("NumberAnimation with infinite loops plays once in a transition")});
}

void OpacityAnimationsTest::transitionFromQml_readsItsAnimations()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot
        DrawTransition {
            stagger: 0.5
            NumberAnimation { property: "opacity"; from: 0; duration: 100 }
            NumberAnimation { property: "opacity"; to: 0.5; duration: 300 }
        })",
        QUrl{});

    const auto object = std::unique_ptr<QObject>{component.create()};

    auto* transition = qobject_cast<DataTransition*>(object.get());
    QVERIFY2(transition, qPrintable(component.errorString()));
    auto list = transition->animations();
    QCOMPARE(list.count(&list), 2);
    QCOMPARE(transition->opacityTimeline().length(), 300.0);
}

void OpacityAnimationsTest::transitionFromQml_warnsOnceItLoaded()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot
        DrawTransition {
            SpringAnimation { property: "opacity" }
        })",
        QUrl{});
    const auto log = WarningLog{};

    const auto object = std::unique_ptr<QObject>{component.create()};

    QVERIFY2(object, qPrintable(component.errorString()));
    QCOMPARE(WarningLog::messages(), QStringList{QStringLiteral("DrawTransition: SpringAnimation cannot play in a transition and is ignored")});
}

void OpacityAnimationsTest::transitionFromQml_rejectsAnimationOnOpacity()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};

    // A transition has no opacity to animate live: each item has its own.
    component.setData(R"(
        import QtQuick
        import QAccelPlot
        DrawTransition {
            NumberAnimation on opacity { from: 0 }
        })",
        QUrl{});

    QVERIFY(component.isError());
}

void OpacityAnimationsTest::animationAddedLater_isReported()
{
    auto transition = DrawTransition{};
    const auto log = WarningLog{};

    Test::addAnimation(transition, R"(NumberAnimation { property: "x" })");

    QCOMPARE(WarningLog::messages(),
        QStringList{QStringLiteral(
            R"(DrawTransition: NumberAnimation does not animate "opacity", the only property a transition animates; it only adds its duration)")});
}

void OpacityAnimationsTest::destroyedAnimation_leavesTheTransition()
{
    auto transition = DrawTransition{};
    auto* animation = Test::addAnimation(transition, R"(NumberAnimation { property: "opacity"; from: 0 })");

    delete animation;

    auto list = transition.animations();
    QCOMPARE(list.count(&list), 0);
    QVERIFY(transition.opacityTimeline().empty());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::OpacityAnimationsTest)
#include "tst_opacity_animations.moc"
