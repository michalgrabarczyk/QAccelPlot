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

/// \brief A property value that is either set explicitly or follows a default.
///
/// Backs a QML property with a \c RESET function: \c value() is the value in effect, and each
/// mutator returns whether it changed, so the owner knows when to emit the change signal.
template <typename T> class ResettableValue {
public:
    /// \brief Constructs an unset value that follows \a fallback.
    explicit ResettableValue(T fallback = T{})
        : fallback_(std::move(fallback))
    {
    }

    /// \brief Returns the explicit value when set, otherwise the default.
    [[nodiscard]] const T& value() const
    {
        return explicit_ ? *explicit_ : fallback_;
    }

    /// \brief Returns whether a value is set explicitly.
    [[nodiscard]] bool isSet() const
    {
        return explicit_.has_value();
    }

    /// \brief Sets the value to \a value; returns whether the value in effect changed.
    bool set(const T& value)
    {
        if (explicit_ && equal(*explicit_, value)) {
            return false;
        }
        const auto changed = !equal(this->value(), value);
        explicit_ = value;
        return changed;
    }

    /// \brief Unsets the value, so it follows the default again; returns whether the value in effect changed.
    bool reset()
    {
        if (!explicit_) {
            return false;
        }
        const auto changed = !equal(*explicit_, fallback_);
        explicit_.reset();
        return changed;
    }

    /// \brief Sets the default to \a fallback; returns whether the value in effect changed.
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
