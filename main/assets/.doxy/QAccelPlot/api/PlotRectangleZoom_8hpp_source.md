

# File PlotRectangleZoom.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**PlotRectangleZoom.hpp**](PlotRectangleZoom_8hpp.md)

[Go to the documentation of this file](PlotRectangleZoom_8hpp.md)


```C++
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

class PlotView;

class PlotRectangleZoom : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Qt::KeyboardModifiers modifiers READ modifiers WRITE setModifiers NOTIFY modifiersChanged)
    Q_PROPERTY(qreal minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(Border* border READ border CONSTANT)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QRectF selectionRect READ selectionRect NOTIFY selectionRectChanged)

public:
    explicit PlotRectangleZoom(QObject* parent = nullptr);

    bool enabled() const;
    void setEnabled(bool value);

    Qt::KeyboardModifiers modifiers() const;
    void setModifiers(Qt::KeyboardModifiers value);

    qreal minimumSize() const;
    void setMinimumSize(qreal value);

    QColor fillColor() const;
    void setFillColor(const QColor& value);

    Border* border() const;

    bool isVisible() const;
    void setVisible(bool value);

    bool active() const;
    QRectF selectionRect() const;

signals:
    void enabledChanged();
    void modifiersChanged();
    void minimumSizeChanged();
    void fillColorChanged();
    void visibleChanged();
    void activeChanged();
    void selectionRectChanged();

private:
    friend class PlotView;
    void setSelection(bool active, const QRectF& rect);

    bool enabled_{false};
    Qt::KeyboardModifiers modifiers_{Qt::ControlModifier};
    qreal minimumSize_{6.0};
    QColor fillColor_{ColorPalette::dark().rectangleZoomFill};
    Border* border_{new Border{ColorPalette::dark().rectangleZoomBorder, 1.0, this}};
    bool visible_{true};
    bool active_{false};
    QRectF selectionRect_;
};

} // namespace QAccelPlot
```


