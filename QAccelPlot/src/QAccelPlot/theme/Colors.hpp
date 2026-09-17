//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QObject>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
// QML_SINGLETON needs QQmlPrivate declared, which qqmlregistration.h alone does not pull in
// on Qt versions before the standalone QtQmlIntegration module (introduced in Qt 6.5).
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

/// \brief QML singleton exposing QAccelPlot's built-in color palettes.
///
/// \par Usage
/// \code
/// import QAccelPlot as QAccelPlot
///
/// QAccelPlot.Plot {
///     plotAreaColor: QAccelPlot.Colors.dark.plotArea
/// }
/// \endcode
///
/// \sa ColorPalette
class Colors : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Colors)
    QML_SINGLETON
    /// \brief Read-only constant: dark theme palette. C++ property defaults use these colors.
    Q_PROPERTY(ColorPalette* dark READ dark CONSTANT)
    /// \brief Read-only constant: light theme palette.
    Q_PROPERTY(ColorPalette* light READ light CONSTANT)

public:
    explicit Colors(QObject* parent = nullptr);

    ColorPalette* dark() const;
    ColorPalette* light() const;
};

} // namespace QAccelPlot
