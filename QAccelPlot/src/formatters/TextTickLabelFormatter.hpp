//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "formatters/TickLabelFormatter.hpp"

#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief A tick label formatter that maps integer tick indices to a user-supplied list of strings.
///
/// The tick value is rounded to the nearest integer and used as an index into \c labels.
/// Values outside the list range are rendered as empty strings.
///
/// \sa NumericTickLabelFormatter, TickLabelFormatter
class TextTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(TextTickLabelFormatter)

    /// \brief The ordered list of label strings, indexed by the rounded tick value.
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged)

public:
    /// \brief Constructs a TextTickLabelFormatter with the given \a parent.
    explicit TextTickLabelFormatter(QObject* parent = nullptr);

    /// \brief Returns the current label list.
    QStringList labels() const;
    /// \brief Sets the label list to \a labels.
    void setLabels(const QStringList& labels);

signals:
    /// \brief Emitted when the labels property changes.
    void labelsChanged();

protected:
    /// \brief Returns the label at the index corresponding to \a value, or an empty string if out of range.
    QString doFormat(qreal value, qreal tickStep) const override;

private:
    QStringList labels_;
};

} // namespace QAccelPlot
