//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Decorative frame configuration exposed by \c PlotView::border.
class PlotBorder : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Border color. Default: transparent.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Border width in pixels. Default: 0 (disabled).
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)

public:
    explicit PlotBorder(QObject* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    qreal width() const;
    void setWidth(qreal width);

signals:
    void colorChanged();
    void widthChanged();

private:
    QColor color_{Qt::transparent};
    qreal width_{0.0};
};

} // namespace QAccelPlot
