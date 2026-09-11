








# Class QAccelPlot::GradientTexture



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md)



_Cached one-dimensional texture used by gradient materials._ 

* `#include <GradientTexture.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**GradientTexture**](#function-gradienttexture-12) () = default<br> |
|   | [**GradientTexture**](#function-gradienttexture-22) (const [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md) &) = delete<br> |
|  void | [**commit**](#function-commit) (QSGMaterialShader::RenderState & state, int binding, QSGTexture \*\* texture) <br>_Commits the texture to the requested shader sampler binding._  |
|  qint64 | [**comparisonKey**](#function-comparisonkey) () const<br>_Returns the scene-graph comparison key for the uploaded texture, or zero when no texture exists._  |
|  [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md) & | [**operator=**](#function-operator) (const [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md) &) = delete<br> |
|  void | [**upload**](#function-upload) (QQuickWindow \* window, const std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; & stops) <br>_Updates the 256-sample texture when_ _stops_ _changed._ |
|   | [**~GradientTexture**](#function-gradienttexture) () <br> |




























## Public Functions Documentation





### function GradientTexture {#function-gradienttexture-12}

```C++
QAccelPlot::GradientTexture::GradientTexture () = default
```




<hr>




### function GradientTexture {#function-gradienttexture-22}

```C++
QAccelPlot::GradientTexture::GradientTexture (
    const GradientTexture &
) = delete
```




<hr>




### function commit {#function-commit}

_Commits the texture to the requested shader sampler binding._ 
```C++
void QAccelPlot::GradientTexture::commit (
    QSGMaterialShader::RenderState & state,
    int binding,
    QSGTexture ** texture
) 
```




<hr>




### function comparisonKey {#function-comparisonkey}

_Returns the scene-graph comparison key for the uploaded texture, or zero when no texture exists._ 
```C++
qint64 QAccelPlot::GradientTexture::comparisonKey () const
```




<hr>




### function operator= {#function-operator}

```C++
GradientTexture & QAccelPlot::GradientTexture::operator= (
    const GradientTexture &
) = delete
```




<hr>




### function upload {#function-upload}

_Updates the 256-sample texture when_ _stops_ _changed._
```C++
void QAccelPlot::GradientTexture::upload (
    QQuickWindow * window,
    const std::vector< GradientStopData > & stops
) 
```




<hr>




### function ~GradientTexture {#function-gradienttexture}

```C++
QAccelPlot::GradientTexture::~GradientTexture () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/materials/GradientTexture.hpp`

