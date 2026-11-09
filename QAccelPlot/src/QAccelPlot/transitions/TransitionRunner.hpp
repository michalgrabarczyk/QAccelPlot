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

/// \brief Runs the enter, update, and exit transitions of one item, such as a custom series.
///
/// A data change splits the items by index: with \a m items before and \a n after, items below
/// both counts are kept, items from \a m on are added, and items from \a n on are removed. Each
/// group runs the transition of its slot; a group without one changes instantly. Kept items run
/// the update transition only when one of them changes. A \c DrawTransition or
/// \c GrowTransition in the update slot then redraws all new items instead.
///
/// The runner asks for a frame once per rendered frame while it is active. The request comes on
/// the GUI thread from \c QQuickWindow::afterAnimating, before the scene graph syncs, so
/// \c DataTransition::runningChanged reaches QML there and the frame renders the new step.
/// \c LineCurve, \c BarSeries, \c PointCloud, and \c ErrorBarSeries animate through it.
///
/// \par Usage
/// - Forward the item's transition properties to the slot getters and setters.
/// - When data arrives and \c animates() is \c true, call \c start(). When it returns \c false,
///   or when \c animates() is \c false, call \c cancel() and show the data.
/// - After \c start() and on \c frameDue(), call \c advance() and show the frame.
/// - On \c interrupted(), call \c finish() and show the target data.
///
/// Use it on the item's thread only.
///
/// \sa DataTransition
class TransitionRunner : public QObject {
    Q_OBJECT

public:
    /// \brief A transition slot of the item.
    enum class Slot {
        Enter,  ///< \brief Animates added items.
        Update, ///< \brief Animates kept items.
        Exit,   ///< \brief Animates removed items.
    };
    Q_ENUM(Slot)

    /// \brief How the items of the item animate.
    using Layout = Internal::TransitionLayout;

    /// \brief Constructs the runner for \a item, which must outlive it.
    explicit TransitionRunner(QQuickItem* item);
    /// \brief Ends the run without emitting \c interrupted().
    ~TransitionRunner() override;

    /// \brief Returns the transition of \a slot, or \c nullptr.
    DataTransition* transition(Slot slot) const;
    /// \brief Assigns \a transition to \a slot, interrupting a pending run first. The runner does not own it.
    ///
    /// The enter and exit slots take only an \c EnterExitTransition.
    /// \return \c false when \a transition is already assigned to \a slot, or does not fit it.
    bool setTransition(Slot slot, DataTransition* transition);
    /// \brief Returns \c true when some slot has an enabled transition.
    bool enabled() const;
    /// \brief Returns \c true when a change from \a currentPointCount drawn items to \a newPointCount items may start a run.
    ///
    /// Without a pending run, \c false means that \c start() would return \c false, so the item can
    /// show the data without copying it for a run.
    bool animates(int currentPointCount, int newPointCount) const;

    /// \brief Returns \c true while a transition advances the run.
    bool active() const;
    /// \brief Returns \c true while the run holds target data that \c finish() has not taken.
    bool pending() const;
    /// \brief Returns the target data. Valid while \c pending() is \c true.
    const std::vector<double>& targetData() const;
    /// \brief Returns the number of items in \c targetData().
    int targetPointCount() const;
    /// \brief Returns the most items a frame of the run holds: the larger of the start and target counts.
    int frameCapacity() const;

    /// \brief Starts a run from \a currentData to \a newData and requests a frame.
    /// \param currentData Items drawn now, copied as the \e from side. Items of a previous run that
    ///                    its last frame has not shown yet, or shows leaving, are left out.
    /// \param currentPointCount Number of items in \a currentData.
    /// \param newData Target items, moved as the \e to side.
    /// \param newPointCount Number of items in \a newData.
    /// \param layout How the items animate; its stride applies to both buffers.
    /// \return \c false, ending any run, when no slot animates this change.
    bool start(const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount, const Layout& layout);
    /// \brief Writes the frame of the run into \a outData and \a outPointCount, or the target data when the run ends.
    /// \return \c true if the run is still active after this call.
    bool advance(std::vector<double>& outData, int& outPointCount);
    /// \cond INTERNAL
    /// \brief Returns the items the last frame fades.
    const Internal::ItemFade& fade() const;
    /// \endcond
    /// \brief Ends the run and moves its target data into \a outData and \a outPointCount.
    /// \return \c false, leaving the outputs unchanged, when the run is not pending.
    bool finish(std::vector<double>& outData, int& outPointCount);
    /// \brief Ends the run and discards its data.
    void cancel();

signals:
    /// \brief Emitted once per rendered frame while the run is active.
    void frameDue();
    /// \brief Emitted when the run stopped before its end: a transition was replaced, cancelled, or destroyed.
    ///
    /// The run still holds the target data. A run still pending after the signal is cancelled.
    void interrupted();
    /// \brief Emitted after the transition of \a slot was destroyed, when \c transition() is already \c nullptr.
    void transitionDestroyed(TransitionRunner::Slot slot);
    /// \brief Emitted when \c active() changes.
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
