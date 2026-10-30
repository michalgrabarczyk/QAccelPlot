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

/// \brief Namespace exposing the \c MarkerShape enum to QML.
namespace MarkerShapeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(MarkerShape)

/// \brief Marker shapes drawn by series with a \c marker grouped property, such as \c LineCurve and \c PointCloud.
///
/// Every shape except \c Pixel fits within a square whose half-width is the marker size. The shaders
/// select a shape by this value minus one, so append new shapes at the end and never reorder
/// or insert. Series that always draw markers, such as \c PointCloud, do not accept \c None.
enum class Shape {
    None,          ///< \brief No markers.
    Circle,        ///< \brief Circle.
    Square,        ///< \brief Square.
    Diamond,       ///< \brief Diamond, narrower than it is tall.
    TriangleUp,    ///< \brief Equilateral triangle pointing up.
    TriangleDown,  ///< \brief Equilateral triangle pointing down.
    TriangleLeft,  ///< \brief Equilateral triangle pointing left.
    TriangleRight, ///< \brief Equilateral triangle pointing right.
    Cross,         ///< \brief Plus sign (+).
    XCross,        ///< \brief Diagonal cross (×).
    HLine,         ///< \brief Short horizontal line.
    VLine,         ///< \brief Short vertical line, e.g. for rug and event plots.
    Star,          ///< \brief Five-pointed star.
    Asterisk,      ///< \brief Eight-armed asterisk: a thin plus and a thin diagonal cross.
    Pixel,         ///< \brief A single pixel; ignores the marker size, fill, and anti-aliasing. Suited to very dense scatter plots.
    Hexagon,       ///< \brief Regular hexagon with a vertex up.
    Pentagon       ///< \brief Regular pentagon with a vertex up.
};
Q_ENUM_NS(Shape)
} // namespace MarkerShapeNS

using MarkerShape = MarkerShapeNS::Shape;

} // namespace QAccelPlot
