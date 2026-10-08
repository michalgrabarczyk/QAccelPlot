

# File HistogramFactory.hpp

[**File List**](files.md) **>** [**data**](dir_d2fb866f403cfee1d904f16ed73cc0a7.md) **>** [**HistogramFactory.hpp**](HistogramFactory_8hpp.md)

[Go to the documentation of this file](HistogramFactory_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/data/Histogram.hpp"

#include <QList>
#include <QObject>
#include <QVariant>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
// QML_SINGLETON needs QQmlPrivate declared, which qqmlregistration.h alone does not pull in
// on Qt versions before the standalone QtQmlIntegration module (introduced in Qt 6.5).
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class HistogramFactory : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Histogram)
    QML_SINGLETON

public:
    explicit HistogramFactory(QObject* parent = nullptr);

    Q_INVOKABLE ::QAccelPlot::Histogram fromSamples(const QList<qreal>& samples, const QVariant& bins) const;
    Q_INVOKABLE ::QAccelPlot::Histogram fromSamples(const QList<qreal>& samples, int binCount, qreal min, qreal max) const;
    Q_INVOKABLE ::QAccelPlot::Histogram fromCounts(const QList<qreal>& edges, const QList<qreal>& counts) const;

    Q_INVOKABLE QList<qreal> linearEdges(qreal min, qreal max, int binCount) const;
    Q_INVOKABLE QList<qreal> logEdges(qreal min, qreal max, int binCount) const;
    Q_INVOKABLE QList<qreal> decadeEdges(qreal min, qreal max) const;
};

} // namespace QAccelPlot
```


