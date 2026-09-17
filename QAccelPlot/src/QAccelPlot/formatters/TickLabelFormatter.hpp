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
/// \sa NumericTickLabelFormatter, DateTimeTickLabelFormatter, LogTickLabelFormatter, TextTickLabelFormatter
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

private:
    QJSValue tickLabel_;
};

} // namespace QAccelPlot
