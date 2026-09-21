//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QJSValue>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Abstract base class for tick label formatters.
///
/// Subclasses implement \c doFormat() to produce a display string for each tick value.
/// An optional \c tickLabel JavaScript callback can override the default formatting at the QML level.
///
/// \sa NumericTickLabelFormatter, DateTimeTickLabelFormatter, TextTickLabelFormatter
class TickLabelFormatter : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Optional JavaScript callback \c function(value, tickStep) that overrides \c doFormat().
    Q_PROPERTY(QJSValue tickLabel READ tickLabel WRITE setTickLabel NOTIFY tickLabelChanged)

public:
    /// \brief Constructs an TickLabelFormatter with the given \a parent.
    explicit TickLabelFormatter(QObject* parent = nullptr);

    /// \brief Returns the display string for \a value at the given \a tickStep.
    ///
    /// Calls the \c tickLabel JS callback if set; otherwise delegates to \c doFormat().
    QString format(qreal value, qreal tickStep) const;

    /// \brief Returns the display string for a major tick at \a value on a logarithmic axis.
    ///
    /// Log-scale ticks have no single step, so the \c tickLabel JS callback receives \a value as
    /// its \c tickStep. Without a callback this delegates to \c doFormatLogTick().
    QString formatLogTick(qreal value) const;

    /// \brief Returns the optional JavaScript override callback.
    QJSValue tickLabel() const;
    /// \brief Sets the JavaScript override callback to \a tickLabel.
    void setTickLabel(const QJSValue& tickLabel);

signals:
    /// \brief Emitted when the tickLabel property changes.
    void tickLabelChanged();
    /// \brief Emitted whenever a property that affects formatted output changes.
    void formatChanged();

protected:
    /// \brief Subclass entry point — returns the formatted label for \a value.
    virtual QString doFormat(qreal value, qreal tickStep) const = 0;

    /// \brief Subclass entry point for log-scale major ticks — returns the label for \a value.
    ///
    /// The default implementation returns \c doFormat(value, value), using the tick's own
    /// magnitude as a proxy for the precision it needs.
    virtual QString doFormatLogTick(qreal value) const;

private:
    /// \brief Calls the \c tickLabel JS callback; returns \c true and sets \a label on success.
    bool callTickLabel(qreal value, qreal tickStep, QString& label) const;

    QJSValue tickLabel_;
};

} // namespace QAccelPlot
