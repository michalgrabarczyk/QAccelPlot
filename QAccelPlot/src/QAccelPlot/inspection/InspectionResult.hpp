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
#include <QPointF>
#include <QVariantMap>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <limits>

namespace QAccelPlot {

/// \brief Namespace exposing the inspection \c Status enum to QML as \c Inspection.
namespace InspectionNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(Inspection)

/// \brief Outcome of an inspection query, or readiness of a series for queries.
enum class Status {
    Ready,           ///< \brief The query ran and matched, or the series can be queried.
    NoMatch,         ///< \brief The query ran and nothing matched.
    Idle,            ///< \brief A query index is required and has not been requested.
    Preparing,       ///< \brief A query index is being built; retry after \c statusChanged.
    Unsupported,     ///< \brief The series type does not support this query.
    Unavailable,     ///< \brief The query cannot run now: missing axes, empty geometry, or a pending data transition.
    InvalidArgument, ///< \brief An argument is nonfinite or out of range.
    Stale,           ///< \brief The expected data revision no longer matches the series.
};
Q_ENUM_NS(Status)
} // namespace InspectionNS

using InspectionStatus = InspectionNS::Status;

/// \brief One XY source sample returned by an inspection query.
struct InspectionSample {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief Query outcome.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    /// \brief True when status is \c Ready.
    Q_PROPERTY(bool valid READ valid CONSTANT)
    /// \brief Data revision the query ran against.
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    /// \brief Source index, counting invalid records; -1 when nothing matched.
    Q_PROPERTY(int index MEMBER index CONSTANT)
    /// \brief Data-space X coordinate.
    Q_PROPERTY(qreal x READ x CONSTANT)
    /// \brief Data-space Y coordinate.
    Q_PROPERTY(qreal y READ y CONSTANT)
    /// \brief Optional per-point scalar; NaN when the series has none.
    Q_PROPERTY(qreal value MEMBER value CONSTANT)
    /// \brief Series-local position in logical pixels.
    Q_PROPERTY(QPointF pixelPosition MEMBER pixelPosition CONSTANT)
    /// \brief Distance from the query position in logical pixels.
    Q_PROPERTY(qreal distance MEMBER distance CONSTANT)
    /// \brief True when the coordinates are interpolated between two samples; index is then the left neighbor.
    Q_PROPERTY(bool interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch};                                                      ///< \brief Query outcome.
    quint64 dataRevision{0};                                                                                 ///< \brief Data revision.
    int index{-1};                                                                                           ///< \brief Source index.
    QPointF position{std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN()};      ///< \brief Data-space XY.
    QPointF pixelPosition{std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN()}; ///< \brief Series-local pixels.
    qreal distance{std::numeric_limits<qreal>::quiet_NaN()};                                                 ///< \brief Pixel distance.
    qreal value{std::numeric_limits<qreal>::quiet_NaN()};                                                    ///< \brief Optional scalar.
    bool interpolated{false};                                                                                ///< \brief Interpolated point.

    /// \brief Returns true when status is \c Ready.
    bool valid() const;
    /// \brief Returns the data-space X coordinate.
    qreal x() const;
    /// \brief Returns the data-space Y coordinate.
    qreal y() const;
};

/// \brief Sample-weighted Y statistics over the valid samples inside a region.
struct InspectionSummary {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief Query outcome; \c NoMatch for an empty region.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    /// \brief True when status is \c Ready.
    Q_PROPERTY(bool valid READ valid CONSTANT)
    /// \brief Data revision the query ran against.
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    /// \brief Number of matching samples.
    Q_PROPERTY(int count MEMBER count CONSTANT)
    /// \brief Smallest Y; NaN for an empty region.
    Q_PROPERTY(qreal minimum MEMBER minimum CONSTANT)
    /// \brief Largest Y; NaN for an empty region.
    Q_PROPERTY(qreal maximum MEMBER maximum CONSTANT)
    /// \brief Source index of the smallest Y, highest index on ties.
    Q_PROPERTY(int minimumIndex MEMBER minimumIndex CONSTANT)
    /// \brief Source index of the largest Y, highest index on ties.
    Q_PROPERTY(int maximumIndex MEMBER maximumIndex CONSTANT)
    /// \brief Mean Y; NaN for an empty region.
    Q_PROPERTY(qreal mean MEMBER mean CONSTANT)
    /// \brief Population standard deviation of Y; NaN for an empty region.
    Q_PROPERTY(qreal standardDeviation MEMBER standardDeviation CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch};               ///< \brief Query outcome.
    quint64 dataRevision{0};                                          ///< \brief Data revision.
    int count{0};                                                     ///< \brief Matching samples.
    qreal minimum{std::numeric_limits<qreal>::quiet_NaN()};           ///< \brief Smallest Y.
    qreal maximum{std::numeric_limits<qreal>::quiet_NaN()};           ///< \brief Largest Y.
    int minimumIndex{-1};                                             ///< \brief Index of the smallest Y.
    int maximumIndex{-1};                                             ///< \brief Index of the largest Y.
    qreal mean{std::numeric_limits<qreal>::quiet_NaN()};              ///< \brief Mean Y.
    qreal standardDeviation{std::numeric_limits<qreal>::quiet_NaN()}; ///< \brief Population standard deviation.

    /// \brief Returns true when status is \c Ready.
    bool valid() const;
};

/// \brief The valid samples on either side of a position on one axis.
struct InspectionBracket {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief \c Ready when at least one neighbor exists.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    /// \brief True when status is \c Ready.
    Q_PROPERTY(bool valid READ valid CONSTANT)
    /// \brief Data revision the query ran against.
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    /// \brief Valid sample with the largest coordinate at or below the position; invalid when there is none.
    Q_PROPERTY(::QAccelPlot::InspectionSample left MEMBER left CONSTANT)
    /// \brief Valid sample with the smallest coordinate above the position; invalid when there is none.
    Q_PROPERTY(::QAccelPlot::InspectionSample right MEMBER right CONSTANT)
    /// \brief True when both neighbors exist and their source indices are consecutive.
    Q_PROPERTY(bool adjacent MEMBER adjacent CONSTANT)
    /// \brief Point on the straight on-screen segment between adjacent neighbors; invalid otherwise.
    Q_PROPERTY(::QAccelPlot::InspectionSample interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; ///< \brief Query outcome.
    quint64 dataRevision{0};                            ///< \brief Data revision.
    InspectionSample left;                              ///< \brief Neighbor at or before the position.
    InspectionSample right;                             ///< \brief Neighbor after the position.
    bool adjacent{false};                               ///< \brief Consecutive source indices.
    InspectionSample interpolated;                      ///< \brief Interpolated point between adjacent neighbors.

    /// \brief Returns true when status is \c Ready.
    bool valid() const;
};

/// \brief One page of source indices inside a region.
struct InspectionPage {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief Query outcome; \c Ready also for an empty page of a matching region.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    /// \brief True when status is \c Ready.
    Q_PROPERTY(bool valid READ valid CONSTANT)
    /// \brief Data revision the indices refer to.
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    /// \brief Number of matching samples in the whole region.
    Q_PROPERTY(int total MEMBER total CONSTANT)
    /// \brief Number of matches skipped before this page.
    Q_PROPERTY(int offset MEMBER offset CONSTANT)
    /// \brief Effective page size after clamping to the maximum.
    Q_PROPERTY(int limit MEMBER limit CONSTANT)
    /// \brief True when matches remain after this page.
    Q_PROPERTY(bool hasMore MEMBER hasMore CONSTANT)
    /// \brief True when pages are in ascending source order; false for index traversal order.
    Q_PROPERTY(bool sourceOrder MEMBER sourceOrder CONSTANT)
    /// \brief Matching source indices.
    Q_PROPERTY(QList<int> indices MEMBER indices CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; ///< \brief Query outcome.
    quint64 dataRevision{0};                            ///< \brief Data revision.
    int total{0};                                       ///< \brief Matches in the region.
    int offset{0};                                      ///< \brief Skipped matches.
    int limit{0};                                       ///< \brief Effective page size.
    bool hasMore{false};                                ///< \brief More pages follow.
    bool sourceOrder{false};                            ///< \brief Ascending source order.
    QList<int> indices;                                 ///< \brief Matching source indices.

    /// \brief Returns true when status is \c Ready.
    bool valid() const;
};

/// \brief A native record of a series that is not a plain XY series, such as a bar, rectangle, or band.
struct InspectionRecord {
    Q_GADGET
    QML_ANONYMOUS
    /// \brief Query outcome.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    /// \brief True when status is \c Ready.
    Q_PROPERTY(bool valid READ valid CONSTANT)
    /// \brief Data revision the query ran against.
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    /// \brief Source index; -1 for an interpolated hit.
    Q_PROPERTY(int index MEMBER index CONSTANT)
    /// \brief True when the fields are interpolated between source records.
    Q_PROPERTY(bool interpolated MEMBER interpolated CONSTANT)
    /// \brief Record values keyed by the series' own field names.
    Q_PROPERTY(QVariantMap fields MEMBER fields CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; ///< \brief Query outcome.
    quint64 dataRevision{0};                            ///< \brief Data revision.
    int index{-1};                                      ///< \brief Source index.
    bool interpolated{false};                           ///< \brief Interpolated fields.
    QVariantMap fields;                                 ///< \brief Record values.

    /// \brief Returns true when status is \c Ready.
    bool valid() const;
};

} // namespace QAccelPlot
