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

/// \brief QML singleton \c Histogram that creates Histogram values from JavaScript arrays.
///
/// \par Usage
/// \code
/// import QAccelPlot as QAccelPlot
///
/// QAccelPlot.BarSeries {
///     Component.onCompleted: setData(QAccelPlot.Histogram.fromSamples(samples, 40).bars())
/// }
/// \endcode
///
/// C++ code uses the static functions of Histogram instead.
///
/// \sa Histogram
class HistogramFactory : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Histogram)
    QML_SINGLETON

public:
    /// \brief Constructs the factory with the given \a parent.
    explicit HistogramFactory(QObject* parent = nullptr);

    /// \brief Counts \a samples into bins.
    ///
    /// \a bins is either a number of equal bins spanning the finite samples, or a list of
    /// finite, strictly increasing edges.
    Q_INVOKABLE ::QAccelPlot::Histogram fromSamples(const QList<qreal>& samples, const QVariant& bins) const;
    /// \brief Counts \a samples into \a binCount equal bins from \a min to \a max.
    Q_INVOKABLE ::QAccelPlot::Histogram fromSamples(const QList<qreal>& samples, int binCount, qreal min, qreal max) const;
    /// \brief Wraps data that is already binned: \a counts has one value per bin between \a edges.
    Q_INVOKABLE ::QAccelPlot::Histogram fromCounts(const QList<qreal>& edges, const QList<qreal>& counts) const;

    /// \brief Returns \a binCount + 1 equally spaced edges from \a min to \a max.
    Q_INVOKABLE QList<qreal> linearEdges(qreal min, qreal max, int binCount) const;
    /// \brief Returns \a binCount + 1 edges from \a min to \a max, equally spaced on a logarithmic axis.
    Q_INVOKABLE QList<qreal> logEdges(qreal min, qreal max, int binCount) const;
    /// \brief Returns the edges from \a min to \a max at 1, 2, ..., 9 times each power of ten: the ticks of a logarithmic axis.
    Q_INVOKABLE QList<qreal> decadeEdges(qreal min, qreal max) const;
};

} // namespace QAccelPlot
