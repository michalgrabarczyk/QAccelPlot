//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/QAccelPlot.hpp"

#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <memory>

namespace QAccelPlot {

/// \brief Exports a plot as a raster image: to a file, to the clipboard, or as a \c QImage.
///
/// The capture contains the plot item and everything drawn inside its bounds, as on screen:
/// series, axes, legend, overlays, and any child items. Children outside the bounds are clipped.
/// The plot must be visible in a window that is shown, not minimized, and renders through a
/// hardware scene graph backend.
///
/// Every request is asynchronous and ends with exactly one \c finished() signal. A request made
/// while \c busy, or while another exporter captures the same plot, is rejected. The capture is taken on the next frame, and fails when the window
/// is hidden, minimized, or destroyed first; encoding and writing a file run on a worker thread.
///
/// \par Usage
/// \code
/// PlotExporter {
///     id: exporter
///     plot: myPlot
///     scale: 2
///     onFinished: (ok, path, error) => console.log(ok ? "Saved " + path : error)
/// }
/// Button { text: "Save"; onClicked: exporter.save("plot.png") }
/// \endcode
///
/// \sa PlotView
class PlotExporter : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotExporter)
    /// \brief Plot to export.
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    /// \brief Image pixels per logical pixel; 0 uses the window's device pixel ratio. Default: 0.
    ///
    /// At a device pixel ratio other than 1, Qt 6.3 and later cannot render every image size, so
    /// a side can be a pixel shorter or longer than the plot's size times the scale.
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    /// \brief Encoder quality from 0 to 100 passed to \c QImageWriter; -1 uses the format's default. Default: -1.
    ///
    /// Lossy formats (JPEG, WebP) trade file size for artifacts. PNG stays lossless and only
    /// trades file size for encoding time. Other formats ignore it.
    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)
    /// \brief Resolution stored in saved files, in dots per inch; 0 stores 96 times the effective scale. Default: 0.
    ///
    /// The default makes documents place the image at its on-screen size.
    Q_PROPERTY(qreal dpi READ dpi WRITE setDpi NOTIFY dpiChanged)
    /// \brief Leaves out the plot's \c axesAreaColor and \c plotAreaColor backgrounds. Default: false.
    ///
    /// Grid, series, axes, and child items such as the legend are drawn as usual. Formats without
    /// an alpha channel, such as JPEG, composite the image over white. The plot shows no
    /// background for the frame it is captured in.
    Q_PROPERTY(bool transparentBackground READ transparentBackground WRITE setTransparentBackground NOTIFY transparentBackgroundChanged)
    /// \brief True from a request until its \c finished() signal.
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    /// \brief Constructs an exporter without a plot.
    explicit PlotExporter(QObject* parent = nullptr);
    /// \brief Restores the plot's appearance if a capture is in progress.
    ~PlotExporter() override;

    PlotExporter(const PlotExporter&) = delete;
    PlotExporter& operator=(const PlotExporter&) = delete;
    PlotExporter(PlotExporter&&) = delete;
    PlotExporter& operator=(PlotExporter&&) = delete;

    /// \brief Returns the plot to export.
    ::QAccelPlot::QAccelPlot* plot() const;
    /// \brief Sets the plot to export; a request in progress keeps its plot.
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    /// \brief Returns the image pixels per logical pixel; 0 means the window's device pixel ratio.
    qreal scale() const;
    /// \brief Sets the image pixels per logical pixel; negative and nonfinite values are ignored with a warning.
    void setScale(qreal scale);
    /// \brief Returns the encoder quality.
    int quality() const;
    /// \brief Sets the encoder quality; values outside -1 to 100 are ignored with a warning.
    void setQuality(int quality);
    /// \brief Returns the resolution stored in saved files; 0 means 96 times the effective scale.
    qreal dpi() const;
    /// \brief Sets the stored resolution; negative and nonfinite values are ignored with a warning.
    void setDpi(qreal dpi);
    /// \brief Returns whether the image leaves out the plot's backgrounds.
    bool transparentBackground() const;
    /// \brief Sets whether the image leaves out the plot's backgrounds.
    void setTransparentBackground(bool transparent);
    /// \brief Returns true while a request is in progress.
    bool busy() const;

    /// \brief Saves the plot to \a path in the format its suffix names.
    ///
    /// \a path may be a local file path or a \c file: URL. An existing file is replaced only
    /// when the new one is written completely. Returns false, after emitting \c finished(), when
    /// the request fails immediately.
    Q_INVOKABLE bool save(const QString& path);
    /// \brief Copies the plot image to the clipboard; returns false when the request fails immediately.
    Q_INVOKABLE bool copyToClipboard();
    /// \brief Captures the plot and emits \c imageReady(); returns false when the request fails immediately.
    Q_INVOKABLE bool grabImage();
    /// \brief Returns the lowercase file suffixes \c save() accepts on this system, such as \c png and \c jpg.
    Q_INVOKABLE static QStringList supportedFormats();

signals:
    /// \brief Emitted when the plot property changes.
    void plotChanged();
    /// \brief Emitted when the scale property changes.
    void scaleChanged();
    /// \brief Emitted when the quality property changes.
    void qualityChanged();
    /// \brief Emitted when the dpi property changes.
    void dpiChanged();
    /// \brief Emitted when the transparentBackground property changes.
    void transparentBackgroundChanged();
    /// \brief Emitted when the busy property changes.
    void busyChanged();
    /// \brief Emitted when a request ends; \a path is the saved file, empty for other requests.
    ///
    /// \a ok is false when the request failed, and \a error describes why.
    void finished(bool ok, const QString& path, const QString& error);
    /// \brief Emitted with the captured \a image before \c finished() for a \c grabImage() request.
    void imageReady(const QImage& image);

private:
    enum class Operation { Save, CopyToClipboard, Grab };
    struct Capture;

    bool startCapture(Operation operation, const QString& path, const QByteArray& format);
    void watchCapture();
    QString captureError(qreal scale) const;
    qreal effectiveScale() const;
    void captured(QImage image);
    void abortCapture(const QString& error);
    void endCapture();
    void saveInBackground(const QImage& image, const QString& path, const QByteArray& format, int quality);
    bool fail(const QString& path, const QString& error);
    void finish(bool ok, const QString& path, const QString& error);
    void setBusy(bool busy);

    QPointer<::QAccelPlot::QAccelPlot> plot_;
    qreal scale_{0.0};
    int quality_{-1};
    qreal dpi_{0.0};
    bool transparentBackground_{false};
    bool busy_{false};
    std::unique_ptr<Capture> capture_;
    QFutureWatcher<QString> saveWatcher_;
    QString savePath_;
};

} // namespace QAccelPlot
