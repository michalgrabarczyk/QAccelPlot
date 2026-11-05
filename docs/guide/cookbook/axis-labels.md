---
description: "Place QAccelPlot axis labels at the start, center, or end of an axis, rotate and offset them, or replace them with a custom QML item."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Axis labels

The `Axis.label` group holds the axis title. `label.text`, `label.font`, and
`label.color` style it, the placement properties move it, and `label.delegate`
replaces it with any QML item.

## Alignment, rotation, and padding

```qml
yAxis: QAccelPlot.Axis {
    label.text: "Amplitude (mV)"
    label.padding: 40
    label.alignment: QAccelPlot.Axis.End // top of a y axis
    label.rotation: 0                    // horizontal text
    label.offset: Qt.point(0, -2)
    layoutSize: 140                      // room for the horizontal label
}
```

| Property | Effect |
| --- | --- |
| `label.alignment` | `Axis.Start`, `Axis.Center` (default), or `Axis.End`. `Start` is the `viewportMin` end: left on horizontal axes, bottom on vertical axes. The label stays flush with that end. |
| `label.rotation` | Clockwise degrees. Default: 0 on horizontal axes, -90 on the left, 90 on the right. Assign `undefined` to return to the default. |
| `label.padding` | Gap between the baseline and the near edge of the rotated label. |
| `label.offset` | Pixel offset applied last. |

The painted label is clipped to the axis item, and the axis does not grow to
fit it. Raise `layoutSize` for a horizontal label on a vertical axis.

## Custom label item

`label.delegate` takes a component. The axis creates one instance as its child,
so `parent` is the axis item and anchors, `x`, `y`, and `rotation` place it.
The item can extend beyond the axis, for example above the plot:

```qml
yAxis: QAccelPlot.Axis {
    id: yAxis
    label.text: "mV"
    label.delegate: Text {
        anchors.bottom: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 20
        text: yAxis.label.text
        font: yAxis.label.font
        color: yAxis.hovered ? yAxis.hoverColor : yAxis.baselineColor
    }
}
```

While a delegate is set, the axis paints no label and ignores the placement
properties above. The delegate binds `label.text`, `label.font`, and colors itself.
Mouse input reaches the axis unless the delegate handles it.

Complete source: [`examples/axes/axis_labels`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axes/axis_labels)
