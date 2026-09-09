//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "formatters/TickLabelFormatter.hpp"

#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief A tick label formatter that produces power-of-ten labels for logarithmic axes.
///
/// Formats each tick value as "10^n".
///
/// \sa NumericTickLabelFormatter, TickLabelFormatter
class LogTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(LogTickLabelFormatter)

public:
    /// \brief Constructs a LogTickLabelFormatter with the given \a parent.
    explicit LogTickLabelFormatter(QObject* parent = nullptr);

protected:
    /// \brief Returns a log-scale label string for \a value.
    QString doFormat(qreal value, qreal tickStep) const override;
};

} // namespace QAccelPlot
