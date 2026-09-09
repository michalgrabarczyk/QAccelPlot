//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "formatters/TickLabelFormatter.hpp"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPointer>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Controls the visual appearance of ticks, sub-ticks, and tick labels on an \c Axis.
///
/// Accessible via the \c Axis::ticker CONSTANT property. Changes are applied on the next
/// paint event.
///
/// \sa Axis
class AxisTicker : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Color of major tick marks. Default: \c Qt::black.
    Q_PROPERTY(QColor tickColor READ tickColor WRITE setTickColor NOTIFY tickColorChanged)
    /// \brief Optional color of tick labels. When invalid, the tickColor is used.
    Q_PROPERTY(QColor tickLabelColor READ tickLabelColor WRITE setTickLabelColor NOTIFY tickLabelColorChanged)
    /// \brief Target number of major tick marks. Actual count is adjusted for "nice" values. Default: 5.
    Q_PROPERTY(int tickCount READ tickCount WRITE setTickCount NOTIFY tickCountChanged)
    /// \brief Number of sub-tick intervals between adjacent major ticks. Default: 10.
    Q_PROPERTY(int subtickCount READ subtickCount WRITE setSubtickCount NOTIFY subtickCountChanged)
    /// \brief Convenience setter for both \c tickLengthIn and \c tickLengthOut simultaneously.
    Q_PROPERTY(qreal tickLength READ tickLength WRITE setTickLength NOTIFY tickLengthChanged)
    /// \brief Convenience setter for both \c subtickLengthIn and \c subtickLengthOut simultaneously.
    Q_PROPERTY(qreal subtickLength READ subtickLength WRITE setSubtickLength NOTIFY subtickLengthChanged)
    /// \brief Length in pixels of the portion of a major tick that extends into the plot area. Default: 8.
    Q_PROPERTY(qreal tickLengthIn READ tickLengthIn WRITE setTickLengthIn NOTIFY tickLengthInChanged)
    /// \brief Length in pixels of the portion of a major tick that extends outside the plot area. Default: 8.
    Q_PROPERTY(qreal tickLengthOut READ tickLengthOut WRITE setTickLengthOut NOTIFY tickLengthOutChanged)
    /// \brief Inward length in pixels of sub-tick marks. Default: 4.
    Q_PROPERTY(qreal subtickLengthIn READ subtickLengthIn WRITE setSubtickLengthIn NOTIFY subtickLengthInChanged)
    /// \brief Outward length in pixels of sub-tick marks. Default: 4.
    Q_PROPERTY(qreal subtickLengthOut READ subtickLengthOut WRITE setSubtickLengthOut NOTIFY subtickLengthOutChanged)
    /// \brief Color of sub-tick marks. Default: \c Qt::darkGray.
    Q_PROPERTY(QColor subtickColor READ subtickColor WRITE setSubtickColor NOTIFY subtickColorChanged)
    /// \brief Width in pixels of major tick mark lines. Default: 2.
    Q_PROPERTY(qreal tickWidth READ tickWidth WRITE setTickWidth NOTIFY tickWidthChanged)
    /// \brief Width in pixels of sub-tick mark lines. Default: 1.
    Q_PROPERTY(qreal subtickWidth READ subtickWidth WRITE setSubtickWidth NOTIFY subtickWidthChanged)
    /// \brief Spacing in pixels between a tick mark and its label. Default: 5.
    Q_PROPERTY(qreal tickLabelPadding READ tickLabelPadding WRITE setTickLabelPadding NOTIFY tickLabelPaddingChanged)
    /// \brief Rotation angle in degrees applied to each tick label. Default: 0.
    Q_PROPERTY(qreal tickLabelRotation READ tickLabelRotation WRITE setTickLabelRotation NOTIFY tickLabelRotationChanged)
    /// \brief Font used to render tick labels. Default: application default font.
    Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont NOTIFY tickLabelFontChanged)
    /// \brief Optional custom formatter for tick labels. When \c nullptr the default NumericTickLabelFormatter is used.
    Q_PROPERTY(TickLabelFormatter* tickLabelFormatter READ tickLabelFormatter WRITE setTickLabelFormatter NOTIFY tickLabelFormatterChanged)

public:
    /// \brief Constructs an AxisTicker with the given \a parent.
    explicit AxisTicker(QObject* parent = nullptr);

    /// \brief Returns the major tick color.
    QColor tickColor() const;
    /// \brief Sets the major tick color to \a c.
    void setTickColor(const QColor& c);

    /// \brief Returns the tick-label color, or an invalid color when it follows tickColor.
    QColor tickLabelColor() const;
    /// \brief Sets the tick-label color to \a c. An invalid color restores the tickColor fallback.
    void setTickLabelColor(const QColor& c);

    /// \brief Returns the target number of major ticks.
    int tickCount() const;
    /// \brief Sets the target major tick count to \a count.
    void setTickCount(int count);

    /// \brief Returns the sub-tick interval count.
    int subtickCount() const;
    /// \brief Sets the sub-tick interval count to \a count.
    void setSubtickCount(int count);

    /// \brief Returns the shared inward+outward tick length (reads \c tickLengthIn).
    qreal tickLength() const;
    /// \brief Sets both \c tickLengthIn and \c tickLengthOut to \a length.
    void setTickLength(qreal length);

    /// \brief Returns the shared inward+outward sub-tick length (reads \c subtickLengthIn).
    qreal subtickLength() const;
    /// \brief Sets both \c subtickLengthIn and \c subtickLengthOut to \a length.
    void setSubtickLength(qreal length);

    /// \brief Returns the inward major tick length.
    qreal tickLengthIn() const;
    /// \brief Sets the inward major tick length to \a length.
    void setTickLengthIn(qreal length);

    /// \brief Returns the outward major tick length.
    qreal tickLengthOut() const;
    /// \brief Sets the outward major tick length to \a length.
    void setTickLengthOut(qreal length);

    /// \brief Returns the inward sub-tick length.
    qreal subtickLengthIn() const;
    /// \brief Sets the inward sub-tick length to \a length.
    void setSubtickLengthIn(qreal length);

    /// \brief Returns the outward sub-tick length.
    qreal subtickLengthOut() const;
    /// \brief Sets the outward sub-tick length to \a length.
    void setSubtickLengthOut(qreal length);

    /// \brief Returns the sub-tick color.
    QColor subtickColor() const;
    /// \brief Sets the sub-tick color to \a c.
    void setSubtickColor(const QColor& c);

    /// \brief Returns the major tick line width.
    qreal tickWidth() const;
    /// \brief Sets the major tick line width to \a width.
    void setTickWidth(qreal width);

    /// \brief Returns the sub-tick line width.
    qreal subtickWidth() const;
    /// \brief Sets the sub-tick line width to \a width.
    void setSubtickWidth(qreal width);

    /// \brief Returns the tick label padding.
    qreal tickLabelPadding() const;
    /// \brief Sets the tick label padding to \a padding.
    void setTickLabelPadding(qreal padding);

    /// \brief Returns the tick label rotation in degrees.
    qreal tickLabelRotation() const;
    /// \brief Sets the tick label rotation to \a rotation degrees.
    void setTickLabelRotation(qreal rotation);

    /// \brief Returns the tick label font.
    QFont tickLabelFont() const;
    /// \brief Sets the tick label font to \a f.
    void setTickLabelFont(const QFont& f);

    /// \brief Returns the custom tick label formatter, or \c nullptr if using the default.
    TickLabelFormatter* tickLabelFormatter() const;
    /// \brief Sets the tick label formatter to \a formatter.
    void setTickLabelFormatter(TickLabelFormatter* formatter);

signals:
    /// \brief Emitted when the tickColor property changes.
    void tickColorChanged();
    /// \brief Emitted when the tickLabelColor property changes.
    void tickLabelColorChanged();
    /// \brief Emitted when the tickCount property changes.
    void tickCountChanged();
    /// \brief Emitted when the subtickCount property changes.
    void subtickCountChanged();
    /// \brief Emitted when the tickLength convenience property changes.
    void tickLengthChanged();
    /// \brief Emitted when the subtickLength convenience property changes.
    void subtickLengthChanged();
    /// \brief Emitted when the tickLengthIn property changes.
    void tickLengthInChanged();
    /// \brief Emitted when the tickLengthOut property changes.
    void tickLengthOutChanged();
    /// \brief Emitted when the subtickLengthIn property changes.
    void subtickLengthInChanged();
    /// \brief Emitted when the subtickLengthOut property changes.
    void subtickLengthOutChanged();
    /// \brief Emitted when the subtickColor property changes.
    void subtickColorChanged();
    /// \brief Emitted when the tickWidth property changes.
    void tickWidthChanged();
    /// \brief Emitted when the subtickWidth property changes.
    void subtickWidthChanged();
    /// \brief Emitted when the tickLabelPadding property changes.
    void tickLabelPaddingChanged();
    /// \brief Emitted when the tickLabelRotation property changes.
    void tickLabelRotationChanged();
    /// \brief Emitted when the tickLabelFont property changes.
    void tickLabelFontChanged();
    /// \brief Emitted when the tickLabelFormatter property changes.
    void tickLabelFormatterChanged();
    /// \brief Emitted when the current formatter's output configuration changes.
    void tickLabelFormatChanged();

private:
    QColor tickColor_{QColor(Qt::black)};
    QColor tickLabelColor_;
    int tickCount_{5};
    int subtickCount_{10};
    qreal tickLengthIn_{8.0};
    qreal tickLengthOut_{8.0};
    qreal subtickLengthIn_{4.0};
    qreal subtickLengthOut_{4.0};
    QColor subtickColor_{QColor(Qt::darkGray)};
    qreal tickWidth_{2.0};
    qreal subtickWidth_{1.0};
    qreal tickLabelPadding_{5.0};
    qreal tickLabelRotation_{0.0};
    QFont tickLabelFont_;
    TickLabelFormatter* defaultTickLabelFormatter_{nullptr};
    QPointer<TickLabelFormatter> customTickLabelFormatter_;
    QMetaObject::Connection customFormatterDestroyedConnection_;
};

} // namespace QAccelPlot
