//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QList>
#include <QString>

#include <optional>

namespace QAccelPlot::Internal {

/// \brief An SI unit prefix: its output symbol and its power of ten.
struct UnitPrefix {
    QString symbol;  ///< \brief Output symbol, such as "k" or "µ" (U+00B5); empty for the bare unit.
    int exponent{0}; ///< \brief Power of ten the prefix stands for, such as 3 for "k".

    /// \brief Returns whether both prefixes have the same symbol and power of ten.
    friend bool operator==(const UnitPrefix& left, const UnitPrefix& right)
    {
        return left.exponent == right.exponent && left.symbol == right.symbol;
    }

    /// \brief Returns whether the prefixes differ in symbol or power of ten.
    friend bool operator!=(const UnitPrefix& left, const UnitPrefix& right)
    {
        return !(left == right);
    }
};

/// \brief Returns every SI prefix from quecto to quetta, including the bare unit, by ascending exponent.
const QList<UnitPrefix>& siUnitPrefixes();

/// \brief Returns every power-of-1000 prefix from quecto to quetta, including the bare unit, by ascending exponent.
const QList<UnitPrefix>& defaultUnitPrefixes();

/// \brief Returns the SI prefix written as \a symbol, or \c std::nullopt when \a symbol is not an SI prefix.
///
/// An empty \a symbol is the bare unit. "µ" (U+00B5), "μ" (U+03BC), and "u" all give micro with the symbol "µ"
/// (U+00B5).
std::optional<UnitPrefix> unitPrefixFromSymbol(const QString& symbol);

/// \brief Returns \a prefixes sorted by ascending exponent, without duplicates.
QList<UnitPrefix> sortedUnitPrefixes(QList<UnitPrefix> prefixes);

/// \brief Returns the prefix of \a prefixes with the largest exponent at which \a magnitude is still at least 1.
///
/// \a prefixes must be sorted by ascending exponent. Falls back to the smallest prefix when none qualifies, and to
/// the bare unit when \a prefixes is empty. A zero or non-finite \a magnitude is treated as 1.
UnitPrefix largestMagnitudePrefix(qreal magnitude, const QList<UnitPrefix>& prefixes);

/// \brief Returns the prefix of \a prefixes whose power of ten is \a exponent, or the bare unit when none matches.
UnitPrefix prefixWithExponent(int exponent, const QList<UnitPrefix>& prefixes);

/// \brief Returns the prefix of \a prefixes with the largest exponent not above \a exponent.
///
/// \a prefixes must be sorted by ascending exponent. Falls back to the smallest prefix when none qualifies, and to
/// the bare unit when \a prefixes is empty.
UnitPrefix snappedUnitPrefix(int exponent, const QList<UnitPrefix>& prefixes);

/// \brief Returns \a value, given in the base unit, expressed in \a prefix.
qreal toPrefix(qreal value, const UnitPrefix& prefix);

} // namespace QAccelPlot::Internal
