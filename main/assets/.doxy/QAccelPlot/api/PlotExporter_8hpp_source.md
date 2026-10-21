

# File PlotExporter.hpp

[**File List**](files.md) **>** [**export**](dir_165621f1d3f543afcb149fc22fafadb8.md) **>** [**PlotExporter.hpp**](PlotExporter_8hpp.md)

[Go to the documentation of this file](PlotExporter_8hpp.md)


```C++
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

class PlotExporter : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotExporter)
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(qreal dpi READ dpi WRITE setDpi NOTIFY dpiChanged)
    Q_PROPERTY(bool transparentBackground READ transparentBackground WRITE setTransparentBackground NOTIFY transparentBackgroundChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit PlotExporter(QObject* parent = nullptr);
    ~PlotExporter() override;

    PlotExporter(const PlotExporter&) = delete;
    PlotExporter& operator=(const PlotExporter&) = delete;
    PlotExporter(PlotExporter&&) = delete;
    PlotExporter& operator=(PlotExporter&&) = delete;

    ::QAccelPlot::QAccelPlot* plot() const;
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    qreal scale() const;
    void setScale(qreal scale);
    int quality() const;
    void setQuality(int quality);
    qreal dpi() const;
    void setDpi(qreal dpi);
    bool transparentBackground() const;
    void setTransparentBackground(bool transparent);
    bool busy() const;

    Q_INVOKABLE bool save(const QString& path);
    Q_INVOKABLE bool copyToClipboard();
    Q_INVOKABLE bool grabImage();
    Q_INVOKABLE static QStringList supportedFormats();

signals:
    void plotChanged();
    void scaleChanged();
    void qualityChanged();
    void dpiChanged();
    void transparentBackgroundChanged();
    void busyChanged();
    void finished(bool ok, const QString& path, const QString& error);
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
```


