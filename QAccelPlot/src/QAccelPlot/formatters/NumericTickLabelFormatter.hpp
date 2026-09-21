//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief The default tick label formatter — produces numeric labels with automatic decimal precision.
///
/// The number of decimal places is derived from \a tickStep so that labels are neither
/// truncated nor cluttered with unnecessary digits. Every label on a linear axis shares one
/// tick step, so all of them show the same number of decimal places (for example
/// 0.0, 0.5, 1.0, 1.5). Decade ticks on a logarithmic axis use superscript powers of ten
/// (10⁻¹, 10⁰, 10¹, 10²), like LogTickLabelFormatter.
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
    /// \brief Returns a decade tick as a superscript power of ten, for example "10²" for 100.
    QString doFormatLogTick(qreal value) const override;
};

} // namespace QAccelPlot
