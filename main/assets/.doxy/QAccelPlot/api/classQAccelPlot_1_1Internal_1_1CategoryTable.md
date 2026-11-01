








# Class QAccelPlot::Internal::CategoryTable



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**CategoryTable**](classQAccelPlot_1_1Internal_1_1CategoryTable.md)



_A snapshot of the categories of a series, looked up by value or name._ 

* `#include <CategoryTable.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Entry**](structQAccelPlot_1_1Internal_1_1CategoryTable_1_1Entry.md) <br>_One category: the value data points carry, an optional name, and an optional color._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  QColor | [**color**](#function-color) (int value) const<br>_Returns the color of the category with_ _value_ _, or an invalid color when it has none._ |
|  std::optional&lt; int &gt; | [**dataPointCategory**](#function-datapointcategory) (const QVariant & value, const char \* seriesName, qsizetype index) const<br>_Returns the category in_ _value_ _, the_`category` _of the QML data point at__index_ _._ |
|  bool | [**empty**](#function-empty) () const<br>_Returns true when there are no categories._  |
|  const std::vector&lt; [**Entry**](structQAccelPlot_1_1Internal_1_1CategoryTable_1_1Entry.md) &gt; & | [**entries**](#function-entries) () const<br>_Returns the categories in list order._  |
|  QList&lt; [**LegendEntry**](structQAccelPlot_1_1LegendEntry.md) &gt; | [**legendEntries**](#function-legendentries) (const [**LegendEntry**](structQAccelPlot_1_1LegendEntry.md) & seriesEntry) const<br>_Returns one legend entry per category:_ _seriesEntry_ _with the category's name and color._ |
|  QString | [**name**](#function-name) (int value) const<br>_Returns the name of the category with_ _value_ _, or an empty string when it has none._ |
|  const QStringList & | [**problems**](#function-problems) () const<br>_Returns a description of each category the snapshot skipped or could not name._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**CategoryTable**](classQAccelPlot_1_1Internal_1_1CategoryTable.md) | [**fromCategories**](#function-fromcategories) (const QList&lt; [**Category**](classQAccelPlot_1_1Category.md) \* &gt; & categories) <br>_Takes a snapshot of_ _categories_ _, skipping null entries._ |


























## Public Functions Documentation





### function color {#function-color}

_Returns the color of the category with_ _value_ _, or an invalid color when it has none._
```C++
QColor QAccelPlot::Internal::CategoryTable::color (
    int value
) const
```




<hr>




### function dataPointCategory {#function-datapointcategory}

_Returns the category in_ _value_ _, the_`category` _of the QML data point at__index_ _._
```C++
std::optional< int > QAccelPlot::Internal::CategoryTable::dataPointCategory (
    const QVariant & value,
    const char * seriesName,
    qsizetype index
) const
```



A number must be a whole number; a string is a category name. Returns `std::nullopt` for a missing or null _value_, and for any other value, which logs a warning naming _seriesName_. 


        

<hr>




### function empty {#function-empty}

_Returns true when there are no categories._ 
```C++
bool QAccelPlot::Internal::CategoryTable::empty () const
```




<hr>




### function entries {#function-entries}

_Returns the categories in list order._ 
```C++
const std::vector< Entry > & QAccelPlot::Internal::CategoryTable::entries () const
```




<hr>




### function legendEntries {#function-legendentries}

_Returns one legend entry per category:_ _seriesEntry_ _with the category's name and color._
```C++
QList< LegendEntry > QAccelPlot::Internal::CategoryTable::legendEntries (
    const LegendEntry & seriesEntry
) const
```



A category without a name is labeled with its value; one without a color keeps the series color. 


        

<hr>




### function name {#function-name}

_Returns the name of the category with_ _value_ _, or an empty string when it has none._
```C++
QString QAccelPlot::Internal::CategoryTable::name (
    int value
) const
```




<hr>




### function problems {#function-problems}

_Returns a description of each category the snapshot skipped or could not name._ 
```C++
const QStringList & QAccelPlot::Internal::CategoryTable::problems () const
```




<hr>
## Public Static Functions Documentation





### function fromCategories {#function-fromcategories}

_Takes a snapshot of_ _categories_ _, skipping null entries._
```C++
static CategoryTable QAccelPlot::Internal::CategoryTable::fromCategories (
    const QList< Category * > & categories
) 
```



Keeps the first of categories that share a value or a name, and describes each later one in `problems()`. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/CategoryTable.hpp`

