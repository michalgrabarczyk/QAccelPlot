//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlotLogging.hpp"

namespace QAccelPlot {

// Default level is QtWarningMsg so debug messages are silent unless explicitly enabled, e.g.:
// QT_LOGGING_RULES="QAccelPlot.debug=true"
Q_LOGGING_CATEGORY(lcQAccelPlot, "QAccelPlot", QtWarningMsg)

} // namespace QAccelPlot
