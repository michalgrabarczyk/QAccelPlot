








# Class QAccelPlot::Internal::HoverIndexBudget



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**HoverIndexBudget**](classQAccelPlot_1_1Internal_1_1HoverIndexBudget.md)



_Decides when queried data is worth a hover index._ [More...](#detailed-description)

* `#include <HoverIndexBudget.hpp>`



















## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::chrono::duration&lt; double, std::nano &gt; | [**CostPerRecord**](#typedef-costperrecord)  <br>_Cost of indexing one record. Fractional, because a timed build rarely costs whole nanoseconds per record._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**HoverIndexBudget**](#function-hoverindexbudget) ([**CostPerRecord**](classQAccelPlot_1_1Internal_1_1HoverIndexBudget.md#typedef-costperrecord) assumedBuildCostPerRecord) <br>_Creates a budget that assumes_ _assumedBuildCostPerRecord_ _until a build has been timed._ |
|  void | [**addBuild**](#function-addbuild) (std::chrono::nanoseconds duration, int recordCount) <br>_Replaces the assumed build cost with that of a build of_ _recordCount_ _records._ |
|  void | [**addScan**](#function-addscan) (std::chrono::nanoseconds duration) <br>_Adds the cost of one scan._  |
|  bool | [**buildDue**](#function-builddue) (int recordCount) const<br>_Returns_ `true` _once the scans have cost as much as indexing__recordCount_ _records._ |
|  void | [**reset**](#function-reset) () <br>_Forgets the scans paid so far. Call when the index stops matching the data._  |
|  void | [**timeBuild**](#function-timebuild) (const int recordCount, Build && build) <br>_Runs_ _build_ _and records its duration as the cost of indexing__recordCount_ _records._ |
|  auto | [**timeScan**](#function-timescan) (Scan && scan) <br>_Runs_ _scan_ _, adds its duration, and returns its result._ |




























## Detailed Description


A scan answers a query on any data at the cost of a pass over it. An index answers almost for free, but must be rebuilt after every data change. Queries therefore scan until the scans since the last data change have cost as much as a build would. That keeps the total within about twice the cheaper choice, whether the data changes every frame or never. 


    
## Public Types Documentation





### typedef CostPerRecord {#typedef-costperrecord}

_Cost of indexing one record. Fractional, because a timed build rarely costs whole nanoseconds per record._ 
```C++
using QAccelPlot::Internal::HoverIndexBudget::CostPerRecord =  std::chrono::duration<double, std::nano>;
```




<hr>
## Public Functions Documentation





### function HoverIndexBudget {#function-hoverindexbudget}

_Creates a budget that assumes_ _assumedBuildCostPerRecord_ _until a build has been timed._
```C++
explicit QAccelPlot::Internal::HoverIndexBudget::HoverIndexBudget (
    CostPerRecord assumedBuildCostPerRecord
) 
```




<hr>




### function addBuild {#function-addbuild}

_Replaces the assumed build cost with that of a build of_ _recordCount_ _records._
```C++
void QAccelPlot::Internal::HoverIndexBudget::addBuild (
    std::chrono::nanoseconds duration,
    int recordCount
) 
```



Builds of few records are ignored: their fixed overhead would overstate the cost per record. 


        

<hr>




### function addScan {#function-addscan}

_Adds the cost of one scan._ 
```C++
void QAccelPlot::Internal::HoverIndexBudget::addScan (
    std::chrono::nanoseconds duration
) 
```




<hr>




### function buildDue {#function-builddue}

_Returns_ `true` _once the scans have cost as much as indexing__recordCount_ _records._
```C++
bool QAccelPlot::Internal::HoverIndexBudget::buildDue (
    int recordCount
) const
```




<hr>




### function reset {#function-reset}

_Forgets the scans paid so far. Call when the index stops matching the data._ 
```C++
void QAccelPlot::Internal::HoverIndexBudget::reset () 
```




<hr>




### function timeBuild {#function-timebuild}

_Runs_ _build_ _and records its duration as the cost of indexing__recordCount_ _records._
```C++
template<typename Build>
inline void QAccelPlot::Internal::HoverIndexBudget::timeBuild (
    const int recordCount,
    Build && build
) 
```




<hr>




### function timeScan {#function-timescan}

_Runs_ _scan_ _, adds its duration, and returns its result._
```C++
template<typename Scan>
inline auto QAccelPlot::Internal::HoverIndexBudget::timeScan (
    Scan && scan
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/HoverIndexBudget.hpp`

