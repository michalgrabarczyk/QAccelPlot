

# File UnitTickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**UnitTickLabelFormatter.hpp**](UnitTickLabelFormatter_8hpp.md)

[Go to the documentation of this file](UnitTickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/formatters/TickLabelFormatter.hpp"
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <QJSValue>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <optional>

namespace QAccelPlot {

class UnitTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(UnitTickLabelFormatter)

    
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
    Q_PROPERTY(PrefixMode prefixMode READ prefixMode WRITE setPrefixMode NOTIFY prefixModeChanged)
    Q_PROPERTY(QString fixedPrefix READ fixedPrefix WRITE setFixedPrefix NOTIFY fixedPrefixChanged)
    Q_PROPERTY(QJSValue customPrefix READ customPrefix WRITE setCustomPrefix NOTIFY customPrefixChanged)
    Q_PROPERTY(QStringList allowedPrefixes READ allowedPrefixes WRITE setAllowedPrefixes RESET resetAllowedPrefixes NOTIFY allowedPrefixesChanged)
    Q_PROPERTY(UnitPosition unitPosition READ unitPosition WRITE setUnitPosition NOTIFY unitPositionChanged)

public:
    enum PrefixMode {
        LargestMagnitude,
        Fixed,
        Custom,
        PerTick,
    };
    Q_ENUM(PrefixMode)

    
    enum UnitPosition {
        AxisLabel,
        TickLabels,
    };
    Q_ENUM(UnitPosition)

    
    explicit UnitTickLabelFormatter(QObject* parent = nullptr);

    QString unit() const;
    void setUnit(const QString& unit);

    PrefixMode prefixMode() const;
    void setPrefixMode(PrefixMode mode);

    QString fixedPrefix() const;
    void setFixedPrefix(const QString& symbol);

    QJSValue customPrefix() const;
    void setCustomPrefix(const QJSValue& callback);

    QStringList allowedPrefixes() const;
    void setAllowedPrefixes(const QStringList& symbols);
    void resetAllowedPrefixes();

    UnitPosition unitPosition() const;
    void setUnitPosition(UnitPosition position);

signals:
    void unitChanged();
    void prefixModeChanged();
    void fixedPrefixChanged();
    void customPrefixChanged();
    void allowedPrefixesChanged();
    void unitPositionChanged();

protected:
    QString doFormat(qreal value, qreal tickStep) const override;
    QString doFormatLogTick(qreal value) const override;
    TickSetLabels doFormatTicks(const TickSet& tickSet) const override;
    QString doFormatValue(qreal value, qreal resolution, const TickSetFormat& tickSetFormat) const override;

private:
    void applyAllowedPrefixes(const QList<Internal::UnitPrefix>& prefixes);
    Internal::UnitPrefix sharedPrefix(const TickSet& tickSet) const;
    Internal::UnitPrefix ownPrefix(qreal value, qreal precision) const;
    TickSetLabels perTickLabels(const TickSet& tickSet) const;
    std::optional<Internal::UnitPrefix> callCustomPrefix(const TickSet& tickSet) const;

    QString unit_;
    PrefixMode prefixMode_{LargestMagnitude};
    Internal::UnitPrefix fixedPrefix_;
    QJSValue customPrefix_;
    QList<Internal::UnitPrefix> allowedPrefixes_;
    UnitPosition unitPosition_{AxisLabel};
};

} // namespace QAccelPlot
```


