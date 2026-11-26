

# File NumericLabel.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**internal**](dir_5d21a914f9fc6a0914b7bb5bdcf315a8.md) **>** [**NumericLabel.hpp**](NumericLabel_8hpp.md)

[Go to the documentation of this file](NumericLabel_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QString>

namespace QAccelPlot::Internal {

QString numericLabel(qreal value, qreal tickStep);

QString logTickLabel(qreal value);

} // namespace QAccelPlot::Internal
```


