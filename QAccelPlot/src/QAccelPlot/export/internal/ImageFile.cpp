//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/export/internal/ImageFile.hpp"

#include <QFileInfo>
#include <QImageWriter>
#include <QPainter>
#include <QSaveFile>
#include <QUrl>

namespace QAccelPlot::Internal {

namespace {

constexpr auto kMetersPerInch = 0.0254;

bool formatStoresAlpha(const QByteArray& format)
{
    static const auto opaqueFormats = QList<QByteArray>{"bmp", "jpeg", "jpg", "pbm", "pgm", "ppm", "wbmp", "xbm"};
    return !opaqueFormats.contains(format);
}

QImage flattenedOnWhite(const QImage& image)
{
    auto opaque = QImage(image.size(), QImage::Format_RGB32);
    opaque.setDotsPerMeterX(image.dotsPerMeterX());
    opaque.setDotsPerMeterY(image.dotsPerMeterY());
    opaque.fill(Qt::white);
    {
        auto painter = QPainter(&opaque);
        painter.drawImage(0, 0, image);
    }
    return opaque;
}

} // namespace

QString localFilePath(const QString& pathOrUrl)
{
    const auto url = QUrl(pathOrUrl);
    return url.isLocalFile() ? url.toLocalFile() : pathOrUrl;
}

QByteArray imageFormatForPath(const QString& path)
{
    const auto suffix = QFileInfo(path).suffix().toLower().toLatin1();
    return QImageWriter::supportedImageFormats().contains(suffix) ? suffix : QByteArray{};
}

void setImageResolution(QImage& image, const qreal dpi)
{
    const auto dotsPerMeter = qRound(dpi / kMetersPerInch);
    image.setDotsPerMeterX(dotsPerMeter);
    image.setDotsPerMeterY(dotsPerMeter);
}

QString writeImageFile(const QImage& image, const QString& path, const QByteArray& format, const int quality)
{
    auto file = QSaveFile(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return QStringLiteral("Cannot open %1: %2").arg(path, file.errorString());
    }
    auto writer = QImageWriter(&file, format);
    writer.setQuality(quality);
    const auto written = image.hasAlphaChannel() && !formatStoresAlpha(format) ? writer.write(flattenedOnWhite(image)) : writer.write(image);
    if (!written) {
        file.cancelWriting();
        return QStringLiteral("Cannot write %1: %2").arg(path, writer.errorString());
    }
    if (!file.commit()) {
        return QStringLiteral("Cannot save %1: %2").arg(path, file.errorString());
    }
    return {};
}

} // namespace QAccelPlot::Internal
