

# File OutlinedRectangle.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**OutlinedRectangle.hpp**](OutlinedRectangle_8hpp.md)

[Go to the documentation of this file](OutlinedRectangle_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QColor>
#include <QQuickItem>

namespace QAccelPlot::Internal {

class OutlinedRectangle : public QQuickItem {
public:
    explicit OutlinedRectangle(QQuickItem* parent = nullptr);

    void setStyle(const QColor& fill, const QColor& borderColor, qreal borderWidth);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    QColor fill_;
    QColor border_;
    qreal borderWidth_{0.0};
};

} // namespace QAccelPlot::Internal
```


