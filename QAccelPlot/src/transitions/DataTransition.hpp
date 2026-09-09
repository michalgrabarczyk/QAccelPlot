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
#include <QObject>
#include <QtQml/qqmlregistration.h>

#include <vector>

namespace QAccelPlot {

/// \brief Abstract base class for animated data transitions on plot elements.
///
/// Subclasses implement \c interpolate() to define how the element animates between
/// an old dataset and a new one. The transition is driven frame-by-frame by \c advance(),
/// which is called from the host element's \c updatePaintNode().
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
    /// \brief Read-only: \c true while the transition is playing.
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    /// \brief Constructs an DataTransition with the given \a parent.
    explicit DataTransition(QObject* parent = nullptr);

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

    /// \brief Returns \c true while a transition is actively playing.
    bool running() const;

    /// \brief Starts a new transition from \a currentData to \a newData.
    /// \param currentData Current XY float buffer (copied as the \e from state).
    /// \param currentPointCount Number of points in \a currentData.
    /// \param newData Target XY float buffer (moved as the \e to state).
    /// \param newPointCount Number of points in \a newData.
    void start(const std::vector<float>& currentData, int currentPointCount, std::vector<float>&& newData, int newPointCount);

    /// \brief Cancels the running transition immediately.
    void cancel();

    /// \brief Advances the animation by one frame, writing the interpolated data into \a outData.
    ///
    /// \return \c true if the animation is still running after this call.
    bool advance(std::vector<float>& outData, int& outPointCount);

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
    virtual void interpolate(float easedProgress, const std::vector<float>& fromData, int fromPointCount, const std::vector<float>& toData, int toPointCount,
        std::vector<float>& outData, int& outPointCount)
        = 0;

private:
    void setRunning(bool running);

    int duration_{300};
    QEasingCurve easing_{QEasingCurve(QEasingCurve::Linear)};
    bool enabled_{true};
    bool running_{false};

    std::vector<float> fromData_;
    std::vector<float> toData_;
    int fromPointCount_{0};
    int toPointCount_{0};
    QElapsedTimer animTimer_;
};

} // namespace QAccelPlot
