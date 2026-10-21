# QAccelPlot

A hardware-accelerated Qt Quick plotting library: series of data drawn against axes inside a plot.

## Language

**Data point**:
One entry of a series' data in data coordinates: a point of a line curve or point cloud, a rectangle, a bar, a band slice (x, low, high), or an error bar (x, y, low, high), together with its per-data-point attributes such as a point value or a category.
_Avoid_: datapoint, record, sample (for series data)

**Error bar**:
The capped line an error bar series draws at one data point, from its low to its high bound along one axis. The data point's value is the coordinate on that axis that the bounds belong to.
_Avoid_: whisker, error range

**Sample**:
A raw input value that a histogram counts into a bin.
_Avoid_: data point (for histogram input)
