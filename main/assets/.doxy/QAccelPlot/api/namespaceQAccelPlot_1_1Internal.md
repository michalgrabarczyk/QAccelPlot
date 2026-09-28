








# Namespace QAccelPlot::Internal



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md)




























## Public Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kDataTextureWidth**](#variable-kdatatexturewidth)   = `{8192}`<br>_Number of floats stored per data texture row, one float per RGBA8 texel._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  constexpr int | [**dataTextureHeight**](#function-datatextureheight) (const int floatCount) <br>_Returns the number of data texture rows needed to store_ _floatCount_ _floats._ |
|  constexpr qint64 | [**dataTextureItemCapacity**](#function-datatextureitemcapacity) (const int maxHeight, const int floatsPerItem) <br>_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture at most__maxHeight_ _rows tall._ |
|  void | [**uploadDataTexture**](#function-uploaddatatexture) (std::unique\_ptr&lt; QSGTexture &gt; & texture, QQuickWindow \* window, const QImage & image) <br>_Uploads an image to the live-data texture using the configured Qt API path._  |




























## Public Attributes Documentation





### variable kDataTextureWidth {#variable-kdatatexturewidth}

_Number of floats stored per data texture row, one float per RGBA8 texel._ 
```C++
constexpr int QAccelPlot::Internal::kDataTextureWidth;
```



data\_texture.glsl reads the width from the bound texture, so this is the only definition. 


        

<hr>
## Public Functions Documentation





### function dataTextureHeight {#function-datatextureheight}

_Returns the number of data texture rows needed to store_ _floatCount_ _floats._
```C++
constexpr int QAccelPlot::Internal::dataTextureHeight (
    const int floatCount
) 
```




<hr>




### function dataTextureItemCapacity {#function-datatextureitemcapacity}

_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture at most__maxHeight_ _rows tall._
```C++
constexpr qint64 QAccelPlot::Internal::dataTextureItemCapacity (
    const int maxHeight,
    const int floatsPerItem
) 
```




<hr>




### function uploadDataTexture {#function-uploaddatatexture}

_Uploads an image to the live-data texture using the configured Qt API path._ 
```C++
void QAccelPlot::Internal::uploadDataTexture (
    std::unique_ptr< QSGTexture > & texture,
    QQuickWindow * window,
    const QImage & image
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/DataTextureLayout.hpp`

