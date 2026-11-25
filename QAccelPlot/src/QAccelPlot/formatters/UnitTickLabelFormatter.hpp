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

/// \brief Numeric tick label formatter that writes the tick labels in a unit with an automatic SI unit prefix.
///
/// The data stays in the base \c unit. All tick labels share one unit prefix chosen from the visible major ticks,
/// and the axis label shows the prefixed unit: \c unit "m" over 0–0.004 gives the tick labels 0, 1, 2, 3, 4 and
/// the axis label "Distance (mm)". Decimals follow the tick step in the prefixed unit, as with
/// \c NumericTickLabelFormatter. On a logarithmic axis the tick labels stay powers of ten in the prefixed unit:
/// 0.000001–0.001 gives 10^-3 to 10^0 with "Distance (mm)". The prefix depends only on the visible range, so each
/// axis sharing one formatter picks its own.
///
/// A \c tickLabel callback receives the value and the tick step in the prefixed unit; on a logarithmic axis the
/// tick step is the value itself.
///
/// \sa NumericTickLabelFormatter, TickLabelFormatter
class UnitTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(UnitTickLabelFormatter)

    /// \brief Base unit symbol of the axis values, such as "m". Default: empty.
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
    /// \brief Rule that picks the unit prefix from the visible range. Default: \c LargestMagnitude.
    Q_PROPERTY(PrefixMode prefixMode READ prefixMode WRITE setPrefixMode NOTIFY prefixModeChanged)
    /// \brief Unit prefix symbol the tick labels use with \c prefixMode \c Fixed, such as "m". Default: "", the bare
    /// unit.
    ///
    /// A prefix outside \c allowedPrefixes snaps to the allowed prefix with the largest power of ten not above it,
    /// or to the smallest allowed prefix. "µ" (U+00B5), "μ" (U+03BC), and "u" all mean micro and read back as "µ"
    /// (U+00B5). An unknown symbol is ignored with a warning.
    Q_PROPERTY(QString fixedPrefix READ fixedPrefix WRITE setFixedPrefix NOTIFY fixedPrefixChanged)
    /// \brief JavaScript callback \c function(min, max, ticks) that returns the unit prefix symbol with \c prefixMode
    /// \c Custom. Default: \c undefined.
    ///
    /// Receives the visible range and an array of the visible major tick values, all in the base unit. Called on
    /// the GUI thread once per tick computation. A result outside \c allowedPrefixes snaps as with
    /// \c fixedPrefix. An exception, a result that is not a string, or an unknown symbol falls back to
    /// \c LargestMagnitude with a warning.
    Q_PROPERTY(QJSValue prefixCallback READ prefixCallback WRITE setPrefixCallback NOTIFY prefixCallbackChanged)
    /// \brief Unit prefix symbols the tick labels may use, with "" for the bare unit. Default: every power-of-1000
    /// prefix from quecto ("q") to quetta ("Q"), and "".
    ///
    /// Any SI prefix may be listed, including "c", "d", "da", and "h". "µ" (U+00B5), "μ" (U+03BC), and "u" all mean
    /// micro, written as "µ" (U+00B5). An unknown symbol is ignored with a warning. An empty list turns prefixing
    /// off: values are written unscaled in the bare unit.
    ///
    /// Reads back the prefixes in effect, sorted by ascending power of ten, without duplicates or unknown symbols,
    /// and with micro as "µ" (U+00B5). Assign \c undefined to restore the default.
    Q_PROPERTY(QStringList allowedPrefixes READ allowedPrefixes WRITE setAllowedPrefixes RESET resetAllowedPrefixes NOTIFY allowedPrefixesChanged)

public:
    /// \brief Rule that picks the unit prefix.
    enum PrefixMode {
        /// \brief The allowed prefix with the largest power of ten at which the largest visible major tick is still
        /// at least 1, or the smallest allowed prefix when none qualifies, shared by all tick labels.
        LargestMagnitude,
        /// \brief \c fixedPrefix, shared by all tick labels at every zoom level.
        Fixed,
        /// \brief The result of \c prefixCallback, shared by all tick labels.
        Custom,
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

    /// \brief Returns the unit prefix symbol used with \c prefixMode \c Fixed.
    QString fixedPrefix() const;
    /// \brief Sets the unit prefix symbol used with \c prefixMode \c Fixed to \a symbol; an unknown symbol is
    /// ignored with a warning.
    void setFixedPrefix(const QString& symbol);

    /// \brief Returns the JavaScript callback that picks the unit prefix with \c prefixMode \c Custom.
    QJSValue prefixCallback() const;
    /// \brief Sets the JavaScript callback that picks the unit prefix with \c prefixMode \c Custom to \a callback.
    void setPrefixCallback(const QJSValue& callback);

    /// \brief Returns the allowed unit prefix symbols in effect, sorted by ascending power of ten.
    QStringList allowedPrefixes() const;
    /// \brief Sets the allowed unit prefix symbols to \a symbols; unknown symbols are ignored with a warning.
    void setAllowedPrefixes(const QStringList& symbols);
    /// \brief Restores every power-of-1000 prefix from quecto to quetta, and the bare unit.
    void resetAllowedPrefixes();

signals:
    /// \brief Emitted when the unit property changes.
    void unitChanged();
    /// \brief Emitted when the prefixMode property changes.
    void prefixModeChanged();
    /// \brief Emitted when the fixedPrefix property changes.
    void fixedPrefixChanged();
    /// \brief Emitted when the prefixCallback property changes.
    void prefixCallbackChanged();
    /// \brief Emitted when the allowedPrefixes property changes.
    void allowedPrefixesChanged();

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
    void applyAllowedPrefixes(const QList<Internal::UnitPrefix>& prefixes);
    Internal::UnitPrefix sharedPrefix(const TickSet& tickSet) const;
    std::optional<Internal::UnitPrefix> callPrefixCallback(const TickSet& tickSet) const;

    QString unit_;
    PrefixMode prefixMode_{LargestMagnitude};
    Internal::UnitPrefix fixedPrefix_;
    QJSValue prefixCallback_;
    QList<Internal::UnitPrefix> allowedPrefixes_;
};

} // namespace QAccelPlot
