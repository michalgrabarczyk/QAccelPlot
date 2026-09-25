

# File DateTimeTickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**DateTimeTickLabelFormatter.hpp**](DateTimeTickLabelFormatter_8hpp.md)

[Go to the documentation of this file](DateTimeTickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class DateTimeTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(DateTimeTickLabelFormatter)

    
    Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat WRITE setDateTimeFormat NOTIFY dateTimeFormatChanged)

public:
    explicit DateTimeTickLabelFormatter(QObject* parent = nullptr);

    QString dateTimeFormat() const;
    void setDateTimeFormat(const QString& format);

signals:
    void dateTimeFormatChanged();

protected:
    QString doFormat(qreal value, qreal tickStep) const override;

private:
    QString dateTimeFormat_{"yyyy-MM-dd HH:mm:ss"};
};

} // namespace QAccelPlot
```


