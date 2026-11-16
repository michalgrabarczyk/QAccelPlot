








# Struct QAccelPlot::Internal::DataTextureUploadTotals



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**DataTextureUploadTotals**](structQAccelPlot_1_1Internal_1_1DataTextureUploadTotals.md)



_How many data textures were uploaded since the last reset, and how many bytes they held._ [More...](#detailed-description)

* `#include <DataTextureUploadTotals.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::int64\_t | [**bytes**](#variable-bytes)   = `{0}`<br>_Bytes of series data uploaded, without the padding of the last texture row._  |
|  std::int64\_t | [**count**](#variable-count)   = `{0}`<br>_Number of uploads._  |












































## Detailed Description


Tests read the totals to check how much data a frame uploads. 


    
## Public Attributes Documentation





### variable bytes {#variable-bytes}

_Bytes of series data uploaded, without the padding of the last texture row._ 
```C++
std::int64_t QAccelPlot::Internal::DataTextureUploadTotals::bytes;
```




<hr>




### variable count {#variable-count}

_Number of uploads._ 
```C++
std::int64_t QAccelPlot::Internal::DataTextureUploadTotals::count;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/DataTextureUploadTotals.hpp`

