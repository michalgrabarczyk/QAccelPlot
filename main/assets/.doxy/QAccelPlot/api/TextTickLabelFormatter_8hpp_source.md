

# File TextTickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_d8921b289ee138c4c21e0a56c5dc2c98.md) **>** [**TextTickLabelFormatter.hpp**](TextTickLabelFormatter_8hpp.md)

[Go to the documentation of this file](TextTickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "formatters/TickLabelFormatter.hpp"

#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class TextTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(TextTickLabelFormatter)

    
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged)

public:
    explicit TextTickLabelFormatter(QObject* parent = nullptr);

    QStringList labels() const;
    void setLabels(const QStringList& labels);

signals:
    void labelsChanged();

protected:
    QString doFormat(qreal value, qreal tickStep) const override;

private:
    QStringList labels_;
};

} // namespace QAccelPlot
```


