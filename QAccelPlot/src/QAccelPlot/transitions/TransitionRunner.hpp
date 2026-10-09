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

/// \brief Runs the \c DataTransition assigned to one item, such as a custom series.
///
/// Holds the item's \c DataTransition::Run and asks for a frame once per rendered frame while it
/// is active. The request comes on the GUI thread from \c QQuickWindow::afterAnimating, before the
/// scene graph syncs, so \c DataTransition::runningChanged reaches QML there and the frame renders
/// the new step. \c LineCurve and \c BarSeries animate through it.
///
/// \par Usage
/// - Forward the item's \c transition property to \c transition() and \c setTransition().
/// - When data arrives and \c enabled() is \c true, call \c start(); otherwise call \c cancel()
///   and show the data.
/// - On \c frameDue(), call \c advance() and show the frame.
/// - On \c interrupted(), call \c finish() and show the target data.
///
/// Use it on the item's thread only.
///
/// \sa DataTransition
class TransitionRunner : public QObject {
    Q_OBJECT

public:
    /// \brief Constructs the runner for \a item, which must outlive it.
    explicit TransitionRunner(QQuickItem* item);
    /// \brief Ends the run without emitting \c interrupted().
    ~TransitionRunner() override;

    /// \brief Returns the assigned transition, or \c nullptr.
    DataTransition* transition() const;
    /// \brief Assigns \a transition, interrupting a pending run first. The runner does not own it.
    /// \return \c false when \a transition is already assigned.
    bool setTransition(DataTransition* transition);
    /// \brief Returns \c true when a transition is assigned and enabled, so new data should start a run.
    bool enabled() const;

    /// \brief Returns \c true while the transition advances the run.
    bool active() const;
    /// \brief Returns \c true while the run holds target data that \c finish() has not taken.
    bool pending() const;
    /// \brief Returns the target data. Valid while \c pending() is \c true.
    const std::vector<double>& targetData() const;
    /// \brief Returns the number of points in \c targetData().
    int targetPointCount() const;

    /// \brief Starts the run on the assigned transition and requests a frame. Requires an assigned transition.
    /// \param currentData Current double buffer (copied as the \e from state).
    /// \param currentPointCount Number of points in \a currentData.
    /// \param newData Target double buffer (moved as the \e to state).
    /// \param newPointCount Number of points in \a newData.
    /// \param stride Number of values per point in both buffers. Default: 2, XY points.
    void start(const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount, int stride = 2);
    /// \brief Writes the frame of the run into \a outData and \a outPointCount, or the target data when the run ends.
    ///
    /// Call it from a \c frameDue() handler.
    /// \return \c true if the run is still active after this call.
    bool advance(std::vector<double>& outData, int& outPointCount);
    /// \brief Ends the run and moves its target data into \a outData and \a outPointCount.
    /// \return \c false, leaving the outputs unchanged, when the run is not pending.
    bool finish(std::vector<double>& outData, int& outPointCount);
    /// \brief Ends the run and discards its data.
    void cancel();

signals:
    /// \brief Emitted once per rendered frame while the run is active.
    void frameDue();
    /// \brief Emitted when the run stopped before its end: the transition was replaced, cancelled, or destroyed.
    ///
    /// The run still holds the target data. A run still pending after the signal is cancelled.
    void interrupted();
    /// \brief Emitted after the assigned transition was destroyed, when \c transition() is already \c nullptr.
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
