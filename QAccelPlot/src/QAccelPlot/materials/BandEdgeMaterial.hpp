//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/LineMaterial.hpp"

namespace QAccelPlot {

/// \brief Line material for the lower or upper edge line of a band, reading <tt>(x, low, high)</tt> samples.
///
/// Uses the line uniforms and fragment stage; only the vertex stage reads three floats per sample.
class BandEdgeMaterial : public LineMaterial {
public:
    /// \brief Which value of each sample the edge line draws.
    enum class Edge { Lower, Upper };

    /// \brief Constructs a material for the \a edge line.
    explicit BandEdgeMaterial(Edge edge);

    /// \brief Returns the edge line this material draws.
    Edge edge() const;

    /// \brief Returns the material type of this edge; the two edges use different shaders.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the edge line shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

private:
    Edge edge_;
};

} // namespace QAccelPlot
