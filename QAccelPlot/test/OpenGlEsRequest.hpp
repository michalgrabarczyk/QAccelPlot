//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QVersionNumber>

#include <optional>

namespace QAccelPlotTest {

/// \brief Returns the OpenGL ES version named by \c QACCELPLOT_OPENGL_ES_VERSION, e.g. "3.0", if it is set.
///
/// Every test built with add_qaccelplot_test() requests a context of that version for its windows,
/// so a desktop Qt build exercises the GLSL ES shader path used on embedded devices.
std::optional<QVersionNumber> requestedOpenGlEsVersion();

} // namespace QAccelPlotTest
