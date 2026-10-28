

# File DataTextureUploadTotals.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**DataTextureUploadTotals.hpp**](DataTextureUploadTotals_8hpp.md)

[Go to the documentation of this file](DataTextureUploadTotals_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstdint>

namespace QAccelPlot::Internal {

struct DataTextureUploadTotals {
    std::int64_t count{0}; 
    std::int64_t bytes{0}; 
};

void recordDataTextureUpload(std::int64_t bytes);

DataTextureUploadTotals dataTextureUploadTotals();

void resetDataTextureUploadTotals();

} // namespace QAccelPlot::Internal
```


