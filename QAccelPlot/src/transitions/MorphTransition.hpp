//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "transitions/DataTransition.hpp"

namespace QAccelPlot {

/// \brief An animation transition that smoothly interpolates point positions between two datasets.
///
/// Each point is linearly blended from its old position to its new position.
/// When the point counts differ, the shorter dataset is resampled to match.
///
/// \sa DrawTransition, DataTransition
class MorphTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(MorphTransition)

public:
    /// \brief Constructs a MorphTransition with the given \a parent.
    explicit MorphTransition(QObject* parent = nullptr);

protected:
    /// \brief Interpolates point positions between the two datasets at \a easedProgress (0–1).
    void interpolate(float easedProgress, const std::vector<float>& fromData, int fromPointCount, const std::vector<float>& toData, int toPointCount,
        std::vector<float>& outData, int& outPointCount) override;
};

} // namespace QAccelPlot
