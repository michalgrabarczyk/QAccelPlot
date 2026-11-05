//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/TextLabel.hpp"

#include <QPointF>
#include <QPointer>
#include <QQmlComponent>

#include <limits>

namespace QAccelPlot {

/// \brief Axis title and its placement, grouped as the Axis \c label property, e.g. <tt>label.text: "Time (s)"</tt>.
///
/// The title is painted inside the axis item and clipped to it. \c delegate replaces it with a QML item.
///
/// \sa Axis
class AxisLabel : public TextLabel {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Position of the title along the axis. Default: \c Axis.Center.
    Q_PROPERTY(Axis::LabelAlignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    /// \brief Clockwise rotation of the title in degrees.
    ///
    /// Defaults to the side's rotation: 0 on horizontal axes, -90 on the left, and 90 on the right.
    /// Assign \c undefined to return to it.
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation RESET resetRotation NOTIFY rotationChanged)
    /// \brief Pixel offset added to the title position after alignment and padding. Default: (0, 0).
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    /// \brief Optional component that replaces the painted title. Default: \c null.
    ///
    /// The axis creates one instance as its child item, so \c parent is the axis and anchors, \c x,
    /// \c y, and \c rotation position it. The instance may extend beyond the axis. While a delegate
    /// is set, the axis paints no title and ignores \c alignment, \c rotation, \c offset, and \c padding.
    Q_PROPERTY(QQmlComponent* delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)

public:
    /// \brief Constructs an axis label owned by \a parent.
    explicit AxisLabel(QObject* parent = nullptr);

    /// \brief Returns the position of the title along the axis.
    Axis::LabelAlignment alignment() const;
    /// \brief Sets the position of the title along the axis to \a alignment.
    void setAlignment(Axis::LabelAlignment alignment);

    /// \brief Returns the title rotation in degrees, including the side's default rotation.
    qreal rotation() const;
    /// \brief Sets the title rotation to \a degrees. A non-finite value restores the side's default rotation.
    void setRotation(qreal degrees);
    /// \brief Restores the side's default title rotation.
    void resetRotation();
    /// \brief Sets the rotation used while none is set to \a degrees. Called by the owning Axis when its side changes.
    void setDefaultRotation(qreal degrees);

    /// \brief Returns the pixel offset of the title.
    QPointF offset() const;
    /// \brief Sets the pixel offset of the title to \a offset.
    void setOffset(const QPointF& offset);

    /// \brief Returns the component that replaces the painted title, or \c nullptr.
    QQmlComponent* delegate() const;
    /// \brief Sets the component that replaces the painted title to \a delegate. \c nullptr restores the painted title.
    void setDelegate(QQmlComponent* delegate);

signals:
    /// \brief Emitted when the alignment property changes.
    void alignmentChanged();
    /// \brief Emitted when the rotation property changes.
    void rotationChanged();
    /// \brief Emitted when the offset property changes.
    void offsetChanged();
    /// \brief Emitted when the delegate property changes.
    void delegateChanged();

private:
    Axis::LabelAlignment alignment_{Axis::Center};
    // NaN while the title follows the side's default rotation.
    qreal rotation_{std::numeric_limits<qreal>::quiet_NaN()};
    qreal defaultRotation_{0.0};
    QPointF offset_;
    QPointer<QQmlComponent> delegate_;
};

} // namespace QAccelPlot
