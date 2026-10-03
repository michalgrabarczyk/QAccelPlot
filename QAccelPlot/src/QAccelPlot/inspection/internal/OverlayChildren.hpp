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

/// \brief Objects declared inside an inspection tool in QML.
///
/// Every object that has the tool's property receives the tool. Items among them that are children
/// of the plot's overlay are kept stacked in declaration order above its other children.
class OverlayChildren {
public:
    /// \brief \a tool holds \a owner and is written to the property \a toolProperty of every object.
    OverlayChildren(QObject& owner, QVariant tool, const char* toolProperty);

    /// \brief Returns the list property that appends to these objects.
    [[nodiscard]] QQmlListProperty<QObject> list();
    /// \brief Sets the plot overlay whose children are stacked.
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
