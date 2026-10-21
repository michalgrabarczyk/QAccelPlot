---
description: "Save a plot as a PNG, JPEG, or other raster image, copy it to the clipboard, or capture it as a QImage."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Exporting images

[`PlotExporter`][plot-exporter] captures a plot as a raster image:

```qml
import QAccelPlot as QAccelPlot

QAccelPlot.PlotExporter {
    id: exporter
    plot: plot
    scale: 2
    onFinished: (ok, path, error) => console.log(ok ? "Saved " + path : error)
}

Button {
    text: "Save"
    enabled: !exporter.busy
    onClicked: exporter.save("plot.png")
}
```

| Method | Result |
| --- | --- |
| `save(path)` | Writes a file in the format the suffix names. `path` may be a `file:` URL from a `FileDialog`. |
| `copyToClipboard()` | Puts the image on the clipboard. |
| `grabImage()` | Emits `imageReady(image)` with a `QImage`. |
| `supportedFormats()` | Lists the suffixes `save()` accepts, such as `png`, `jpg`, `bmp`, `webp`, `tif`. Image format plugins decide which are present. |

The image contains the plot item and everything inside its bounds as shown on
screen: series, axes, the legend, overlays, and child items.

## Options

| Property | Default | Meaning |
| --- | --- | --- |
| `scale` | `0` | Image pixels per logical pixel. `0` uses the window's device pixel ratio, so the image matches the screen. The layout does not change: a 2× image has the same ticks and text positions, drawn at twice the resolution. At a device pixel ratio other than 1, a side can differ by one pixel from its size × scale. |
| `quality` | `-1` | 0–100, passed to `QImageWriter`. `-1` uses the format's default. JPEG and WebP trade size for artifacts; PNG stays lossless. |
| `dpi` | `0` | Resolution stored in the file. `0` stores 96 × scale, so documents place the image at its on-screen size. |
| `transparentBackground` | `false` | Leaves out `axesAreaColor` and `plotAreaColor`, so the image takes the background it is placed on. The grid, series, axes, and legend are drawn as usual. |

Formats without an alpha channel, such as JPEG and BMP, composite transparent
pixels over white. For a light image of a dark-themed plot, bind its colors to
`Colors.light` before exporting, as the
[Theming example](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/theming)
does.

## Requests

- Each request ends with one `finished(ok, path, error)`. `path` is empty
  except for `save()`.
- A request made while `busy` only returns `false` and warns; `finished()`
  stays with the request in progress.
- The capture happens on the next frame. It fails if the window is hidden,
  minimized, or destroyed first. Encoding and writing the file run on a worker
  thread, and an existing file is replaced only once the new one is complete.
- A request fails immediately, returning `false`, when another exporter is
  capturing the same plot, the file format is unsupported, the plot is hidden,
  has no size, or is not in a window, the window is hidden or minimized, the
  window uses the software backend, or the image would exceed the GPU's
  maximum texture size.
- With `transparentBackground` set, the plot shows no background for the one
  frame the capture is taken in.

## C++

```cpp
auto* exporter = new QAccelPlot::PlotExporter(this);
exporter->setPlot(plot);
exporter->setScale(3.0);
connect(exporter, &QAccelPlot::PlotExporter::imageReady, this, [](const QImage& image) {
    // Use the image on the UI thread.
});
exporter->grabImage();
```

Complete source: [`examples/data/export`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/data/export)

[plot-exporter]: ../api/classQAccelPlot_1_1PlotExporter.md
