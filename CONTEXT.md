# QAccelPlot

A hardware-accelerated Qt Quick plotting library: series of data drawn against axes inside a plot.

## Language

**Data point**:
One entry of a series' data in data coordinates: a point of a line curve or point cloud, a rectangle, a bar, or a band slice (x, low, high), together with its per-data-point attributes such as a point value or a category.
_Avoid_: datapoint, record, sample (for series data)

**Sample**:
A raw input value that a histogram counts into a bin.
_Avoid_: data point (for histogram input)
