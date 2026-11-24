# QAccelPlot

A hardware-accelerated Qt Quick plotting library: series of data drawn against axes inside a plot.

## Language

**Axis label**:
The text beside an axis or color bar naming the quantity it shows, with its unit, such as "Distance (mm)". Distinct from the tick labels.
_Avoid_: axis title, title

**Body**:
The filled part of a candle in the candlestick style, spanning from its open to its close.

**Candle**:
The mark a candlestick series draws for one data point (x, open, high, low, close), in either style: a body with a wick, or an OHLC bar.
_Avoid_: bar, stick, OHLC (as a noun for the mark)

**Data point**:
One entry of a series' data in data coordinates: a point of a line curve or point cloud, a rectangle, a bar, a band slice (x, low, high), an error bar (x, y, low, high), or a candle (x, open, high, low, close), together with its per-data-point attributes such as a point value or a category.
_Avoid_: datapoint, record, sample (for series data)

**Error bar**:
The capped line an error bar series draws at one data point, from its low to its high bound along one axis. The data point's value is the coordinate on that axis that the bounds belong to.
_Avoid_: whisker, error range

**OHLC bar**:
A candle in the OHLC style: a line from its low to its high, with the open as a tick to the left and the close as a tick to the right.
_Avoid_: bar (alone)

**Sample**:
A raw input value that a histogram counts into a bin.
_Avoid_: data point (for histogram input)

**Tick label**:
The text at a major tick giving its value.

**Unit**:
The base unit of the values an axis or color bar shows, written without a prefix, such as "m" or "s". The data is always in this unit.

**Unit prefix**:
The SI prefix a value is written with, such as "m" (milli) in "12.3 mm". An axis or color bar either shares one unit prefix across its tick labels, chosen from the visible range and usually shown in the axis label ("Distance (mm)"), or gives each tick label its own ("500 mm, 1 m").
_Avoid_: display prefix, multiplier, SI scale

**Up candle / Down candle**:
A candle whose close is at or above its open is an up candle; one whose close is below its open is a down candle.
_Avoid_: rising, falling, increasing, decreasing, bullish, bearish

**Wick**:
The line of a candle in the candlestick style, from its low to its high, behind the body.
_Avoid_: shadow, tail
