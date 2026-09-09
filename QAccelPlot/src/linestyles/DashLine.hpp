//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "linestyles/LineStyle.hpp"

#include <QList>

namespace QAccelPlot {

/// \brief A line style that renders the curve as a customisable dashed line.
///
/// The \c pattern property accepts a list of alternating dash and gap lengths in pixels,
/// following the same convention as SVG \c stroke-dasharray.
///
/// \sa SolidLine, NoLine, LineStyle
class DashLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(DashLine)

    /// \brief Alternating dash/gap lengths in pixels, e.g. \c [10, 5] for a 10px dash with 5px gap.
    Q_PROPERTY(QList<qreal> pattern READ pattern WRITE setPattern NOTIFY patternChanged)

public:
    /// \brief Constructs a DashLine with the given \a parent.
    explicit DashLine(QObject* parent = nullptr);

    /// \brief Returns the current dash pattern.
    QList<qreal> pattern() const;
    /// \brief Sets the dash pattern to \a pattern.
    void setPattern(const QList<qreal>& pattern);

    /// \brief Returns a render-thread-safe snapshot of the dash parameters.
    DashParameters dashParameters() const override;

signals:
    /// \brief Emitted when the pattern property changes.
    void patternChanged();

private:
    QList<qreal> pattern_;
};

} // namespace QAccelPlot
