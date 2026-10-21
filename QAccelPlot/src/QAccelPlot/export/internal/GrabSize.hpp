//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QSize>

namespace QAccelPlot::Internal {

/// \brief Sizes of one item grab.
struct GrabSize {
    QSize target; ///< \brief Size to pass to \c QQuickItem::grabToImage().
    QSize image;  ///< \brief Pixel size of the image the grab renders.
};

/// \brief Returns the grab whose image is closest to \a pixels in a window at \a devicePixelRatio.
///
/// Qt 6.2 renders the target size in pixels. Later versions multiply it by the device pixel ratio
/// and round, so some pixel counts cannot be rendered and the image can differ from \a pixels by a
/// pixel; a tie picks the smaller image.
GrabSize grabSizeFor(const QSize& pixels, qreal devicePixelRatio);

} // namespace QAccelPlot::Internal
