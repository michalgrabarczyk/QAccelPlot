//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "linestyles/LineStyle.hpp"

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
