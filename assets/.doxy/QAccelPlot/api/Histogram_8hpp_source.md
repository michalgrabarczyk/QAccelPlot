

# File Histogram.hpp

[**File List**](files.md) **>** [**data**](dir_d2fb866f403cfee1d904f16ed73cc0a7.md) **>** [**Histogram.hpp**](Histogram_8hpp.md)

[Go to the documentation of this file](Histogram_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QList>
#include <QObject>
#include <QVariantList>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <cstddef>
#include <vector>

namespace QAccelPlot {

class Histogram {
    Q_GADGET
    QML_ANONYMOUS

    Q_PROPERTY(int binCount READ binCount CONSTANT)
    Q_PROPERTY(QList<qreal> edges READ edgeList CONSTANT)
    Q_PROPERTY(QList<qreal> values READ valueList CONSTANT)
    Q_PROPERTY(qreal total READ total CONSTANT)
    Q_PROPERTY(bool uniform READ isUniform CONSTANT)
    Q_PROPERTY(qreal binWidth READ binWidth CONSTANT)

public:
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, int binCount);
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, int binCount, double min, double max);
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, std::vector<double> edges);
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, int binCount);
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, int binCount, double min, double max);
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, std::vector<double> edges);
    static Histogram fromCounts(std::vector<double> edges, std::vector<double> counts);

    static std::vector<double> linearEdges(double min, double max, int binCount);
    static std::vector<double> logEdges(double min, double max, int binCount);
    static std::vector<double> decadeEdges(double min, double max);

    int binCount() const;
    const std::vector<double>& edges() const;
    const std::vector<double>& values() const;
    double total() const;
    bool isUniform() const;
    double binWidth() const;

    Q_INVOKABLE ::QAccelPlot::Histogram density() const;

    std::vector<double> barData() const;
    Q_INVOKABLE QVariantList bars() const;

private:
    template <typename Sample> static Histogram spanning(const Sample* samples, std::size_t sampleCount, int binCount);
    template <typename Sample> static Histogram binned(const Sample* samples, std::size_t sampleCount, std::vector<double> edges);

    QList<qreal> edgeList() const;
    QList<qreal> valueList() const;

    std::vector<double> edges_;
    std::vector<double> values_;
    double total_{0.0};
    bool uniform_{false};
    bool density_{false};
};

} // namespace QAccelPlot
```


