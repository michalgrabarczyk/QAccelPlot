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

/// \brief Plain-data snapshot of dash rendering parameters.
///
/// Extracted from a \c DashLine on the main thread so the line renderer never
/// needs to touch the QObject hierarchy directly.
struct DashParameters {
    bool enabled{false}; ///< \brief \c true when dashed rendering is active.
    float period{0.0f};  ///< \brief Total dash+gap cycle length in pixels.
    float offset{0.0f};  ///< \brief Phase offset into the dash pattern in pixels.
    int patternSize{0};  ///< \brief Number of valid entries in \c pattern.
    float pattern[8]{};  ///< \brief Alternating dash/gap lengths (up to 8 entries).
};

/// \brief Abstract base class for all line styles.
///
/// Subclasses control whether a line is drawn (\c showLine()) and supply
/// dash parameters to the renderer via \c dashParameters().
///
/// \sa SolidLine, NoLine, DashLine
class LineStyle : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Whether the line should be rendered. Subclasses override to suppress (e.g. NoLine).
    Q_PROPERTY(bool showLine READ showLine CONSTANT)

public:
    /// \brief Constructs an LineStyle with the given \a parent.
    explicit LineStyle(QObject* parent = nullptr);

    /// \brief Returns \c true if the line should be rendered. Default: \c true.
    virtual bool showLine() const;
    /// \brief Returns the dash parameters for this style. Default: disabled dash.
    virtual DashParameters dashParameters() const;

signals:
    /// \brief Emitted when any style property changes, triggering a curve redraw.
    void styleChanged();
};

} // namespace QAccelPlot
