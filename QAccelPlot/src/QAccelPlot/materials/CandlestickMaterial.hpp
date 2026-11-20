//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QSGGeometry>

namespace QAccelPlot {

/// \brief Material that draws candles from (x, open, high, low, close) data points in a data texture.
///
/// Each candle takes \c kVerticesPerCandle vertices: two triangles each for three parts, the wick
/// above and below the body and the body of a candlestick, or the line and two ticks of an OHLC
/// bar. The shader derives the candle, part, and corner from the vertex index. The inherited
/// \c color is the up candle color.
class CandlestickMaterial : public DataTextureMaterial {
public:
    /// \brief Vertices per candle: a quad of two triangles for each of three parts.
    static constexpr int kVerticesPerCandle{18};

    /// \brief Constructs a CandlestickMaterial with default uniform values.
    CandlestickMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the candlestick shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode renderMode) const override;

    /// \brief Returns the vertex attribute set: a 4-byte placeholder the shader ignores.
    static const QSGGeometry::AttributeSet& attributeSet();

    QColor downColor{Qt::red};          ///< \brief Color of down candles.
    QColor hoverColor{Qt::transparent}; ///< \brief Color of the highlighted candle.
    QColor glowDownColor;               ///< \brief Halo color of down candles; \c glow holds the one of up candles.
    float hoveredIndex{-1.0f};          ///< \brief Index of the highlighted candle, or -1 for none.
    float lineWidth{1.0f};              ///< \brief Width of wicks, OHLC lines, and ticks in pixels.
    float halfWidth{0.4f};              ///< \brief Half the candle width in x data units.
    float minimumWidth{1.0f};           ///< \brief Smallest candle width in pixels.
    float ohlc{0.0f};                   ///< \brief 1.0 to draw OHLC bars instead of candlesticks (float for std140 UBO compatibility).
    float candleCount{0.0f};            ///< \brief Number of candles drawn; vertices beyond them are culled.
    float antialiasing{1.0f};           ///< \brief 1.0 to smooth the edges.

protected:
    /// \brief Compares the candle-specific uniforms after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
