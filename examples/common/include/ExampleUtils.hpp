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
void setupScreenshotHandler(QGuiApplication& app, QQmlApplicationEngine& engine, int delayMs = 2000);

} // namespace QAccelPlotExample
