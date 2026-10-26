# Vibration Monitoring

A condition-monitoring dashboard for a motor that drives a pump through a gearbox. Eight
accelerometers stream at 25.6 kHz into ten-second scrolling waveforms: about 2 million
samples, all replaced every frame, with no downsampling. The Pump DE bearing has an outer-race
fault: its RMS level is in alarm, and with Pump DE selected, the fault's harmonics rise above the
alarm envelope in the spectrum and stand out in the spectrogram.

| Panel | Shows | QAccelPlot types |
| --- | --- | --- |
| Waveforms | Eight channels stacked in one plot, named on the Y axis | `LineCurve` ×8 fed by `postData()`, `TextTickLabelFormatter`, `PlotInspector` |
| RMS | Level of each channel over the last second, beside its row | `BarSeries` colored by value through a `Colormap`, `MorphTransition` |
| Spectrum | Averaged spectrum of the selected channel, its alarm envelope 6 dB above a healthy baseline, the fault's peak | `LineCurve` with `GradientFill`, log axis, `BandSeries`, `DataAnchor` |
| Spectrogram | Short-time spectra of the selected channel over the same ten seconds, frequency aligned with the spectrum | `HeatmapSeries` fed by `postValues()`, keyed by a level histogram (`BarSeries` colored through the shared `Colormap`) |

All data is synthetic and deterministic: shaft, gear-mesh, fan, and vane-pass tones, seeded
noise, and bearing impacts at the ball pass frequency (BPFO, 106 Hz) that ring a housing
resonance. Every component repeats after eight seconds.

## Interaction

- Click a waveform row or an RMS bar to show that channel's spectrum and spectrogram.
- Hover for values; wheel to zoom down to single samples, which show as dots; drag to pan; double-click to reset.
- Space or **Hold** freezes the acquisition, so a zoomed-in trace stops scrolling.
- **Dark** switches between `QAccelPlot.Colors.dark` and `QAccelPlot.Colors.light`.

## Data flow

`AcquisitionWorker` runs on a background thread. For each display frame it generates the new
samples, posts every waveform with `LineCurve::postData()`, and ten times per second posts the
spectrum, the spectrogram (`HeatmapSeries::postValues()`), and the RMS bars. It never touches
an item: the posted buffers are moved into the series on the UI thread. It starts the next frame
only after the UI thread applied the previous one, so slow frames never queue up.

| File | Role |
| --- | --- |
| `src/VibrationModel` | Signal model of the eight channels |
| `src/SpectrumAnalyzer` | Radix-2 FFT, averaged spectrum, spectrogram ring buffer |
| `src/AcquisitionWorker` | Background acquisition and the handoff to the series |
| `src/AcquisitionClock` | Real-time clock with hold, or a stepped clock for recordings |
| `src/MonitorController` | The `monitor` object the QML binds to |
| `qml/DemoTimeline.qml` | Scripted eight-second loop: inspects the fault's peak, zooms into an impact |

## Options

| Option | Effect |
| --- | --- |
| `--theme light` | Starts with the light palette |
| `--demo` | Plays the scripted loop on live data |
| `--record <dir>` | Records the scripted loop to `<dir>/frame_00000.png`, ... and quits |
| `--frames <n>`, `--fps <n>` | Frames to record (default 200) and the time step between them (default 25 per second) |

A recording runs at a fixed 896 × 504 window size. Each frame advances the acquisition time and
Qt's animation clock by exactly `1 / fps`, so transitions play as they do live, and is captured
offscreen. The result does not depend on how fast the machine renders, and runs with either theme
match frame for frame. Turn the frames into a GIF with ffmpeg or gifski:

```sh
QAccelPlotExampleVibrationMonitoring --record frames/dark --theme dark
python scripts/frames-to-gif.py frames/dark vibration-dark.gif
```
