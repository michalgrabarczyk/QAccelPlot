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

/// \brief Namespace exposing the \c NanGapMode enum to QML.
namespace NanGapModeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(NanGapMode)

/// \brief Rendering strategies for invalid samples: NaN, ±Inf, or a non-positive value on a log-scale axis.
enum class Mode {
    Break,   ///< \brief No line segment or fill is drawn to or from an invalid sample.
    Connect, ///< \brief Invalid samples are skipped; the line and fill join the neighboring valid samples.
};
Q_ENUM_NS(Mode)
} // namespace NanGapModeNS

using NanGapMode = NanGapModeNS::Mode;

/// \brief Controls how a \c LineCurve renders gaps in its data.
///
/// Accessible via the \c LineCurve::gaps CONSTANT grouped property, for example
/// <tt>gaps.nanMode: QAccelPlot.NanGapMode.Connect</tt>.
///
/// A sample is invalid when its X or Y coordinate is NaN or ±Inf, or is not strictly positive on a
/// log-scale axis. Invalid samples are never drawn as markers, never hit-tested, and are excluded from
/// auto-ranging coordinate by coordinate, regardless of \c nanMode.
///
/// \sa LineCurve
class LineCurveGaps : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief How the line and gradient fill treat invalid samples. Default: \c NanGapMode.Break.
    Q_PROPERTY(NanGapMode nanMode READ nanMode WRITE setNanMode NOTIFY nanModeChanged)

public:
    /// \brief Constructs a LineCurveGaps with the given \a parent.
    explicit LineCurveGaps(QObject* parent = nullptr);

    /// \brief Returns how invalid samples are rendered.
    NanGapMode nanMode() const;
    /// \brief Sets how invalid samples are rendered to \a mode.
    void setNanMode(NanGapMode mode);

signals:
    /// \brief Emitted when the nanMode property changes.
    void nanModeChanged();

private:
    NanGapMode nanMode_{NanGapMode::Break};
};

} // namespace QAccelPlot
