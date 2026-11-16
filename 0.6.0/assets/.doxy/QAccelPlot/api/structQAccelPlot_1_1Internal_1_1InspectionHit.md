








# Struct QAccelPlot::Internal::InspectionHit



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**InspectionHit**](structQAccelPlot_1_1Internal_1_1InspectionHit.md)



_Nearest-data-point result: source index and pixel distance, or index -1._ 

* `#include <InspectionTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**distance**](#variable-distance)   = `{std::numeric\_limits&lt;double&gt;::infinity()}`<br> |
|  int | [**index**](#variable-index)   = `{-1}`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**consider**](#function-consider) (int candidate, double candidateDistance) noexcept<br>_Keeps the closer candidate; equal distances resolve to the highest index, the data point drawn last._  |




























## Public Attributes Documentation





### variable distance {#variable-distance}

```C++
double QAccelPlot::Internal::InspectionHit::distance;
```




<hr>




### variable index {#variable-index}

```C++
int QAccelPlot::Internal::InspectionHit::index;
```




<hr>
## Public Functions Documentation





### function consider {#function-consider}

_Keeps the closer candidate; equal distances resolve to the highest index, the data point drawn last._ 
```C++
void QAccelPlot::Internal::InspectionHit::consider (
    int candidate,
    double candidateDistance
) noexcept
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionTypes.hpp`

