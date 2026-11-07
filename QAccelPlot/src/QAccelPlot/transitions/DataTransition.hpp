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
#include <QtQml/qqmlregistration.h>

QT_FORWARD_DECLARE_CLASS(QAbstractAnimation)

namespace QAccelPlot {

namespace Internal {
/// \brief How a transition moves its items.
enum class TransitionKind {
    Morph, ///< \brief Interpolates each item from its old values to its new ones.
    Draw,  ///< \brief Reveals items one after another.
    Grow,  ///< \brief Grows item values from a baseline.
};
} // namespace Internal

/// \brief Abstract base class for animated data transitions on plot series.
///
/// A series animates a data change with up to three transitions, matched to its items by index:
/// \c enterTransition for added items, \c updateTransition for kept items, and \c exitTransition
/// for removed items. \c MorphTransition, \c DrawTransition, and \c GrowTransition are the
/// available transitions.
///
/// Time comes from Qt's animation timer on the transition's thread, the clock of QML animations.
/// An installed \c QAnimationDriver therefore steps transitions too, for example to record
/// frames offscreen at a fixed time step.
///
/// One transition can be assigned to several series, and to several slots of one series. Each
/// assignment keeps its own \c Run, so the animations are independent.
///
/// \sa MorphTransition, DrawTransition, GrowTransition, TransitionRunner
class DataTransition : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Duration of the transition in milliseconds. Default: 300.
    ///
    /// With 0 or less, a run shows the target data at its next frame.
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    /// \brief Easing curve applied to the progress of each item. Default: \c QEasingCurve::Linear.
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)
    /// \brief Whether the transition is active. When \c false, the items it animates change instantly.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// \brief Read-only: \c true while the transition is playing on at least one series.
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    /// \brief Share of \c duration over which the start times of the items are spread, in [0, 1].
    ///
    /// 0 animates all items together; 1 animates them one after another, each changing at once.
    /// Items start in index order. Values outside [0, 1] are clamped. Default: 0, and 1 for \c DrawTransition.
    Q_PROPERTY(qreal stagger READ stagger WRITE setStagger NOTIFY staggerChanged)

public:
    /// \brief One animation of a transition on one series.
    ///
    /// A run is \e active from \c DataTransition::start() until \c end(), \c DataTransition::cancel(),
    /// or the destruction of the transition. Destroying a run ends it.
    class Run {
    public:
        /// \brief Constructs an idle run.
        Run() = default;
        /// \brief Destroys the run, ending it on its transition.
        ~Run();

        Run(const Run&) = delete;
        Run& operator=(const Run&) = delete;

        /// \brief Returns \c true while the run is active.
        bool active() const;
        /// \brief Returns the transition the run is active on, or \c nullptr.
        DataTransition* transition() const;
        /// \brief Ends the run.
        void end();

    private:
        friend class DataTransition;

        DataTransition* transition_{nullptr};
        int startTime_{0};
    };

    /// \brief Destroys the transition, ending its runs.
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

    /// \brief Returns the share of the duration over which item start times are spread.
    qreal stagger() const;
    /// \brief Sets the stagger to \a stagger, clamped to [0, 1].
    void setStagger(qreal stagger);

    /// \brief Starts \a run, restarting it if it is already active.
    ///
    /// A run active on another transition is ended first.
    void start(Run& run);
    /// \brief Ends every active run immediately.
    void cancel();
    /// \brief Returns the progress of \a run in [0, 1], before easing.
    ///
    /// It is 1 once the duration has elapsed, and 1 for a run that is not active on this transition.
    double progress(const Run& run) const;

    /// \cond INTERNAL
    /// \brief Returns how the transition moves its items.
    virtual Internal::TransitionKind kind() const = 0;
    /// \endcond

signals:
    /// \brief Emitted when the duration property changes.
    void durationChanged();
    /// \brief Emitted when the easing property changes.
    void easingChanged();
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when the running property changes.
    void runningChanged();
    /// \brief Emitted when the stagger property changes.
    void staggerChanged();

private:
    // Only the transitions of the library derive from DataTransition.
    friend class MorphTransition;
    friend class EnterExitTransition;

    explicit DataTransition(QObject* parent = nullptr);

    void removeRun(Run* run);
    void setRunning(bool running);

    int duration_{300};
    QEasingCurve easing_{QEasingCurve(QEasingCurve::Linear)};
    bool enabled_{true};
    bool running_{false};
    qreal stagger_{0.0};
    QList<Run*> runs_;
    QAbstractAnimation* clock_;
};

} // namespace QAccelPlot
