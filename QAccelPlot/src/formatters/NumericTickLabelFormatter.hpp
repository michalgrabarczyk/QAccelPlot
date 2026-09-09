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

/// \brief The default tick label formatter — produces numeric labels with automatic decimal precision.
///
/// The number of decimal places is derived from \a tickStep so that labels are neither
/// truncated nor cluttered with unnecessary digits.
///
/// \sa DateTimeTickLabelFormatter, LogTickLabelFormatter, TickLabelFormatter
class NumericTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(NumericTickLabelFormatter)

public:
    /// \brief Constructs a NumericTickLabelFormatter with the given \a parent.
    explicit NumericTickLabelFormatter(QObject* parent = nullptr);

protected:
    /// \brief Returns a numeric string for \a value, precision matched to \a tickStep.
    QString doFormat(qreal value, qreal tickStep) const override;
};

} // namespace QAccelPlot
