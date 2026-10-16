

# File OutlinedRectangle.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**OutlinedRectangle.hpp**](OutlinedRectangle_8hpp.md)

[Go to the documentation of this file](OutlinedRectangle_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QQuickItem>

namespace QAccelPlot {

class OutlinedRectangle : public QQuickItem {
public:
    explicit OutlinedRectangle(QQuickItem* parent = nullptr);

    void setColors(const QColor& fill, const QColor& border);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    QColor fill_;
    QColor border_;
};

} // namespace QAccelPlot
```


