//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QJSValue>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief The visible major ticks of one axis viewport, passed to \c TickLabelFormatter::formatTicks().
struct TickSet {
    QList<qreal> values;    ///< \brief Data-space values of the visible major ticks, in ascending order.
    qreal viewportMin{0.0}; ///< \brief Lower bound of the visible range.
    qreal viewportMax{0.0}; ///< \brief Upper bound of the visible range.
    qreal step{0.0};        ///< \brief Major tick step on a linear scale; 0 on a logarithmic scale.
    bool logScale{false};   ///< \brief Whether the values were placed at powers of ten.
};

/// \brief The text a formatter returns for a \c TickSet.
struct TickSetLabels {
    QStringList labels; ///< \brief One tick label per tick value, in the same order.
    QString unitText;   ///< \brief Unit text for the axis label, shared by the whole tick set; empty for none.
};

/// \brief Abstract base class for tick label formatters.
///
/// Subclasses implement \c doFormat() to produce a display string for each tick value, and may override
/// \c doFormatTicks() to label all visible major ticks at once and add unit text for the axis label.
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

    /// \brief Returns the tick labels and unit text for the visible major ticks in \a tickSet.
    ///
    /// Delegates to \c doFormatTicks(). The result has exactly one label per tick value: missing labels are
    /// empty and extra ones are dropped. Called on the GUI thread, once per tick computation. The formatter may
    /// be shared by several axes, so anything decided for one tick set belongs in the result, not in the formatter.
    TickSetLabels formatTicks(const TickSet& tickSet) const;

    /// \brief Returns the smallest major tick step a linear axis may use with this formatter.
    ///
    /// The axis raises a smaller "nice" step to this value. The default is \c 0, no minimum.
    /// Called from the scene graph sync as well as the GUI thread, so overrides must not call into QML.
    virtual qreal minimumTickStep() const;

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

    /// \brief Subclass entry point for a whole tick set — returns its tick labels and unit text.
    ///
    /// Override to decide something once for all visible major ticks, such as a shared unit prefix. The default
    /// implementation labels each tick with \c format(), or \c formatLogTick() on a logarithmic scale, and returns
    /// no unit text.
    virtual TickSetLabels doFormatTicks(const TickSet& tickSet) const;

private:
    /// \brief Calls the \c tickLabel JS callback; returns \c true and sets \a label on success.
    bool callTickLabel(qreal value, qreal tickStep, QString& label) const;

    QJSValue tickLabel_;
};

} // namespace QAccelPlot
