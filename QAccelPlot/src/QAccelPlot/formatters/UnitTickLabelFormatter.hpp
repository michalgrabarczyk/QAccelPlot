//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QString>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Numeric tick label formatter that writes the tick labels in a unit with an automatic SI unit prefix.
///
/// The data stays in the base \c unit. On a linear axis all tick labels share one unit prefix chosen from the
/// visible major ticks, and the axis label shows the prefixed unit: \c unit "m" over 0–0.004 gives the tick
/// labels 0, 1, 2, 3, 4 and the axis label "Distance (mm)". Decimals follow the tick step in the prefixed unit,
/// as with \c NumericTickLabelFormatter. The prefix depends only on the visible range, so each axis sharing one
/// formatter picks its own.
///
/// A \c tickLabel callback receives the value and the tick step in the prefixed unit.
///
/// \sa NumericTickLabelFormatter, TickLabelFormatter
class UnitTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(UnitTickLabelFormatter)

    /// \brief Base unit symbol of the axis values, such as "m". Default: empty.
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
    /// \brief Rule that picks the unit prefix from the visible range. Default: \c LargestMagnitude.
    Q_PROPERTY(PrefixMode prefixMode READ prefixMode WRITE setPrefixMode NOTIFY prefixModeChanged)

public:
    /// \brief Rule that picks the unit prefix.
    enum PrefixMode {
        /// \brief The largest power-of-1000 prefix, from quecto to quetta, at which the largest visible major tick
        /// is still at least 1, shared by all tick labels.
        LargestMagnitude,
    };
    Q_ENUM(PrefixMode)

    /// \brief Constructs a UnitTickLabelFormatter with the given \a parent.
    explicit UnitTickLabelFormatter(QObject* parent = nullptr);

    /// \brief Returns the base unit symbol.
    QString unit() const;
    /// \brief Sets the base unit symbol to \a unit.
    void setUnit(const QString& unit);

    /// \brief Returns the rule that picks the unit prefix.
    PrefixMode prefixMode() const;
    /// \brief Sets the rule that picks the unit prefix to \a mode.
    void setPrefixMode(PrefixMode mode);

signals:
    /// \brief Emitted when the unit property changes.
    void unitChanged();
    /// \brief Emitted when the prefixMode property changes.
    void prefixModeChanged();

protected:
    /// \brief Returns a numeric string for \a value, precision matched to \a tickStep.
    QString doFormat(qreal value, qreal tickStep) const override;
    /// \brief Returns a decade tick as a superscript power of ten, for example "10²" for 100.
    QString doFormatLogTick(qreal value) const override;
    /// \brief Returns the tick labels in the shared unit prefix of \a tickSet, and the prefixed unit as unit text.
    TickSetLabels doFormatTicks(const TickSet& tickSet) const override;
    /// \brief Returns \a value in the unit prefix of \a tickSetFormat, followed by a space and the prefixed unit,
    /// such as "12.3 mm".
    QString doFormatValue(qreal value, qreal resolution, const TickSetFormat& tickSetFormat) const override;

private:
    QString unit_;
    PrefixMode prefixMode_{LargestMagnitude};
};

} // namespace QAccelPlot
