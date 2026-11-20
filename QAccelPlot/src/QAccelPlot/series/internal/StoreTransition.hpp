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

/// \brief Shows the runs of a \c TransitionRunner as the drawn data points of a \c DataPointStore.
///
/// The store's data points have no attributes. The drawn data points change on \c advance(), \c end(),
/// and \c discard(), so the series refreshes what depends on them, such as its hover lookup, after each.
class StoreTransition {
public:
    /// \brief Constructs the transition for \a runner and \a store, which must outlive it.
    StoreTransition(TransitionRunner& runner, DataPointStore& store);

    /// \brief Stores the \a count double data points moved from \a data and starts a run to them from the drawn ones.
    /// \return \c false when no run started; call \c discard() then.
    bool start(std::vector<double>&& data, int count, const TransitionRunner::Layout& layout);
    /// \brief Draws the next frame of the run, or the stored data points again once it has ended.
    /// \return \c false when the drawn data points did not change.
    bool advance();
    /// \brief Ends a run that draws other data points than the stored ones, drawing the stored ones again.
    /// \return \c false when the stored data points were drawn already.
    bool end();
    /// \brief Ends the run and draws the stored data points again.
    void discard();

private:
    TransitionRunner& runner_;
    DataPointStore& store_;
    // Frame buffer of the animation, recycled from frame to frame.
    std::vector<double> frameBuffer_;
};

} // namespace QAccelPlot::Internal
