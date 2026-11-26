

# File UnitPrefix.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**internal**](dir_5d21a914f9fc6a0914b7bb5bdcf315a8.md) **>** [**UnitPrefix.hpp**](UnitPrefix_8hpp.md)

[Go to the documentation of this file](UnitPrefix_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QList>
#include <QString>

#include <optional>

namespace QAccelPlot::Internal {

struct UnitPrefix {
    QString symbol;  
    int exponent{0}; 

    friend bool operator==(const UnitPrefix& left, const UnitPrefix& right)
    {
        return left.exponent == right.exponent && left.symbol == right.symbol;
    }

    friend bool operator!=(const UnitPrefix& left, const UnitPrefix& right)
    {
        return !(left == right);
    }
};

const QList<UnitPrefix>& siUnitPrefixes();

const QList<UnitPrefix>& defaultUnitPrefixes();

std::optional<UnitPrefix> unitPrefixFromSymbol(const QString& symbol);

QList<UnitPrefix> sortedUnitPrefixes(QList<UnitPrefix> prefixes);

UnitPrefix largestMagnitudePrefix(qreal magnitude, const QList<UnitPrefix>& prefixes);

UnitPrefix prefixWithExponent(int exponent, const QList<UnitPrefix>& prefixes);

UnitPrefix snappedUnitPrefix(int exponent, const QList<UnitPrefix>& prefixes);

qreal toPrefix(qreal value, const UnitPrefix& prefix);

} // namespace QAccelPlot::Internal
```


