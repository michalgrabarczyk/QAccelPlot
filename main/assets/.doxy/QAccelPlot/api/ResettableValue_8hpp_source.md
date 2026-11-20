

# File ResettableValue.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**ResettableValue.hpp**](ResettableValue_8hpp.md)

[Go to the documentation of this file](ResettableValue_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/MathUtils.hpp"

#include <optional>
#include <type_traits>
#include <utility>

namespace QAccelPlot::Internal {

template <typename T> class ResettableValue {
public:
    explicit ResettableValue(T fallback = T{})
        : fallback_(std::move(fallback))
    {
    }

    [[nodiscard]] const T& value() const
    {
        return explicit_ ? *explicit_ : fallback_;
    }

    [[nodiscard]] bool isSet() const
    {
        return explicit_.has_value();
    }

    bool set(const T& value)
    {
        if (explicit_ && equal(*explicit_, value)) {
            return false;
        }
        const auto changed = !equal(this->value(), value);
        explicit_ = value;
        return changed;
    }

    bool reset()
    {
        if (!explicit_) {
            return false;
        }
        const auto changed = !equal(*explicit_, fallback_);
        explicit_.reset();
        return changed;
    }

    bool setFallback(const T& fallback)
    {
        if (equal(fallback_, fallback)) {
            return false;
        }
        fallback_ = fallback;
        return !explicit_;
    }

private:
    static bool equal(const T& left, const T& right)
    {
        if constexpr (std::is_floating_point_v<T>) {
            return nearlyEqual(left, right);
        } else {
            return left == right;
        }
    }

    std::optional<T> explicit_;
    T fallback_;
};

} // namespace QAccelPlot::Internal
```


