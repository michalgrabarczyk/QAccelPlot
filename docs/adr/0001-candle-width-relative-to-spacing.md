# Candle width is a fraction of the candle spacing

`CandlestickSeries.candleWidth` is a fraction of `spacing`, not a width in data units like `BarSeries.barWidth`. `spacing` defaults to the smallest positive gap between consecutive x values, can be pinned in x data units, and goes back to automatic when assigned `undefined` (QML `RESET`); reading it returns the spacing in effect. Financial x values are usually epoch timestamps, so a width in data units would have no sensible default and would change with every timeframe and time unit. Highcharts (`pointRange`) and Plotly also derive candle width from the closest x distance.

## Considered Options

- **Width in data units** (as `BarSeries`): no default works for timestamp x values.
- **Automatic spacing only**: one bad timestamp shrinks every candle, a single streamed candle has no gap, and volume bars on timestamps can't align. Pinning `spacing` covers these.
- **Median gap**: survives an outlier but overlaps candles wherever gaps are below the median.
- **NaN for automatic, or a mode enum**: NaN as "unset" is not a Qt idiom, and a mode enum lets a set `spacing` be silently ignored. `RESET` to `undefined` matches `Item.width` and `AxisLabel.rotation`.
