//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QCoreApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

namespace QAccelPlot::Test {

/// \brief Creates the Qt Quick object that \a qml declares, with \c QtQuick imported, owned by \a parent.
inline QObject* createQuickObject(const QString& qml, QObject* parent)
{
    static auto* engine = new QQmlEngine(QCoreApplication::instance());
    auto component = QQmlComponent{engine};
    component.setData(QStringLiteral("import QtQuick\n%1").arg(qml).toUtf8(), QUrl{});
    auto* object = component.create();
    if (!object) {
        qFatal("%s", qPrintable(component.errorString()));
    }
    object->setParent(parent);
    return object;
}

/// \brief Adds the animation that \a qml declares, such as \c NumberAnimation { property: "opacity" }, to \a transition.
inline QObject* addAnimation(DataTransition& transition, const QString& qml)
{
    auto* animation = createQuickObject(qml, &transition);
    auto list = transition.animations();
    list.append(&list, animation);
    return animation;
}

} // namespace QAccelPlot::Test
