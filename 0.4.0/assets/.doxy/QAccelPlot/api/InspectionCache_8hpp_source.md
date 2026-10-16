

# File InspectionCache.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**InspectionCache.hpp**](InspectionCache_8hpp.md)

[Go to the documentation of this file](InspectionCache_8hpp.md)


```C++
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

class InspectionCache : public QObject {
public:
    class Host {
    public:
        virtual ~Host();
        virtual InspectionSource indexSource() const = 0;
        virtual bool indexSourceAvailable() const = 0;
        virtual void indexReady() = 0;
    };

    explicit InspectionCache(Host& host);
    ~InspectionCache() override;

    void invalidate();
    void request();
    [[nodiscard]] bool requested() const;
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
```


