

# File DataTransition.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**transitions**](dir_33e4f9f956353311d613e58793c6ec62.md) **>** [**DataTransition.hpp**](DataTransition_8hpp.md)

[Go to the documentation of this file](DataTransition_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QEasingCurve>
#include <QList>
#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

QT_FORWARD_DECLARE_CLASS(QAbstractAnimation)

namespace QAccelPlot {

namespace Internal {
class OpacityTimeline;
} // namespace Internal

class DataTransition : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ANONYMOUS
    Q_CLASSINFO("DefaultProperty", "animations")

    
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(qreal stagger READ stagger WRITE setStagger NOTIFY staggerChanged)
    Q_PROPERTY(QQmlListProperty<QObject> animations READ animations)

public:
    class Run {
    public:
        Run() = default;
        ~Run();

        Run(const Run&) = delete;
        Run& operator=(const Run&) = delete;

        bool active() const;
        DataTransition* transition() const;
        void end();

    private:
        friend class DataTransition;

        DataTransition* transition_{nullptr};
        int startTime_{0};
    };

    ~DataTransition() override;

    int duration() const;
    void setDuration(int duration);

    QEasingCurve easing() const;
    void setEasing(const QEasingCurve& easing);

    bool enabled() const;
    void setEnabled(bool enabled);

    bool running() const;

    qreal stagger() const;
    void setStagger(qreal stagger);

    QQmlListProperty<QObject> animations();

    void start(Run& run);
    void cancel();
    double progress(const Run& run) const;

    Internal::OpacityTimeline opacityTimeline() const;

    void classBegin() override;
    void componentComplete() override;

signals:
    void durationChanged();
    void easingChanged();
    void enabledChanged();
    void runningChanged();
    void staggerChanged();

private:
    // Only the transitions of the library derive from DataTransition.
    friend class MorphTransition;
    friend class EnterExitTransition;

    explicit DataTransition(QObject* parent = nullptr);

    static void appendAnimation(QQmlListProperty<QObject>* list, QObject* animation);
    static qsizetype animationCount(QQmlListProperty<QObject>* list);
    static QObject* animationAt(QQmlListProperty<QObject>* list, qsizetype index);
    static void clearAnimations(QQmlListProperty<QObject>* list);

    void removeRun(Run* run);
    void setRunning(bool running);
    void warnAboutIgnored(const QList<QObject*>& animations) const;

    int duration_{300};
    QEasingCurve easing_{QEasingCurve(QEasingCurve::Linear)};
    bool enabled_{true};
    bool running_{false};
    qreal stagger_{0.0};
    QList<Run*> runs_;
    QList<QObject*> animations_;
    bool loading_{false};
    QAbstractAnimation* clock_;
};

} // namespace QAccelPlot
```


