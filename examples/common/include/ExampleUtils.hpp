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

namespace QAccelPlotExample {

void configureGraphicsApi();
void setupEngineFailureHandler(QGuiApplication& app, QQmlApplicationEngine& engine);
// Handles the visual test options after the QML engine has loaded the example:
//   --page <name>        Shows the page whose button in the "examplePages" TabBar has this objectName.
//   --screenshot <path>  Saves a screenshot and <path>.rhi.json render metadata, then quits.
void setupScreenshotHandler(QGuiApplication& app, QQmlApplicationEngine& engine, int delayMs = 2000);

} // namespace QAccelPlotExample
