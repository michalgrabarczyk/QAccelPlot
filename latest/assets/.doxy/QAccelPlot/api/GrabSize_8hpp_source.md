

# File GrabSize.hpp

[**File List**](files.md) **>** [**export**](dir_165621f1d3f543afcb149fc22fafadb8.md) **>** [**internal**](dir_502a5035d8e883b896944d94c324e4a9.md) **>** [**GrabSize.hpp**](GrabSize_8hpp.md)

[Go to the documentation of this file](GrabSize_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QSize>

namespace QAccelPlot::Internal {

struct GrabSize {
    QSize target; 
    QSize image;  
};

GrabSize grabSizeFor(const QSize& pixels, qreal devicePixelRatio);

} // namespace QAccelPlot::Internal
```


