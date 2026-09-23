

# File Colormap.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**Colormap.hpp**](Colormap_8hpp.md)

[Go to the documentation of this file](Colormap_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <QObject>
#include <QQmlListProperty>
#include <QtQml/qqmlregistration.h>

#include <vector>

namespace QAccelPlot {

class Colormap : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Colormap)

    
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY colormapChanged)
    Q_PROPERTY(QQmlListProperty<QObject> stops READ stops NOTIFY colormapChanged)
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY colormapChanged)
    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY colormapChanged)
    Q_PROPERTY(Normalization norm READ norm WRITE setNorm NOTIFY colormapChanged)

public:
    enum class Preset {
        Viridis,   
        Plasma,    
        Inferno,   
        Magma,     
        Turbo,     
        Grayscale, 
        Rainbow    
    };
    Q_ENUM(Preset)

    
    enum class Normalization {
        Linear, 
        Log     
    };
    Q_ENUM(Normalization)

    
    explicit Colormap(QObject* parent = nullptr);

    Preset preset() const;
    void setPreset(Preset preset);

    QQmlListProperty<QObject> stops();

    qreal min() const;
    void setMin(qreal value);

    qreal max() const;
    void setMax(qreal value);

    Normalization norm() const;
    void setNorm(Normalization norm);

    const std::vector<GradientStopData>& resolvedStops() const;

signals:
    void colormapChanged();

private:
    void rebuildStops();

    static void appendStop(QQmlListProperty<QObject>* list, QObject* stop);
    static qsizetype stopCount(QQmlListProperty<QObject>* list);
    static QObject* stopAt(QQmlListProperty<QObject>* list, qsizetype index);
    static void clearStops(QQmlListProperty<QObject>* list);

    Preset preset_{Preset::Viridis};
    QList<QObject*> stopObjects_;
    std::vector<GradientStopData> resolvedStops_;
    qreal min_;
    qreal max_;
    Normalization norm_{Normalization::Linear};
};

} // namespace QAccelPlot
```


