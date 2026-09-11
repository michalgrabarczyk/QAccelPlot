---
description: "Step-by-step guide to building your first QAccelPlot application with CMake FetchContent, QML plot composition, and C++ data supply."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Getting started

This walkthrough embeds QAccelPlot with CMake, creates a minimal QML plot, and
loads its data from C++. It uses this project layout:

```text
my-plot-app/
├── CMakeLists.txt
├── main.cpp
└── qml/
    └── Main.qml
```

## 1. Add QAccelPlot to the build

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.22)

project(MyPlotApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Quick ShaderTools)

include(FetchContent)
FetchContent_Declare(
    QAccelPlot
    GIT_REPOSITORY https://github.com/michalgrabarczyk/QAccelPlot.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(QAccelPlot)

qt_add_executable(MyPlotApp MANUAL_FINALIZATION
    main.cpp
)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
qt_add_resources(MyPlotApp "app_qml"
    PREFIX "/app"
    FILES qml/Main.qml
)

target_link_libraries(MyPlotApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    QAccelPlot::QAccelPlot
)

qt_import_qml_plugins(MyPlotApp)
qt_finalize_executable(MyPlotApp)

# Windows has no build RPATH, so deploy Qt for direct Explorer launches.
if(WIN32)
    find_program(MY_PLOT_APP_WINDEPLOYQT
        NAMES windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
        REQUIRED
    )

    add_custom_command(TARGET MyPlotApp POST_BUILD
        COMMAND "${MY_PLOT_APP_WINDEPLOYQT}"
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
            --no-translations
            "$<TARGET_FILE:MyPlotApp>"
        COMMENT "Deploying the Qt runtime beside MyPlotApp"
        VERBATIM
    )
endif()
```

`QAccelPlot::QAccelPlot` is the application-facing target. It brings in both
the backing library and the static QML plugin. When QAccelPlot is included as a
subproject, its examples, tests, benchmarks, and install rules default to off.
The `ShaderTools` component is required when building QAccelPlot from source so
that its GLSL shaders can be compiled and embedded in the library.

To use an existing source checkout instead, replace the `FetchContent` block
with:

```cmake
add_subdirectory(path/to/QAccelPlot)
```

## 2. Create the plot

Create `qml/Main.qml`:

```qml
import QtQuick
import QAccelPlot as QAccelPlot

Window {
    width: 800
    height: 500
    visible: true
    title: "QAccelPlot example"

    QAccelPlot.Plot {
        id: plot
        anchors.fill: parent
        legendVisible: false

        xAxis: QAccelPlot.Axis {
            viewportMin: 0
            viewportMax: 10
            dataMin: 0
            dataMax: 10
            side: QAccelPlot.Axis.Bottom
            label: "Time (s)"
        }

        yAxis: QAccelPlot.Axis {
            viewportMin: -1.2
            viewportMax: 1.2
            dataMin: -1.2
            dataMax: 1.2
            side: QAccelPlot.Axis.Left
            label: "Amplitude"
            axisTitlePadding: 34
        }

        QAccelPlot.LineCurve {
            objectName: "curve"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: "darkorange"
            lineWidth: 2
        }
    }
}
```

Every series must be connected to an X and Y axis. [`viewportMin`][viewport-min]
and [`viewportMax`][viewport-max] define what is visible; [`dataMin`][data-min]
and [`dataMax`][data-max] describe the known data extent used when rescaling.

## 3. Supply data from C++

Create `main.cpp`:

```cpp
#include "series/LineCurve.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <vector>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/app/Main.qml")));

    auto* root = engine.rootObjects().value(0);
    auto* curve = root
        ? root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("curve"))
        : nullptr;
    if (!curve) {
        return -1;
    }

    const std::vector<double> times { 0, 2.5, 5, 7.5, 10 };
    const std::vector<double> amplitudes { 0, 0.8, -0.6, 0.4, 0 };
    curve->setData(times, amplitudes);
    return app.exec();
}
```

[`setData()`][set-data-vectors] accepts separate X and Y vectors, which is a convenient fit for
small, static data sets. For larger or frequently updated data, see the
[Performance guide](performance.md) for more efficient data-ingestion paths.

## 4. Configure and build

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_USE_QT_PRIVATE_API=OFF
cmake --build build --config Release
```

The `QACCELPLOT_USE_QT_PRIVATE_API` flag affects performance; see
the [Performance guide](performance.md#select-the-texture-upload-mode).

## Default interaction

- Drag inside the plot area to pan all attached axes.
- Use the mouse wheel over the plot to zoom all axes around the pointer.
- Use the mouse wheel over an individual axis to zoom only that axis.
- Double-click the plot to rescale all axes to their data ranges.
- Double-click an axis to rescale that axis only.

See [Troubleshooting](troubleshooting.md) for common setup issues.

[viewport-min]: api/classQAccelPlot_1_1Axis.md#property-viewportmin-12
[viewport-max]: api/classQAccelPlot_1_1Axis.md#property-viewportmax-12
[data-min]: api/classQAccelPlot_1_1Axis.md#property-datamin-12
[data-max]: api/classQAccelPlot_1_1Axis.md#property-datamax-12
[set-data-vectors]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-22
