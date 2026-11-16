//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

namespace QAccelPlot {

/// \brief The default line style — renders a continuous solid line with no gaps.
class SolidLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(SolidLine)

public:
    /// \brief Constructs a SolidLine with the given \a parent.
    explicit SolidLine(QObject* parent = nullptr);
};

} // namespace QAccelPlot
