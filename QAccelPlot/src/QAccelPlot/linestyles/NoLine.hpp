//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

namespace QAccelPlot {

/// \brief A line style that suppresses line rendering entirely, leaving only markers visible.
class NoLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(NoLine)

public:
    /// \brief Constructs a NoLine with the given \a parent.
    explicit NoLine(QObject* parent = nullptr);

    /// \brief Returns \c false, suppressing line rendering for this curve.
    bool showLine() const override;
};

} // namespace QAccelPlot
