//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/inspection/internal/InspectionIndex.hpp"

#include <QObject>
#include <QTimer>

#include <memory>

namespace QAccelPlot {

/// \brief Builds an \c InspectionIndex on a worker thread from a snapshot of a series' records.
class InspectionCache : public QObject {
public:
    /// \brief Series-side callbacks; all are invoked on the series' thread.
    class Host {
    public:
        virtual ~Host();
        virtual InspectionSource indexSource() const = 0;
        virtual bool indexSourceAvailable() const = 0;
        virtual void indexReady() = 0;
    };

    explicit InspectionCache(Host& host);
    ~InspectionCache() override;

    /// \brief Drops the index, the request for one, and pending work.
    void invalidate();
    /// \brief Starts building an index unless one is ready or already in progress.
    void request();
    [[nodiscard]] bool requested() const;
    /// \brief Returns the index for the current records, or null while none is ready.
    [[nodiscard]] const InspectionIndex* index() const;

private:
    struct Job {
        std::atomic_bool cancelled{false};
        std::atomic_bool done{false};
        quint64 generation{0};
        std::unique_ptr<InspectionIndex> index;
    };

    void tick();
    void launch();

    Host& host_;
    QTimer timer_;
    quint64 generation_{0};
    bool requested_{false};
    std::shared_ptr<Job> job_;
    std::unique_ptr<InspectionIndex> index_;
};

} // namespace QAccelPlot
