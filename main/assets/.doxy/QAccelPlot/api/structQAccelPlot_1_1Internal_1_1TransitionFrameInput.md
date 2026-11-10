








# Struct QAccelPlot::Internal::TransitionFrameInput



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**TransitionFrameInput**](structQAccelPlot_1_1Internal_1_1TransitionFrameInput.md)



_The two sides of a transition and the steps of its slots at one frame._ 

* `#include <TransitionFrame.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::optional&lt; [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) &gt; | [**enter**](#variable-enter)  <br>_Step of the enter slot, when it runs._  |
|  std::optional&lt; [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) &gt; | [**exit**](#variable-exit)  <br>_Step of the exit slot, when it runs._  |
|  [**TransitionItems**](structQAccelPlot_1_1Internal_1_1TransitionItems.md) | [**from**](#variable-from)  <br>_Items drawn when the change happened._  |
|  [**TransitionLayout**](structQAccelPlot_1_1Internal_1_1TransitionLayout.md) | [**layout**](#variable-layout)  <br>_How the items animate._  |
|  [**TransitionItems**](structQAccelPlot_1_1Internal_1_1TransitionItems.md) | [**to**](#variable-to)  <br>_The new items._  |
|  std::optional&lt; [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) &gt; | [**update**](#variable-update)  <br>_Step of the update slot, when it runs._  |












































## Public Attributes Documentation





### variable enter {#variable-enter}

_Step of the enter slot, when it runs._ 
```C++
std::optional<TransitionStep> QAccelPlot::Internal::TransitionFrameInput::enter;
```




<hr>




### variable exit {#variable-exit}

_Step of the exit slot, when it runs._ 
```C++
std::optional<TransitionStep> QAccelPlot::Internal::TransitionFrameInput::exit;
```




<hr>




### variable from {#variable-from}

_Items drawn when the change happened._ 
```C++
TransitionItems QAccelPlot::Internal::TransitionFrameInput::from;
```




<hr>




### variable layout {#variable-layout}

_How the items animate._ 
```C++
TransitionLayout QAccelPlot::Internal::TransitionFrameInput::layout;
```




<hr>




### variable to {#variable-to}

_The new items._ 
```C++
TransitionItems QAccelPlot::Internal::TransitionFrameInput::to;
```




<hr>




### variable update {#variable-update}

_Step of the update slot, when it runs._ 
```C++
std::optional<TransitionStep> QAccelPlot::Internal::TransitionFrameInput::update;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

