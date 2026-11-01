








# Class QAccelPlot::Internal::ScopedPaintedItemTextures



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md)



_Rasterizes the painted items under a root item at a capture scale until destroyed._ [More...](#detailed-description)

* `#include <ScopedPaintedItemTextures.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ScopedPaintedItemTextures**](#function-scopedpainteditemtextures-13) (QQuickItem \* root, qreal pixelsPerLogicalPixel, qreal devicePixelRatio) <br>_Resizes the textures of_ _root_ _and its painted descendants rendered at__devicePixelRatio_ _._ |
|   | [**ScopedPaintedItemTextures**](#function-scopedpainteditemtextures-23) (const [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) &) = delete<br> |
|   | [**ScopedPaintedItemTextures**](#function-scopedpainteditemtextures-33) ([**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) &&) = delete<br> |
|  [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) & | [**operator=**](#function-operator) (const [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) &) = delete<br> |
|  [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) & | [**operator=**](#function-operator_1) ([**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) &&) = delete<br> |
|   | [**~ScopedPaintedItemTextures**](#function-scopedpainteditemtextures) () <br>_Restores the previous texture sizes of the items that still exist._  |




























## Detailed Description


A `QQuickPaintedItem` rasterizes into a texture sized for its window's device pixel ratio, so an item grab at a higher scale would magnify, and blur, that texture. This raises every painted descendant's texture to _pixelsPerLogicalPixel_ and restores the previous texture sizes on destruction. 


    
## Public Functions Documentation





### function ScopedPaintedItemTextures {#function-scopedpainteditemtextures-13}

_Resizes the textures of_ _root_ _and its painted descendants rendered at__devicePixelRatio_ _._
```C++
QAccelPlot::Internal::ScopedPaintedItemTextures::ScopedPaintedItemTextures (
    QQuickItem * root,
    qreal pixelsPerLogicalPixel,
    qreal devicePixelRatio
) 
```




<hr>




### function ScopedPaintedItemTextures {#function-scopedpainteditemtextures-23}

```C++
QAccelPlot::Internal::ScopedPaintedItemTextures::ScopedPaintedItemTextures (
    const ScopedPaintedItemTextures &
) = delete
```




<hr>




### function ScopedPaintedItemTextures {#function-scopedpainteditemtextures-33}

```C++
QAccelPlot::Internal::ScopedPaintedItemTextures::ScopedPaintedItemTextures (
    ScopedPaintedItemTextures &&
) = delete
```




<hr>




### function operator= {#function-operator}

```C++
ScopedPaintedItemTextures & QAccelPlot::Internal::ScopedPaintedItemTextures::operator= (
    const ScopedPaintedItemTextures &
) = delete
```




<hr>




### function operator= {#function-operator_1}

```C++
ScopedPaintedItemTextures & QAccelPlot::Internal::ScopedPaintedItemTextures::operator= (
    ScopedPaintedItemTextures &&
) = delete
```




<hr>




### function ~ScopedPaintedItemTextures {#function-scopedpainteditemtextures}

_Restores the previous texture sizes of the items that still exist._ 
```C++
QAccelPlot::Internal::ScopedPaintedItemTextures::~ScopedPaintedItemTextures () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/export/internal/ScopedPaintedItemTextures.hpp`

