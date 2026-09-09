//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/GradientTexture.hpp"

#include <QColorSpace>
#include <QQuickWindow>
#include <QSGTexture>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {
constexpr auto kGradientTextureWidth = int{256};
constexpr auto kMinStopRange = float{1e-6f};

QColor evaluateGradient(const std::vector<GradientStopData>& stops, const float position)
{
    if (position <= stops.front().position) {
        return stops.front().color;
    }
    if (position >= stops.back().position) {
        return stops.back().color;
    }

    for (std::size_t index = 1; index < stops.size(); ++index) {
        const auto& left = stops[index - 1];
        const auto& right = stops[index];
        if (position > right.position) {
            continue;
        }

        const auto range = std::max(right.position - left.position, kMinStopRange);
        const auto interpolation = std::clamp((position - left.position) / range, 0.0f, 1.0f);
        return QColor::fromRgbF(left.color.redF() + (right.color.redF() - left.color.redF()) * interpolation,
            left.color.greenF() + (right.color.greenF() - left.color.greenF()) * interpolation,
            left.color.blueF() + (right.color.blueF() - left.color.blueF()) * interpolation,
            left.color.alphaF() + (right.color.alphaF() - left.color.alphaF()) * interpolation);
    }
    return stops.back().color;
}
} // namespace

GradientTexture::~GradientTexture()
{
    delete texture_;
}

void GradientTexture::upload(QQuickWindow* window, const std::vector<GradientStopData>& stops)
{
    if (!window || stops.size() < 2 || matches(stops)) {
        return;
    }

    if (image_.size() != QSize{kGradientTextureWidth, 1}) {
        image_ = QImage(kGradientTextureWidth, 1, QImage::Format_RGBA8888);
        image_.setColorSpace(QColorSpace());
    }
    if (image_.isNull()) {
        return;
    }

    for (auto x = 0; x < kGradientTextureWidth; ++x) {
        const auto position = static_cast<float>(x) / static_cast<float>(kGradientTextureWidth - 1);
        image_.setPixelColor(x, 0, evaluateGradient(stops, position));
    }

    auto* replacement = window->createTextureFromImage(image_);
    if (!replacement) {
        return;
    }
    replacement->setFiltering(QSGTexture::Linear);
    replacement->setHorizontalWrapMode(QSGTexture::ClampToEdge);
    replacement->setVerticalWrapMode(QSGTexture::ClampToEdge);
    delete texture_;
    texture_ = replacement;
    stops_ = stops;
}

void GradientTexture::commit(QSGMaterialShader::RenderState& state, const int binding, QSGTexture** texture)
{
    if (!texture_) {
        *texture = nullptr;
        return;
    }
    auto* rhi = state.rhi();
    auto* updates = state.resourceUpdateBatch();
    if (!rhi || !updates) {
        *texture = nullptr;
        return;
    }
    texture_->commitTextureOperations(rhi, updates);
    *texture = texture_;
    Q_UNUSED(binding)
}

qint64 GradientTexture::comparisonKey() const
{
    return texture_ ? texture_->comparisonKey() : qint64{};
}

bool GradientTexture::matches(const std::vector<GradientStopData>& stops) const
{
    if (stops_.size() != stops.size()) {
        return false;
    }
    for (std::size_t index = 0; index < stops.size(); ++index) {
        if (std::abs(stops_[index].position - stops[index].position) > kMinStopRange || stops_[index].color != stops[index].color) {
            return false;
        }
    }
    return true;
}

} // namespace QAccelPlot
