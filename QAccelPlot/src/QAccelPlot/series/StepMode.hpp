//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Namespace exposing the \c StepMode enum to QML.
namespace StepModeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(StepMode)

/// \brief How a line joins consecutive data points: straight, or as a horizontal step and a vertical riser.
///
/// A step mode decides where each data point's value holds. The line starts at the first data point and
/// ends at the last one; a step never reaches past them.
enum class Mode {
    None,   ///< \brief Straight segments between data points.
    Before, ///< \brief Each value holds from the previous data point's X up to its own X: the line rises first.
    After,  ///< \brief Each value holds from its own X up to the next data point's X: the line runs flat first.
    Center, ///< \brief Each value holds between the midpoints to its neighbors, halfway on screen (the geometric mean on a log axis).
};
Q_ENUM_NS(Mode)
} // namespace StepModeNS

using StepMode = StepModeNS::Mode;

} // namespace QAccelPlot
