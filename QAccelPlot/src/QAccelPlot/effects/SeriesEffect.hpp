//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Abstract base class for visual effects applied to a series.
///
/// Effects are attached through the \c PlotSeries::effects list and evaluated when the series
/// renders. Each series supports some effects; assigning another logs a warning and the effect is
/// ignored. Of several effects of one type, the first enabled one applies.
///
/// | Effect | Series |
/// |---|---|
/// | \c GradientFill, \c GradientStroke | \c LineCurve |
/// | \c Glow | \c LineCurve, \c PointCloud, \c BarSeries, \c ErrorBarSeries |
/// | \c FadeTrail | \c LineCurve, \c PointCloud |
///
/// \sa GradientFill, GradientStroke, Glow, FadeTrail, PlotSeries
class SeriesEffect : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Whether this effect is active. Disabled effects are ignored during rendering.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    /// \brief Constructs a SeriesEffect with the given \a parent.
    explicit SeriesEffect(QObject* parent = nullptr);

    /// \brief Returns \c true if the effect is active.
    bool enabled() const;
    /// \brief Sets the effect's enabled state to \a enabled.
    void setEnabled(bool enabled);

    /// \brief Re-reads inputs that can change without a notification and emits \c effectChanged() if they did.
    ///
    /// The series calls this on the GUI thread once per frame. The default implementation does nothing.
    virtual void refresh();

signals:
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when any effect property changes, requesting a redraw of the series.
    void effectChanged();

private:
    bool enabled_{true};
};

} // namespace QAccelPlot
