








# Class QAccelPlot::DataTexture



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataTexture**](classQAccelPlot_1_1DataTexture.md)



_Series data uploaded to the GPU as an RGBA8888 texture, one float per texel._ [More...](#detailed-description)

* `#include <DataTexture.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**DataTexture**](#function-datatexture-12) () = default<br>_Constructs an empty data texture;_ `texture()` _stays_`nullptr` _until the first upload._ |
|   | [**DataTexture**](#function-datatexture-22) (std::unique\_ptr&lt; QSGTexture &gt; texture) <br>_Wraps the existing_ _texture_ _, for example in tests._ |
|  QSGTexture \* | [**texture**](#function-texture) () const<br>_Returns the texture the shaders sample, or_ `nullptr` _before the first successful upload._ |
|  void | [**upload**](#function-upload) (QQuickWindow \* window, const float \* data, int floatCount) <br>_Uploads_ _floatCount_ _raw floats from__data_ _, reusing GPU and CPU buffers where possible._ |




























## Detailed Description


Materials hold it through a `std::shared_ptr`, so several nodes of one series can sample the same upload. It is created and destroyed on the render thread, like the materials holding it. 


    
## Public Functions Documentation





### function DataTexture {#function-datatexture-12}

_Constructs an empty data texture;_ `texture()` _stays_`nullptr` _until the first upload._
```C++
QAccelPlot::DataTexture::DataTexture () = default
```




<hr>




### function DataTexture {#function-datatexture-22}

_Wraps the existing_ _texture_ _, for example in tests._
```C++
explicit QAccelPlot::DataTexture::DataTexture (
    std::unique_ptr< QSGTexture > texture
) 
```




<hr>




### function texture {#function-texture}

_Returns the texture the shaders sample, or_ `nullptr` _before the first successful upload._
```C++
QSGTexture * QAccelPlot::DataTexture::texture () const
```




<hr>




### function upload {#function-upload}

_Uploads_ _floatCount_ _raw floats from__data_ _, reusing GPU and CPU buffers where possible._
```C++
void QAccelPlot::DataTexture::upload (
    QQuickWindow * window,
    const float * data,
    int floatCount
) 
```



Does nothing when _window_ is `nullptr` or _floatCount_ is not positive. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/DataTexture.hpp`

