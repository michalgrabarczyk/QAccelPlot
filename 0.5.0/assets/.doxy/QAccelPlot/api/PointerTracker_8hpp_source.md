

# File PointerTracker.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**PointerTracker.hpp**](PointerTracker_8hpp.md)

[Go to the documentation of this file](PointerTracker_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QHoverEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPointF>

#include <optional>

namespace QAccelPlot::Internal {

class PointerTracker final : public QObject {
    Q_OBJECT

public:
    explicit PointerTracker(QObject* parent = nullptr);

    [[nodiscard]] std::optional<QPointF> position() const;

    void press(const QMouseEvent& event);
    void release();
    void hoverEnter(const QHoverEvent& event);
    void hoverMove(const QHoverEvent& event);
    void hoverLeave();
    void mouseMove(const QMouseEvent& event);

signals:
    void changed();

private:
    void setPosition(const QPointF& position);

    std::optional<QPointF> position_;
    // True while the pointer was last placed by a touch, which keeps it in place after the release.
    bool touchPointer_{false};
    bool touchPressed_{false};
};

} // namespace QAccelPlot::Internal
```


