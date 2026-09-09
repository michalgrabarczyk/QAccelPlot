//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

/// \file BenchmarkRunner.hpp
/// \brief QObject bridge between the benchmark engine and the QML UI.

#include "BenchmarkMetrics.hpp"
#include "BenchmarkReporter.hpp"
#include "BenchmarkScenario.hpp"
#include "ExistingDataBufferPool.hpp"
#include "QAccelPlot.hpp"
#include "series/LineCurve.hpp"

#include <QElapsedTimer>
#include <QObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickWindow>
#include <QVariantList>
#include <QVariantMap>

#include <memory>
#include <vector>

namespace QAccelPlot {

/// \brief Manages benchmark execution and exposes live metrics to QML.
///
/// BenchmarkRunner owns the offscreen Qt Quick context, runs each
/// BenchmarkScenario in sequence, and emits signals so the QML dashboard
/// can update in real time.  Results are accumulated in a BenchmarkReporter
/// and exposed as a QVariantList for binding to a QML TableView.
class BenchmarkRunner : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double liveFps READ liveFps NOTIFY liveFpsChanged)
    Q_PROPERTY(double liveFrameTimeMs READ liveFrameTimeMs NOTIFY liveFrameTimeMsChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString currentScenarioName READ currentScenarioName NOTIFY currentScenarioNameChanged)
    Q_PROPERTY(QString currentScenarioLabel READ currentScenarioLabel NOTIFY currentScenarioLabelChanged)
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)
    Q_PROPERTY(QVariantList availableScenarios READ availableScenarios NOTIFY availableScenariosChanged)
    Q_PROPERTY(QString systemInfo READ systemInfo CONSTANT)

public:
    explicit BenchmarkRunner(QObject* parent = nullptr);
    ~BenchmarkRunner() override;

    bool isRunning() const;
    double liveFps() const;
    double liveFrameTimeMs() const;
    double progress() const;
    QString currentScenarioName() const;
    QString currentScenarioLabel() const;
    QVariantList results() const;
    QVariantList availableScenarios() const;
    QString systemInfo() const;

    Q_INVOKABLE void runAll();
    Q_INVOKABLE void runScenario(const QString& name);
    Q_INVOKABLE void addCustomScenario(const QString& name, const QString& label, const QString& desc, int pointCount, int curveCount, int updateMode, int durationSeconds = 10, int warmupSeconds = 2);
    Q_INVOKABLE bool exportJson(const QString& path);
    Q_INVOKABLE void stop();

signals:
    void runningChanged();
    void liveFpsChanged();
    void liveFrameTimeMsChanged();
    void progressChanged();
    void currentScenarioNameChanged();
    void currentScenarioLabelChanged();
    void resultsChanged();
    void availableScenariosChanged();
    void scenarioCompleted(const QString& name);
    void allCompleted();

private slots:
    void requestRender();
    void render();
    void runNextScenario();
    void updateData();
    void finishScenario();

private:
    void setupScene(const BenchmarkScenario& scenario);
    void clearCurves();
    void setRunning(bool running);
    void setLiveFps(double fps);
    void setLiveFrameTimeMs(double ms);
    void setProgress(double p);
    void setCurrentScenarioName(const QString& name);
    void setCurrentScenarioLabel(const QString& label);

    // Qt Quick offscreen rendering
    QQuickRenderControl* renderControl_{nullptr};
    QQuickWindow* window_{nullptr};
    QOpenGLContext* context_{nullptr};
    QOffscreenSurface* offscreenSurface_{nullptr};
    QOpenGLFramebufferObject* fbo_{nullptr};

    // QAccelPlot scene items
    QAccelPlot* plot_{nullptr};
    Axis* xAxis_{nullptr};
    Axis* yAxis_{nullptr};
    std::vector<LineCurve*> curves_;

    // Benchmark state
    std::vector<BenchmarkScenario> availableScenarios_;
    std::vector<BenchmarkScenario> scenariosToRun_;
    size_t currentScenarioIndex_{0};
    BenchmarkMetrics metrics_;
    BenchmarkReporter reporter_;
    QElapsedTimer scenarioTimer_;
    double currentTargetDurationMs_{0};
    int currentPointCount_{0};
    int currentFrameIndex_{0};
    BenchmarkScenario::UpdateMode currentUpdateMode_{BenchmarkScenario::UpdateMode::Static};
    std::unique_ptr<ExistingDataBufferPool> dataBufferPool_;

    // Live property values
    bool running_{false};
    double liveFps_{0.0};
    double liveFrameTimeMs_ = 0.0;
    double progress_ = 0.0;
    QString currentScenarioName_;
    QString currentScenarioLabel_;
    QVariantList results_;

    bool renderRequested_{false};
    bool scenarioRunning_{false};
};

} // namespace QAccelPlot
