

# File InspectionResult.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**InspectionResult.hpp**](InspectionResult_8hpp.md)

[Go to the documentation of this file](InspectionResult_8hpp.md)


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
#include <QPointF>
#include <QVariantMap>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <limits>

namespace QAccelPlot {

namespace InspectionNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(Inspection)


enum class Status {
    Ready,           
    NoMatch,         
    Idle,            
    Preparing,       
    Unsupported,     
    Unavailable,     
    InvalidArgument, 
    Stale,           
};
Q_ENUM_NS(Status)
} // namespace InspectionNS

using InspectionStatus = InspectionNS::Status;

struct InspectionSample {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(int index MEMBER index CONSTANT)
    Q_PROPERTY(qreal x READ x CONSTANT)
    Q_PROPERTY(qreal y READ y CONSTANT)
    Q_PROPERTY(qreal value MEMBER value CONSTANT)
    Q_PROPERTY(QPointF pixelPosition MEMBER pixelPosition CONSTANT)
    Q_PROPERTY(qreal distance MEMBER distance CONSTANT)
    Q_PROPERTY(bool interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch};                                                      
    quint64 dataRevision{0};                                                                                 
    int index{-1};                                                                                           
    QPointF position{std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN()};      
    QPointF pixelPosition{std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN()}; 
    qreal distance{std::numeric_limits<qreal>::quiet_NaN()};                                                 
    qreal value{std::numeric_limits<qreal>::quiet_NaN()};                                                    
    bool interpolated{false};                                                                                

    bool valid() const;
    qreal x() const;
    qreal y() const;
};

struct InspectionSummary {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(int count MEMBER count CONSTANT)
    Q_PROPERTY(qreal minimum MEMBER minimum CONSTANT)
    Q_PROPERTY(qreal maximum MEMBER maximum CONSTANT)
    Q_PROPERTY(int minimumIndex MEMBER minimumIndex CONSTANT)
    Q_PROPERTY(int maximumIndex MEMBER maximumIndex CONSTANT)
    Q_PROPERTY(qreal mean MEMBER mean CONSTANT)
    Q_PROPERTY(qreal standardDeviation MEMBER standardDeviation CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch};               
    quint64 dataRevision{0};                                          
    int count{0};                                                     
    qreal minimum{std::numeric_limits<qreal>::quiet_NaN()};           
    qreal maximum{std::numeric_limits<qreal>::quiet_NaN()};           
    int minimumIndex{-1};                                             
    int maximumIndex{-1};                                             
    qreal mean{std::numeric_limits<qreal>::quiet_NaN()};              
    qreal standardDeviation{std::numeric_limits<qreal>::quiet_NaN()}; 

    bool valid() const;
};

struct InspectionBracket {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(::QAccelPlot::InspectionSample left MEMBER left CONSTANT)
    Q_PROPERTY(::QAccelPlot::InspectionSample right MEMBER right CONSTANT)
    Q_PROPERTY(bool adjacent MEMBER adjacent CONSTANT)
    Q_PROPERTY(::QAccelPlot::InspectionSample interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; 
    quint64 dataRevision{0};                            
    InspectionSample left;                              
    InspectionSample right;                             
    bool adjacent{false};                               
    InspectionSample interpolated;                      

    bool valid() const;
};

struct InspectionPage {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(int total MEMBER total CONSTANT)
    Q_PROPERTY(int offset MEMBER offset CONSTANT)
    Q_PROPERTY(int limit MEMBER limit CONSTANT)
    Q_PROPERTY(bool hasMore MEMBER hasMore CONSTANT)
    Q_PROPERTY(bool sourceOrder MEMBER sourceOrder CONSTANT)
    Q_PROPERTY(QList<int> indices MEMBER indices CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; 
    quint64 dataRevision{0};                            
    int total{0};                                       
    int offset{0};                                      
    int limit{0};                                       
    bool hasMore{false};                                
    bool sourceOrder{false};                            
    QList<int> indices;                                 

    bool valid() const;
};

struct InspectionRecord {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(int index MEMBER index CONSTANT)
    Q_PROPERTY(bool interpolated MEMBER interpolated CONSTANT)
    Q_PROPERTY(QVariantMap fields MEMBER fields CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; 
    quint64 dataRevision{0};                            
    int index{-1};                                      
    bool interpolated{false};                           
    QVariantMap fields;                                 

    bool valid() const;
};

} // namespace QAccelPlot
```


