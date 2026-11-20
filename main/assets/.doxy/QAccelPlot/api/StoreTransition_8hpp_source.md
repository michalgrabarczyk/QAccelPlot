

# File StoreTransition.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**StoreTransition.hpp**](StoreTransition_8hpp.md)

[Go to the documentation of this file](StoreTransition_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <vector>

namespace QAccelPlot {
class DataPointStore;
}

namespace QAccelPlot::Internal {

class StoreTransition {
public:
    StoreTransition(TransitionRunner& runner, DataPointStore& store);

    bool start(std::vector<double>&& data, int count, const TransitionRunner::Layout& layout);
    bool advance();
    bool end();
    void discard();

private:
    TransitionRunner& runner_;
    DataPointStore& store_;
    // Frame buffer of the animation, recycled from frame to frame.
    std::vector<double> frameBuffer_;
};

} // namespace QAccelPlot::Internal
```


