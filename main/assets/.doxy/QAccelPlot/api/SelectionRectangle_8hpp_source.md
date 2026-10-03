

# File SelectionRectangle.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**SelectionRectangle.hpp**](SelectionRectangle_8hpp.md)

[Go to the documentation of this file](SelectionRectangle_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QQuickItem>

namespace QAccelPlot {

class OutlinedRectangle;
class SelectionTool;

class SelectionRectangle final : public QQuickItem {
public:
    explicit SelectionRectangle(SelectionTool& tool);

private:
    void attach();
    void sync();

    SelectionTool& tool_;
    OutlinedRectangle* rectangle_;
};

} // namespace QAccelPlot
```


