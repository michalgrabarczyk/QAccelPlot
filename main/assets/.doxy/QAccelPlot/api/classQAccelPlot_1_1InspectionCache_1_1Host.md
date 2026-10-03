








# Class QAccelPlot::InspectionCache::Host



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionCache**](classQAccelPlot_1_1InspectionCache.md) **>** [**Host**](classQAccelPlot_1_1InspectionCache_1_1Host.md)



_Series-side callbacks; all are invoked on the series' thread._ 

* `#include <InspectionCache.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
| virtual void | [**indexReady**](#function-indexready) () = 0<br> |
| virtual [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) | [**indexSource**](#function-indexsource) () const = 0<br> |
| virtual bool | [**indexSourceAvailable**](#function-indexsourceavailable) () const = 0<br> |
| virtual  | [**~Host**](#function-host) () <br> |




























## Public Functions Documentation





### function indexReady {#function-indexready}

```C++
virtual void QAccelPlot::InspectionCache::Host::indexReady () = 0
```




<hr>




### function indexSource {#function-indexsource}

```C++
virtual InspectionSource QAccelPlot::InspectionCache::Host::indexSource () const = 0
```




<hr>




### function indexSourceAvailable {#function-indexsourceavailable}

```C++
virtual bool QAccelPlot::InspectionCache::Host::indexSourceAvailable () const = 0
```




<hr>




### function ~Host {#function-host}

```C++
virtual QAccelPlot::InspectionCache::Host::~Host () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionCache.hpp`

