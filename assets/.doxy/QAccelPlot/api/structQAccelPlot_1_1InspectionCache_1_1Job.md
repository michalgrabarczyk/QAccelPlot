








# Struct QAccelPlot::InspectionCache::Job



[**ClassList**](annotated.md) **>** [**Job**](structQAccelPlot_1_1InspectionCache_1_1Job.md)




























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::atomic\_bool | [**cancelled**](#variable-cancelled)   = `{false}`<br> |
|  std::atomic\_bool | [**done**](#variable-done)   = `{false}`<br> |
|  quint64 | [**generation**](#variable-generation)   = `{0}`<br> |
|  std::unique\_ptr&lt; [**InspectionIndex**](classQAccelPlot_1_1InspectionIndex.md) &gt; | [**index**](#variable-index)  <br> |












































## Public Attributes Documentation





### variable cancelled {#variable-cancelled}

```C++
std::atomic_bool QAccelPlot::InspectionCache::Job::cancelled;
```




<hr>




### variable done {#variable-done}

```C++
std::atomic_bool QAccelPlot::InspectionCache::Job::done;
```




<hr>




### variable generation {#variable-generation}

```C++
quint64 QAccelPlot::InspectionCache::Job::generation;
```




<hr>




### variable index {#variable-index}

```C++
std::unique_ptr<InspectionIndex> QAccelPlot::InspectionCache::Job::index;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionCache.hpp`

