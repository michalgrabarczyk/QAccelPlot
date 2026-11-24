//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QString>

namespace QAccelPlot::Internal {

/// \brief Returns \a value with the decimal places needed to write \a tickStep exactly.
///
/// Steps 1, 2, 5, and 10 get no decimals, 0.5 and 0.1 get one, 0.25 and 0.05 get two. A non-finite or
/// non-positive step gets one. A value that rounds to zero is written without a minus sign.
QString numericLabel(qreal value, qreal tickStep);

/// \brief Returns a decade \a value as a superscript power of ten, such as "10²" or "10⁻³".
///
/// A value that is not a positive power of ten falls back to \c numericLabel(value, value).
QString logTickLabel(qreal value);

} // namespace QAccelPlot::Internal
