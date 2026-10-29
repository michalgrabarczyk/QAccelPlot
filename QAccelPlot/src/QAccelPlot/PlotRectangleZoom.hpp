//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/Border.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QColor>
#include <QObject>
#include <QRectF>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class QAccelPlot;

/// \brief Rectangle zoom configuration and selection state exposed by PlotView.
class PlotRectangleZoom : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Enables rectangle selection with the left mouse button. Default: false.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// \brief Required Qt::KeyboardModifiers for starting selection. Default: Qt::ControlModifier.
    Q_PROPERTY(Qt::KeyboardModifiers modifiers READ modifiers WRITE setModifiers NOTIFY modifiersChanged)
    /// \brief Minimum selection width and height in logical pixels. Default: 6; clamped to zero or greater.
    Q_PROPERTY(qreal minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    /// \brief Selection fill color. Default: Colors.dark.rectangleZoomFill.
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    /// \brief Selection outline inside the rectangle's edges. Default: Colors.dark.rectangleZoomBorder, width 1.
    Q_PROPERTY(Border* border READ border CONSTANT)
    /// \brief Whether a rectangle selection is in progress.
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    /// \brief Selection rectangle in plot item-local logical pixels; empty when inactive.
    Q_PROPERTY(QRectF selectionRect READ selectionRect NOTIFY selectionRectChanged)

public:
    /// \brief Constructs configuration owned by the given parent.
    explicit PlotRectangleZoom(QObject* parent = nullptr);

    /// \brief Returns whether rectangle selection is enabled.
    bool enabled() const;
    /// \brief Enables or disables rectangle selection; changing this setting cancels an active selection.
    void setEnabled(bool value);

    /// \brief Returns the required keyboard modifiers.
    Qt::KeyboardModifiers modifiers() const;
    /// \brief Sets the exact keyboard modifiers required at press time; Qt::NoModifier allows unmodified dragging.
    void setModifiers(Qt::KeyboardModifiers value);

    /// \brief Returns the minimum selection width and height in logical pixels.
    qreal minimumSize() const;
    /// \brief Sets the minimum size, clamping negative values to zero and ignoring nonfinite values.
    void setMinimumSize(qreal value);

    /// \brief Returns the selection fill color.
    QColor fillColor() const;
    /// \brief Sets the selection fill color.
    void setFillColor(const QColor& value);

    /// \brief Returns the selection outline settings. The object is owned by this configuration.
    Border* border() const;

    /// \brief Returns whether selection is in progress.
    bool active() const;
    /// \brief Returns the current item-local selection rectangle.
    QRectF selectionRect() const;

signals:
    /// \brief Emitted when enabled changes.
    void enabledChanged();
    /// \brief Emitted when modifiers changes.
    void modifiersChanged();
    /// \brief Emitted when minimumSize changes.
    void minimumSizeChanged();
    /// \brief Emitted when fillColor changes.
    void fillColorChanged();
    /// \brief Emitted when selection starts or ends.
    void activeChanged();
    /// \brief Emitted when the selection rectangle changes.
    void selectionRectChanged();

private:
    friend class QAccelPlot;
    void setSelection(bool active, const QRectF& rect);

    bool enabled_{false};
    Qt::KeyboardModifiers modifiers_{Qt::ControlModifier};
    qreal minimumSize_{6.0};
    QColor fillColor_{ColorPalette::dark().rectangleZoomFill};
    Border* border_{new Border{ColorPalette::dark().rectangleZoomBorder, 1.0, this}};
    bool active_{false};
    QRectF selectionRect_;
};

} // namespace QAccelPlot
