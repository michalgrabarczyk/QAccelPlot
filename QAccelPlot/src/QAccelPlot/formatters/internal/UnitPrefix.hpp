//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QList>
#include <QString>

namespace QAccelPlot::Internal {

/// \brief An SI unit prefix: its output symbol and its power of ten.
struct UnitPrefix {
    QString symbol;  ///< \brief Output symbol, such as "k" or "µ" (U+00B5); empty for the bare unit.
    int exponent{0}; ///< \brief Power of ten the prefix stands for, such as 3 for "k".
};

/// \brief Returns every power-of-1000 prefix from quecto to quetta, including the bare unit, by ascending exponent.
const QList<UnitPrefix>& defaultUnitPrefixes();

/// \brief Returns the prefix of \a prefixes with the largest exponent at which \a magnitude is still at least 1.
///
/// \a prefixes must be sorted by ascending exponent. Falls back to the smallest prefix when none qualifies, and to
/// the bare unit when \a prefixes is empty. A zero or non-finite \a magnitude is treated as 1.
UnitPrefix largestMagnitudePrefix(qreal magnitude, const QList<UnitPrefix>& prefixes);

/// \brief Returns the prefix of \a prefixes whose power of ten is \a exponent, or the bare unit when none matches.
UnitPrefix prefixWithExponent(int exponent, const QList<UnitPrefix>& prefixes);

/// \brief Returns \a value, given in the base unit, expressed in \a prefix.
qreal toPrefix(qreal value, const UnitPrefix& prefix);

} // namespace QAccelPlot::Internal
