//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QString>
#include <QStringList>

#include <functional>
#include <optional>

namespace QAccelPlotExample {

void configureGraphicsApi();
void setupEngineFailureHandler(QGuiApplication& app, QQmlApplicationEngine& engine);
// Handles the visual test options after the QML engine has loaded the example:
//   --page <name>        Shows the page whose button in the "examplePages" TabBar has this objectName.
//   --screenshot <path>  Saves a screenshot and <path>.rhi.json render metadata, then quits.
void setupScreenshotHandler(QGuiApplication& app, QQmlApplicationEngine& engine, int delayMs = 2000);

// Returns true for --theme light and false for --theme dark or no --theme option.
bool lightThemeRequested(const QStringList& arguments);

// Frame recording options: --record <dir> --frames <n> --fps <n>.
struct RecordingOptions {
    QString directory;       // Receives frame_00000.png, frame_00001.png, ...; older frame_*.png files there are removed.
    int frameCount{0};       // Number of frames to record.
    int framesPerSecond{25}; // Simulated time and Qt's animation clock advance exactly 1 / framesPerSecond per frame.
};

// Call before creating QGuiApplication when the example supports --record. With --record, text
// uses grayscale antialiasing: subpixel antialiasing needs an opaque target, which the offscreen
// capture lacks, and would leave color fringes around glyphs. Qt Quick also uses its basic render
// loop, which, unlike the threaded one, leaves the animation clock to installRecordingClock().
void configureFrameRecording(int argc, char* argv[]);

// Returns the recording options, or std::nullopt without --record. Options missing from the
// arguments keep their value from defaults. Sets error and returns std::nullopt for invalid values.
std::optional<RecordingOptions> recordingOptions(const QStringList& arguments, const RecordingOptions& defaults, QString& error);

// Called on the GUI thread before each recorded frame. Show the scene at timeSeconds, then call
// ready on the GUI thread once the window holds everything the frame shows.
using FramePreparer = std::function<void(int frameIndex, double timeSeconds, const std::function<void()>& ready)>;

// Installs the animation clock that a recording steps frame by frame. Call it when recording, after
// creating QGuiApplication and before loading QML: Qt Quick installs its own clock with the first
// window, and the first clock installed stays in use.
void installRecordingClock();

// Records options.frameCount frames of window into options.directory, then quits the application.
// Each frame is prepared through prepareFrame and captured offscreen at the window's logical size.
// With installRecordingClock() called first, Qt's animation clock steps with the frames, so QML
// animations and data transitions advance by the frame time too. The frames do not depend on how
// fast the machine renders. Returns false when the directory cannot be created.
bool startFrameRecording(QGuiApplication& app, QQuickWindow* window, const RecordingOptions& options, FramePreparer prepareFrame);

} // namespace QAccelPlotExample
