

# File TransitionRunner.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**transitions**](dir_33e4f9f956353311d613e58793c6ec62.md) **>** [**TransitionRunner.hpp**](TransitionRunner_8hpp.md)

[Go to the documentation of this file](TransitionRunner_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QObject>
#include <QPointer>

#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class TransitionRunner : public QObject {
    Q_OBJECT

public:
    explicit TransitionRunner(QQuickItem* item);
    ~TransitionRunner() override;

    DataTransition* transition() const;
    bool setTransition(DataTransition* transition);
    bool enabled() const;

    bool active() const;
    bool pending() const;
    const std::vector<double>& targetData() const;
    int targetPointCount() const;

    void start(const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount, int stride = 2);
    bool advance(std::vector<double>& outData, int& outPointCount);
    bool finish(std::vector<double>& outData, int& outPointCount);
    void cancel();

signals:
    void frameDue();
    void interrupted();
    void transitionDestroyed();

private:
    void interrupt();
    void onTransitionDestroyed();
    void onRunningChanged();
    void connectWindow(QQuickWindow* window);
    void onAfterAnimating();
    void requestFrame();

    QPointer<DataTransition> transition_;
    DataTransition::Run run_;
    QPointer<QQuickWindow> window_;
};

} // namespace QAccelPlot
```


