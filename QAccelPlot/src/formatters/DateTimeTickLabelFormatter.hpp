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

/// \brief A tick label formatter that displays tick values as formatted date/time strings.
///
/// Tick values are interpreted as milliseconds since the Unix epoch (1970-01-01T00:00:00 UTC).
///
/// \sa NumericTickLabelFormatter, TickLabelFormatter
class DateTimeTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(DateTimeTickLabelFormatter)

    /// \brief The QDateTime format string used to render each tick label. Default: \c "yyyy-MM-dd HH:mm:ss".
    Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat WRITE setDateTimeFormat NOTIFY dateTimeFormatChanged)

public:
    /// \brief Constructs a DateTimeTickLabelFormatter with the given \a parent.
    explicit DateTimeTickLabelFormatter(QObject* parent = nullptr);

    /// \brief Returns the current date/time format string.
    QString dateTimeFormat() const;
    /// \brief Sets the date/time format string to \a format.
    void setDateTimeFormat(const QString& format);

signals:
    /// \brief Emitted when the dateTimeFormat property changes.
    void dateTimeFormatChanged();

protected:
    /// \brief Returns a date/time string for \a value (milliseconds since epoch).
    QString doFormat(qreal value, qreal tickStep) const override;

private:
    QString dateTimeFormat_{"yyyy-MM-dd HH:mm:ss"};
};

} // namespace QAccelPlot
