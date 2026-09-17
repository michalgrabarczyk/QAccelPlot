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

/// \brief Abstract base class for visual effects applied to a \c LineCurve.
///
/// Effects are attached via the \c LineCurve::effects list property and evaluated
/// during \c updatePaintNode to modify line or fill rendering.
///
/// \sa GradientFill, GradientStroke, LineCurve
class LineCurveEffect : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Whether this effect is active. Disabled effects are ignored during rendering.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    /// \brief Constructs an LineCurveEffect with the given \a parent.
    explicit LineCurveEffect(QObject* parent = nullptr);

    /// \brief Returns \c true if the effect is active.
    bool enabled() const;
    /// \brief Sets the effect's enabled state to \a enabled.
    void setEnabled(bool enabled);

signals:
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when any effect property changes, requesting a curve redraw.
    void effectChanged();

private:
    bool enabled_{true};
};

} // namespace QAccelPlot
