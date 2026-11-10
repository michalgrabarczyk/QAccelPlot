

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
#include "QAccelPlot/transitions/EnterExitTransition.hpp"
#include "QAccelPlot/transitions/internal/TransitionFrame.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>

#include <array>
#include <optional>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class TransitionRunner : public QObject {
    Q_OBJECT

public:
    enum class Slot {
        Enter,  
        Update, 
        Exit,   
    };
    Q_ENUM(Slot)

    
    using Layout = Internal::TransitionLayout;

    explicit TransitionRunner(QQuickItem* item);
    ~TransitionRunner() override;

    DataTransition* transition(Slot slot) const;
    bool setTransition(Slot slot, DataTransition* transition);

    template <typename Item> void notifyTransitionChanges(Item* item)
    {
        connect(this, &TransitionRunner::transitionChanged, item, [item](const Slot slot) {
            switch (slot) {
            case Slot::Enter:
                emit item->enterTransitionChanged();
                break;
            case Slot::Update:
                emit item->updateTransitionChanged();
                break;
            case Slot::Exit:
                emit item->exitTransitionChanged();
                break;
            }
        });
    }

    bool enabled() const;
    bool animates(int currentPointCount, int newPointCount) const;

    bool active() const;
    bool pending() const;
    const std::vector<double>& targetData() const;
    int targetPointCount() const;
    int frameCapacity() const;

    bool start(const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount, const Layout& layout);
    bool advance(std::vector<double>& outData, int& outPointCount);
    const Internal::ItemFade& fade() const;
    bool finish(std::vector<double>& outData, int& outPointCount);
    void cancel();

signals:
    void frameDue();
    void interrupted();
    void transitionChanged(TransitionRunner::Slot slot);
    void activeChanged();

private:
    static constexpr auto kSlotCount = std::size_t{3};

    std::optional<Internal::TransitionKind> enabledKind(Slot slot) const;
    std::optional<Internal::TransitionStep> step(Slot slot) const;
    bool finished() const;
    void endRuns();
    void restartRuns(const std::array<bool, kSlotCount>& used);
    void interrupt();
    void onTransitionDestroyed(Slot slot);
    void onRunningChanged();
    void updateActive();
    void connectWindow(QQuickWindow* window);
    void onAfterAnimating();
    void requestFrame();

    std::array<QPointer<DataTransition>, kSlotCount> transitions_;
    std::array<QMetaObject::Connection, kSlotCount> destroyedConnections_;
    std::array<QMetaObject::Connection, kSlotCount> runningConnections_;
    std::array<DataTransition::Run, kSlotCount> runs_;
    std::array<bool, kSlotCount> used_{};
    bool pending_{false};
    bool active_{false};
    std::vector<double> from_;
    int fromCount_{0};
    std::vector<double> to_;
    int toCount_{0};
    int shownCount_{0};
    Layout layout_;
    Internal::ItemFade fade_;
    QPointer<QQuickWindow> window_;
};

} // namespace QAccelPlot
```


