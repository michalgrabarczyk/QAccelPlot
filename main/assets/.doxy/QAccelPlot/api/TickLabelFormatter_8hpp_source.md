

# File TickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**TickLabelFormatter.hpp**](TickLabelFormatter_8hpp.md)

[Go to the documentation of this file](TickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QJSValue>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

struct TickSet {
    QList<qreal> values;    
    qreal viewportMin{0.0}; 
    qreal viewportMax{0.0}; 
    qreal step{0.0};        
    bool logScale{false};   
};

struct TickSetFormat {
    int prefixExponent{0}; 
};

struct TickSetLabels {
    QStringList labels;   
    QString unitText;     
    TickSetFormat format; 
};

class TickLabelFormatter : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QJSValue tickLabel READ tickLabel WRITE setTickLabel NOTIFY tickLabelChanged)

public:
    explicit TickLabelFormatter(QObject* parent = nullptr);

    QString format(qreal value, qreal tickStep) const;

    QString formatLogTick(qreal value) const;

    TickSetLabels formatTicks(const TickSet& tickSet) const;

    QString formatValue(qreal value, qreal resolution, const TickSetFormat& tickSetFormat) const;

    virtual qreal minimumTickStep() const;

    QJSValue tickLabel() const;
    void setTickLabel(const QJSValue& tickLabel);

signals:
    void tickLabelChanged();
    void formatChanged();

protected:
    virtual QString doFormat(qreal value, qreal tickStep) const = 0;

    virtual QString doFormatLogTick(qreal value) const;

    virtual TickSetLabels doFormatTicks(const TickSet& tickSet) const;

    virtual QString doFormatValue(qreal value, qreal resolution, const TickSetFormat& tickSetFormat) const;

private:
    bool callTickLabel(qreal value, qreal tickStep, QString& label) const;

    QJSValue tickLabel_;
};

} // namespace QAccelPlot
```


