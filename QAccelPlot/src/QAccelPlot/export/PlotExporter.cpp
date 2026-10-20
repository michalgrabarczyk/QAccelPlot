//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/export/PlotExporter.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/export/internal/ImageFile.hpp"
#include "QAccelPlot/export/internal/ScopedPaintedItemTextures.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QImageWriter>
#include <QPromise>
#include <QQuickItemGrabResult>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QRunnable>
#include <QThreadPool>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kLogicalDpi = 96.0;
constexpr auto kMinQuality = -1;
constexpr auto kMaxQuality = 100;

bool isValidFactor(const qreal value)
{
    return std::isfinite(value) && value >= 0.0;
}

QSize imagePixelSize(const QQuickItem* item, const qreal scale)
{
    return QSize(std::max(1, qRound(item->width() * scale)), std::max(1, qRound(item->height() * scale)));
}

// Qt 6.2 grabs targetSize physical pixels; later versions multiply targetSize by the device pixel ratio.
QSize grabTargetSize(const QSize& pixels, const qreal devicePixelRatio)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    Q_UNUSED(devicePixelRatio)
    return pixels;
#else
    return QSize(std::max(1, qRound(pixels.width() / devicePixelRatio)), std::max(1, qRound(pixels.height() / devicePixelRatio)));
#endif
}

QWindow* shownWindow(QQuickWindow* window)
{
    auto* renderWindow = QQuickRenderControl::renderWindowFor(window);
    return renderWindow ? renderWindow : window;
}

bool isHidden(const QWindow::Visibility visibility)
{
    return visibility == QWindow::Hidden || visibility == QWindow::Minimized;
}

// A hidden, minimized, or otherwise unexposed window renders no frames, so a grab in it would never complete.
// Exposure follows hiding only once the platform reports it, so the visibility is checked as well.
bool isShown(QQuickWindow* window)
{
    const auto* shown = shownWindow(window);
    return !isHidden(shown->visibility()) && shown->isExposed();
}

// Deletes \a grab once Qt no longer uses it: the render thread runs an unfinished grab when its window renders next.
void releaseGrab(const QSharedPointer<QQuickItemGrabResult>& grab, const QObject* window, const bool finished)
{
    if (!grab) {
        return;
    }
    auto* const owner = new QObject;
    QObject::connect(owner, &QObject::destroyed, [grab]() { });
    if (finished || !window) {
        owner->deleteLater();
        return;
    }
    QObject::connect(grab.data(), &QQuickItemGrabResult::ready, owner, &QObject::deleteLater);
    QObject::connect(window, &QObject::destroyed, owner, &QObject::deleteLater);
}

} // namespace

// Options are copied when the request starts, so property changes do not affect it.
struct PlotExporter::Capture {
    Capture(const Operation operation, QString path, QByteArray format, const int quality, const qreal dpi, ::QAccelPlot::QAccelPlot* plot, const qreal scale)
        : operation(operation)
        , path(std::move(path))
        , format(std::move(format))
        , quality(quality)
        , dpi(dpi)
        , plot(plot)
        , window(plot->window())
        , textures(plot, scale, plot->window()->effectiveDevicePixelRatio())
    {
    }

    Operation operation;
    QString path;
    QByteArray format;
    int quality;
    qreal dpi;
    QPointer<::QAccelPlot::QAccelPlot> plot;
    // A QWindow: hiding it from ~QWindow can end the capture after the QQuickWindow part is gone.
    QPointer<QWindow> window;
    Internal::ScopedPaintedItemTextures textures;
    QSharedPointer<QQuickItemGrabResult> grab;
    bool grabFinished{false};
    // Context of the connections that end the capture; destroying it disconnects them.
    QObject connections;
};

PlotExporter::PlotExporter(QObject* parent)
    : QObject(parent)
{
    connect(&saveWatcher_, &QFutureWatcherBase::finished, this, [this]() {
        const auto error = saveWatcher_.result();
        finish(error.isEmpty(), savePath_, error);
    });
}

PlotExporter::~PlotExporter()
{
    endCapture();
}

::QAccelPlot::QAccelPlot* PlotExporter::plot() const
{
    return plot_;
}

void PlotExporter::setPlot(::QAccelPlot::QAccelPlot* plot)
{
    if (plot_ == plot) {
        return;
    }
    plot_ = plot;
    emit plotChanged();
}

qreal PlotExporter::scale() const
{
    return scale_;
}

void PlotExporter::setScale(const qreal scale)
{
    if (!isValidFactor(scale)) {
        qCWarning(lcQAccelPlot) << "PlotExporter: ignoring scale" << scale << "; it must be 0 or positive.";
        return;
    }
    if (scale_ == scale) {
        return;
    }
    scale_ = scale;
    emit scaleChanged();
}

int PlotExporter::quality() const
{
    return quality_;
}

void PlotExporter::setQuality(const int quality)
{
    if (quality < kMinQuality || quality > kMaxQuality) {
        qCWarning(lcQAccelPlot) << "PlotExporter: ignoring quality" << quality << "; it must be from -1 to 100.";
        return;
    }
    if (quality_ == quality) {
        return;
    }
    quality_ = quality;
    emit qualityChanged();
}

qreal PlotExporter::dpi() const
{
    return dpi_;
}

void PlotExporter::setDpi(const qreal dpi)
{
    if (!isValidFactor(dpi)) {
        qCWarning(lcQAccelPlot) << "PlotExporter: ignoring dpi" << dpi << "; it must be 0 or positive.";
        return;
    }
    if (dpi_ == dpi) {
        return;
    }
    dpi_ = dpi;
    emit dpiChanged();
}

bool PlotExporter::transparentBackground() const
{
    return transparentBackground_;
}

void PlotExporter::setTransparentBackground(const bool transparent)
{
    if (transparentBackground_ == transparent) {
        return;
    }
    transparentBackground_ = transparent;
    emit transparentBackgroundChanged();
}

bool PlotExporter::busy() const
{
    return busy_;
}

bool PlotExporter::save(const QString& path)
{
    const auto filePath = Internal::localFilePath(path);
    const auto format = Internal::imageFormatForPath(filePath);
    if (format.isEmpty()) {
        return fail(filePath, QStringLiteral("Cannot save %1: unsupported image format.").arg(filePath));
    }
    return startCapture(Operation::Save, filePath, format);
}

bool PlotExporter::copyToClipboard()
{
    return startCapture(Operation::CopyToClipboard, {}, {});
}

bool PlotExporter::grabImage()
{
    return startCapture(Operation::Grab, {}, {});
}

QStringList PlotExporter::supportedFormats()
{
    auto formats = QStringList{};
    for (const auto& format : QImageWriter::supportedImageFormats()) {
        formats.append(QString::fromLatin1(format));
    }
    return formats;
}

bool PlotExporter::startCapture(const Operation operation, const QString& path, const QByteArray& format)
{
    if (busy_) {
        return fail(path, QStringLiteral("Another export is in progress."));
    }
    const auto scale = effectiveScale();
    if (const auto error = captureError(scale); !error.isEmpty()) {
        return fail(path, error);
    }

    const auto dpi = dpi_ > 0.0 ? dpi_ : kLogicalDpi * scale;
    capture_ = std::make_unique<Capture>(operation, path, format, quality_, dpi, plot_.data(), scale);
    plot_->beginExport(transparentBackground_);
    capture_->grab = plot_->grabToImage(grabTargetSize(imagePixelSize(plot_, scale), plot_->window()->effectiveDevicePixelRatio()));
    if (!capture_->grab) {
        endCapture();
        return fail(path, QStringLiteral("Qt Quick could not start the capture."));
    }
    setBusy(true);
    watchCapture();
    return true;
}

void PlotExporter::watchCapture()
{
    auto* const context = &capture_->connections;
    connect(capture_->grab.data(), &QQuickItemGrabResult::ready, context, [this]() {
        capture_->grabFinished = true;
        captured(capture_->grab->image());
    });
    connect(shownWindow(plot_->window()), &QWindow::visibilityChanged, context, [this](const QWindow::Visibility visibility) {
        if (isHidden(visibility)) {
            abortCapture(QStringLiteral("The plot's window was hidden before the capture."));
        }
    });
    connect(capture_->window, &QObject::destroyed, context, [this]() { abortCapture(QStringLiteral("The plot's window was destroyed before the capture.")); });
}

QString PlotExporter::captureError(const qreal scale) const
{
    if (!plot_) {
        return QStringLiteral("No plot is set.");
    }
    if (plot_->exporting_) {
        return QStringLiteral("Another export of this plot is in progress.");
    }
    auto* window = plot_->window();
    if (!window) {
        return QStringLiteral("The plot is not in a window.");
    }
    if (!isShown(window) || !plot_->isVisible()) {
        return QStringLiteral("The plot is not visible.");
    }
    if (plot_->width() <= 0.0 || plot_->height() <= 0.0) {
        return QStringLiteral("The plot has no size.");
    }
    if (!Internal::supportsCustomShaderRendering(window)) {
        return QStringLiteral("Exporting requires a hardware scene graph backend.");
    }
    const auto pixels = imagePixelSize(plot_, scale);
    const auto limit = Internal::maxTextureSize(window);
    if (pixels.width() > limit || pixels.height() > limit) {
        return QStringLiteral("The image size %1x%2 exceeds the GPU texture limit of %3 pixels.").arg(pixels.width()).arg(pixels.height()).arg(limit);
    }
    return {};
}

qreal PlotExporter::effectiveScale() const
{
    if (scale_ > 0.0) {
        return scale_;
    }
    const auto* window = plot_ ? plot_->window() : nullptr;
    return window ? window->effectiveDevicePixelRatio() : 1.0;
}

void PlotExporter::captured(QImage image)
{
    const auto operation = capture_->operation;
    const auto path = capture_->path;
    const auto format = capture_->format;
    const auto quality = capture_->quality;
    const auto dpi = capture_->dpi;
    endCapture();

    if (image.isNull()) {
        finish(false, path, QStringLiteral("The capture returned an empty image."));
        return;
    }
    image.setDevicePixelRatio(1.0);
    Internal::setImageResolution(image, dpi);
    switch (operation) {
    case Operation::Save:
        saveInBackground(image, path, format, quality);
        break;
    case Operation::CopyToClipboard:
        QGuiApplication::clipboard()->setImage(image);
        finish(true, {}, {});
        break;
    case Operation::Grab:
        emit imageReady(image);
        finish(true, {}, {});
        break;
    }
}

void PlotExporter::abortCapture(const QString& error)
{
    const auto path = capture_->path;
    endCapture();
    finish(false, path, error);
}

void PlotExporter::endCapture()
{
    if (!capture_) {
        return;
    }
    if (capture_->plot) {
        capture_->plot->endExport();
    }
    releaseGrab(capture_->grab, capture_->window, capture_->grabFinished);
    capture_.reset();
}

void PlotExporter::saveInBackground(const QImage& image, const QString& path, const QByteArray& format, const int quality)
{
    savePath_ = path;
    auto promise = std::make_shared<QPromise<QString>>();
    saveWatcher_.setFuture(promise->future());
    promise->start();
    QThreadPool::globalInstance()->start(QRunnable::create([promise, image, path, format, quality]() {
        promise->addResult(Internal::writeImageFile(image, path, format, quality));
        promise->finish();
    }));
}

bool PlotExporter::fail(const QString& path, const QString& error)
{
    qCWarning(lcQAccelPlot).noquote() << "PlotExporter:" << error;
    emit finished(false, path, error);
    return false;
}

void PlotExporter::finish(const bool ok, const QString& path, const QString& error)
{
    if (!ok) {
        qCWarning(lcQAccelPlot).noquote() << "PlotExporter:" << error;
    }
    setBusy(false);
    emit finished(ok, path, error);
}

void PlotExporter::setBusy(const bool busy)
{
    if (busy_ == busy) {
        return;
    }
    busy_ = busy;
    emit busyChanged();
}

} // namespace QAccelPlot
