

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
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QtQml/qqmlregistration.h>

#include <vector>

namespace QAccelPlot {

class DataTransition : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    class Run {
    public:
        Run() = default;
        ~Run();

        Run(const Run&) = delete;
        Run& operator=(const Run&) = delete;

        bool active() const;
        bool pending() const;
        const std::vector<double>& targetData() const;
        int targetPointCount() const;

        bool finish(std::vector<double>& outData, int& outPointCount);
        void cancel();

    private:
        friend class DataTransition;

        void detach();

        DataTransition* transition_{nullptr};
        bool pending_{false};
        std::vector<double> fromData_;
        std::vector<double> toData_;
        int fromPointCount_{0};
        int toPointCount_{0};
        QElapsedTimer timer_;
    };

    explicit DataTransition(QObject* parent = nullptr);
    ~DataTransition() override;

    int duration() const;
    void setDuration(int duration);

    QEasingCurve easing() const;
    void setEasing(const QEasingCurve& easing);

    bool enabled() const;
    void setEnabled(bool enabled);

    bool running() const;

    void start(Run& run, const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount);

    void cancel();

    bool advance(Run& run, std::vector<double>& outData, int& outPointCount);

signals:
    void durationChanged();
    void easingChanged();
    void enabledChanged();
    void runningChanged();

protected:
    virtual void interpolate(double easedProgress, const std::vector<double>& fromData, int fromPointCount, const std::vector<double>& toData, int toPointCount,
        std::vector<double>& outData, int& outPointCount)
        = 0;

private:
    void removeRun(Run* run);
    void setRunning(bool running);

    int duration_{300};
    QEasingCurve easing_{QEasingCurve(QEasingCurve::Linear)};
    bool enabled_{true};
    bool running_{false};
    QList<Run*> runs_;
};

} // namespace QAccelPlot
```


