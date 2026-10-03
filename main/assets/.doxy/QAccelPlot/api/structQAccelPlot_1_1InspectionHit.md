








# Struct QAccelPlot::InspectionHit



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md)



_Nearest-sample result: source index and pixel distance, or index -1._ 

* `#include <InspectionTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**distance**](#variable-distance)   = `{std::numeric\_limits&lt;double&gt;::infinity()}`<br> |
|  int | [**index**](#variable-index)   = `{-1}`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**consider**](#function-consider) (int candidate, double candidateDistance) noexcept<br>_Keeps the closer candidate; equal distances resolve to the highest index, the sample drawn last._  |




























## Public Attributes Documentation





### variable distance {#variable-distance}

```C++
double QAccelPlot::InspectionHit::distance;
```




<hr>




### variable index {#variable-index}

```C++
int QAccelPlot::InspectionHit::index;
```




<hr>
## Public Functions Documentation





### function consider {#function-consider}

_Keeps the closer candidate; equal distances resolve to the highest index, the sample drawn last._ 
```C++
void QAccelPlot::InspectionHit::consider (
    int candidate,
    double candidateDistance
) noexcept
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionTypes.hpp`

