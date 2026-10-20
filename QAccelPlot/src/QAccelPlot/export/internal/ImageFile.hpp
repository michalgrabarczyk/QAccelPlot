//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QByteArray>
#include <QImage>
#include <QString>

namespace QAccelPlot::Internal {

/// \brief Returns \a pathOrUrl as a local file path; a \c file: URL, as a QML file dialog returns, is converted.
QString localFilePath(const QString& pathOrUrl);

/// \brief Returns the lowercase \c QImageWriter format for \a path's suffix, or an empty array when none can write it.
QByteArray imageFormatForPath(const QString& path);

/// \brief Sets the resolution stored with \a image to \a dpi dots per inch.
void setImageResolution(QImage& image, qreal dpi);

/// \brief Writes \a image to \a path in \a format and returns an error message, or an empty string on success.
///
/// \a quality is passed to \c QImageWriter::setQuality(). Formats without an alpha channel get
/// the image composited over white. The file is replaced atomically, so a failed write leaves an
/// existing file intact. Safe to call from a worker thread.
QString writeImageFile(const QImage& image, const QString& path, const QByteArray& format, int quality);

} // namespace QAccelPlot::Internal
