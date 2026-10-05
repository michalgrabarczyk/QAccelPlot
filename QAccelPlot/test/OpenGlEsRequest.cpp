//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "OpenGlEsRequest.hpp"

#include <QSurfaceFormat>

namespace QAccelPlotTest {

std::optional<QVersionNumber> requestedOpenGlEsVersion()
{
    const auto text = qEnvironmentVariable("QACCELPLOT_OPENGL_ES_VERSION").trimmed();
    if (text.isEmpty()) {
        return std::nullopt;
    }
    auto suffixIndex = qsizetype{0};
    const auto version = QVersionNumber::fromString(text, &suffixIndex);
    if (version.isNull() || suffixIndex != text.size() || version.segmentCount() > 2 || version.majorVersion() <= 0) {
        qFatal("QACCELPLOT_OPENGL_ES_VERSION must look like \"3.0\", got \"%s\".", qPrintable(text));
    }
    return version;
}

} // namespace QAccelPlotTest

namespace {

// Runs before main(): Qt Quick derives its OpenGL context from the default format, which must be
// set before the first window is created.
bool requestOpenGlEs()
{
    const auto version = QAccelPlotTest::requestedOpenGlEsVersion();
    if (!version) {
        return false;
    }
    auto format = QSurfaceFormat::defaultFormat();
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(version->majorVersion(), version->minorVersion());
    QSurfaceFormat::setDefaultFormat(format);
    return true;
}

const auto kOpenGlEsRequested = requestOpenGlEs();

} // namespace
