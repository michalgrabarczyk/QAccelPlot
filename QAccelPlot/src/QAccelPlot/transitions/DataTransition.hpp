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

/// \brief Abstract base class for animated data transitions on plot elements.
///
/// Subclasses implement \c interpolate() to define how the element animates between
/// an old dataset and a new one. The transition is driven frame-by-frame by \c advance(),
/// which the host element calls on the GUI thread before each scene graph synchronization,
/// so \c runningChanged is always emitted on the GUI thread.
///
/// One transition can be assigned to several elements. Each element keeps its own \c Run with the
/// data it animates between, so the elements animate independently.
///
/// \sa DrawTransition, MorphTransition, LineCurve
class DataTransition : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Duration of the transition in milliseconds. Default: 300.
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    /// \brief Easing curve applied to the animation progress. Default: \c QEasingCurve::Linear.
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)
    /// \brief Whether the transition is active. When \c false, data updates are applied instantly.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// \brief Read-only: \c true while the transition is playing on at least one element.
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    /// \brief One animation of a transition on one host element.
    ///
    /// Holds the data the host animates from and to. A run is \e active while the transition
    /// advances it, and \e pending until the host takes the target data with \c finish(). A run
    /// ended by \c DataTransition::cancel() or by destroying the transition stays pending, so the
    /// host can still show the target data. Destroying a run ends it.
    class Run {
    public:
        /// \brief Constructs an idle run.
        Run() = default;
        /// \brief Destroys the run, ending it on its transition.
        ~Run();

        Run(const Run&) = delete;
        Run& operator=(const Run&) = delete;

        /// \brief Returns \c true while the transition advances this run.
        bool active() const;
        /// \brief Returns \c true while the run holds target data that \c finish() has not taken.
        bool pending() const;
        /// \brief Returns the target data. Valid while \c pending() is \c true.
        const std::vector<double>& targetData() const;
        /// \brief Returns the number of points in \c targetData().
        int targetPointCount() const;

        /// \brief Ends the run and moves its target data into \a outData and \a outPointCount.
        ///
        /// \return \c false, leaving the outputs unchanged, when the run is not pending.
        bool finish(std::vector<double>& outData, int& outPointCount);
        /// \brief Ends the run and discards its data.
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

    /// \brief Constructs an DataTransition with the given \a parent.
    explicit DataTransition(QObject* parent = nullptr);
    /// \brief Destroys the transition. Its runs stay pending, so their hosts can still show the target data.
    ~DataTransition() override;

    /// \brief Returns the animation duration in milliseconds.
    int duration() const;
    /// \brief Sets the animation duration to \a duration milliseconds.
    void setDuration(int duration);

    /// \brief Returns the easing curve.
    QEasingCurve easing() const;
    /// \brief Sets the easing curve to \a easing.
    void setEasing(const QEasingCurve& easing);

    /// \brief Returns \c true if the transition is enabled.
    bool enabled() const;
    /// \brief Sets the enabled state to \a enabled.
    void setEnabled(bool enabled);

    /// \brief Returns \c true while at least one run is active.
    bool running() const;

    /// \brief Starts \a run from \a currentData to \a newData, restarting it if it is already active.
    /// \param run Run of the host element; ended first if another transition advances it.
    /// \param currentData Current XY double buffer (copied as the \e from state).
    /// \param currentPointCount Number of points in \a currentData.
    /// \param newData Target XY double buffer (moved as the \e to state).
    /// \param newPointCount Number of points in \a newData.
    void start(Run& run, const std::vector<double>& currentData, int currentPointCount, std::vector<double>&& newData, int newPointCount);

    /// \brief Ends every active run immediately.
    ///
    /// The runs stay pending; hosts show the target data after \c running turns \c false.
    void cancel();

    /// \brief Advances \a run by one frame, writing the interpolated data into \a outData.
    ///
    /// When the duration has elapsed, writes the target data and finishes the run.
    /// \return \c true if \a run is still active after this call.
    bool advance(Run& run, std::vector<double>& outData, int& outPointCount);

signals:
    /// \brief Emitted when the duration property changes.
    void durationChanged();
    /// \brief Emitted when the easing property changes.
    void easingChanged();
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when the running property changes.
    void runningChanged();

protected:
    /// \brief Subclass entry point — computes the interpolated dataset at \a easedProgress (0–1).
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
