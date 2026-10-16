

# File OverlayChildren.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**OverlayChildren.hpp**](OverlayChildren_8hpp.md)

[Go to the documentation of this file](OverlayChildren_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QList>
#include <QPointer>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QVariant>

namespace QAccelPlot {

class OverlayChildren {
public:
    OverlayChildren(QObject& owner, QVariant tool, const char* toolProperty);

    [[nodiscard]] QQmlListProperty<QObject> list();
    void setOverlay(QQuickItem* overlay);

private:
    static void append(QQmlListProperty<QObject>* list, QObject* object);
    static qsizetype count(QQmlListProperty<QObject>* list);
    static QObject* at(QQmlListProperty<QObject>* list, qsizetype index);
    void restack();

    QObject& owner_;
    QVariant tool_;
    const char* toolProperty_;
    QPointer<QQuickItem> overlay_;
    QList<QPointer<QObject>> objects_;
};

} // namespace QAccelPlot
```


