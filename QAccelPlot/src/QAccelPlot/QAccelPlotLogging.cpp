//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/QAccelPlotLogging.hpp"

namespace QAccelPlot {

// Default level is QtWarningMsg so debug messages are silent unless explicitly enabled, e.g.:
// QT_LOGGING_RULES="QAccelPlot.debug=true"
Q_LOGGING_CATEGORY(lcQAccelPlot, "QAccelPlot", QtWarningMsg)

} // namespace QAccelPlot
