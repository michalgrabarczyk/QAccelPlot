

# File LineCurveGaps.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**LineCurveGaps.hpp**](LineCurveGaps_8hpp.md)

[Go to the documentation of this file](LineCurveGaps_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

namespace NanGapModeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(NanGapMode)


enum class Mode {
    Break,   
    Connect, 
};
Q_ENUM_NS(Mode)
} // namespace NanGapModeNS

using NanGapMode = NanGapModeNS::Mode;

class LineCurveGaps : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(NanGapMode nanMode READ nanMode WRITE setNanMode NOTIFY nanModeChanged)

public:
    explicit LineCurveGaps(QObject* parent = nullptr);

    NanGapMode nanMode() const;
    void setNanMode(NanGapMode mode);

signals:
    void nanModeChanged();

private:
    NanGapMode nanMode_{NanGapMode::Break};
};

} // namespace QAccelPlot
```


