

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

struct InspectionDataPoint {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(int index MEMBER index CONSTANT)
    Q_PROPERTY(qreal x MEMBER x CONSTANT)
    Q_PROPERTY(qreal y MEMBER y CONSTANT)
    Q_PROPERTY(qreal value MEMBER value CONSTANT)
    Q_PROPERTY(qreal pixelX MEMBER pixelX CONSTANT)
    Q_PROPERTY(qreal pixelY MEMBER pixelY CONSTANT)
    Q_PROPERTY(qreal distance MEMBER distance CONSTANT)
    Q_PROPERTY(bool interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch};      
    quint64 dataRevision{0};                                 
    int index{-1};                                           
    qreal x{std::numeric_limits<qreal>::quiet_NaN()};        
    qreal y{std::numeric_limits<qreal>::quiet_NaN()};        
    qreal value{std::numeric_limits<qreal>::quiet_NaN()};    
    qreal pixelX{std::numeric_limits<qreal>::quiet_NaN()};   
    qreal pixelY{std::numeric_limits<qreal>::quiet_NaN()};   
    qreal distance{std::numeric_limits<qreal>::quiet_NaN()}; 
    bool interpolated{false};                                

    bool valid() const;
};

struct InspectionSummary {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(InspectionNS::Status status MEMBER status CONSTANT)
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
    Q_PROPERTY(InspectionNS::Status status MEMBER status CONSTANT)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(quint64 dataRevision MEMBER dataRevision CONSTANT)
    Q_PROPERTY(InspectionDataPoint left MEMBER left CONSTANT)
    Q_PROPERTY(InspectionDataPoint right MEMBER right CONSTANT)
    Q_PROPERTY(bool adjacent MEMBER adjacent CONSTANT)
    Q_PROPERTY(InspectionDataPoint interpolated MEMBER interpolated CONSTANT)

public:
    InspectionStatus status{InspectionStatus::NoMatch}; 
    quint64 dataRevision{0};                            
    InspectionDataPoint left;                           
    InspectionDataPoint right;                          
    bool adjacent{false};                               
    InspectionDataPoint interpolated;                   

    bool valid() const;
};

struct InspectionPage {
    Q_GADGET
    QML_ANONYMOUS
    Q_PROPERTY(InspectionNS::Status status MEMBER status CONSTANT)
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
    Q_PROPERTY(InspectionNS::Status status MEMBER status CONSTANT)
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


