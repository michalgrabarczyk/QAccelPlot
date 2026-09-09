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

/// \brief The default line style — renders a continuous solid line with no gaps.
class SolidLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(SolidLine)

public:
    /// \brief Constructs a SolidLine with the given \a parent.
    explicit SolidLine(QObject* parent = nullptr);
};

} // namespace QAccelPlot
