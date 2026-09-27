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

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

/// \brief Controls the outline a \c RectangleList draws inside each rectangle's edges.
///
/// Accessible via the \c border CONSTANT grouped property of \c RectangleList, for example
/// <tt>border.width: 1</tt>.
///
/// \sa RectangleList
class RectangleBorder : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    /// \brief Outline width in pixels, clamped to at least 0. Default: 0, no outline.
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    /// \brief Outline color. Default: black.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    /// \brief Constructs a RectangleBorder owned by \a parent.
    explicit RectangleBorder(QObject* parent = nullptr);

    /// \brief Returns the outline width in pixels.
    qreal width() const;
    /// \brief Sets the outline width to \a width pixels. Negative values are clamped to 0.
    void setWidth(qreal width);

    /// \brief Returns the outline color.
    QColor color() const;
    /// \brief Sets the outline color to \a color.
    void setColor(const QColor& color);

signals:
    /// \brief Emitted when the width property changes.
    void widthChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();

private:
    qreal width_{0.0};
    QColor color_{Qt::black};
};

} // namespace QAccelPlot
