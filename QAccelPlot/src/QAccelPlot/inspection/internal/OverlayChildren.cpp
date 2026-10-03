//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/OverlayChildren.hpp"

#include <utility>

namespace QAccelPlot {

OverlayChildren::OverlayChildren(QObject& owner, QVariant tool, const char* toolProperty)
    : owner_(owner)
    , tool_(std::move(tool))
    , toolProperty_(toolProperty)
{
}

QQmlListProperty<QObject> OverlayChildren::list()
{
    return {&owner_, this, &OverlayChildren::append, &OverlayChildren::count, &OverlayChildren::at, nullptr};
}

void OverlayChildren::setOverlay(QQuickItem* overlay)
{
    overlay_ = overlay;
    restack();
}

void OverlayChildren::append(QQmlListProperty<QObject>* list, QObject* object)
{
    if (!object) {
        return;
    }
    auto* self = static_cast<OverlayChildren*>(list->data);
    self->objects_.push_back(object);
    if (object->metaObject()->indexOfProperty(self->toolProperty_) >= 0) {
        object->setProperty(self->toolProperty_, self->tool_);
    }
    if (auto* item = qobject_cast<QQuickItem*>(object)) {
        // QML moves the items to the overlay in an unspecified order.
        QObject::connect(item, &QQuickItem::parentChanged, &self->owner_, [self]() { self->restack(); });
    }
}

qsizetype OverlayChildren::count(QQmlListProperty<QObject>* list)
{
    return static_cast<OverlayChildren*>(list->data)->objects_.size();
}

QObject* OverlayChildren::at(QQmlListProperty<QObject>* list, const qsizetype index)
{
    return static_cast<OverlayChildren*>(list->data)->objects_.at(index);
}

void OverlayChildren::restack()
{
    if (!overlay_) {
        return;
    }
    auto* previous = static_cast<QQuickItem*>(nullptr);
    for (const auto& object : std::as_const(objects_)) {
        auto* item = qobject_cast<QQuickItem*>(object.data());
        if (!item || item->parentItem() != overlay_) {
            continue;
        }
        auto* below = previous ? previous : overlay_->childItems().constLast();
        if (below != item) {
            item->stackAfter(below);
        }
        previous = item;
    }
}

} // namespace QAccelPlot
