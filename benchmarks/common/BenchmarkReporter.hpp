//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

/// \file BenchmarkReporter.hpp
/// \brief Formats and outputs benchmark results as JSON or a console table.

#include "BenchmarkMetrics.hpp"
#include "BenchmarkScenario.hpp"

#include <QString>

#include <vector>

namespace QAccelPlot {

/// \brief Formats benchmark results for output to JSON files or the console.
///
/// Collects results from multiple scenarios and serializes them together with
/// system-info metadata. The JSON schema is designed for machine consumption
/// (CI tooling, historical tracking), while the console table gives a quick
/// human-readable overview.
///
/// \par JSON schema
/// \code
/// {
///   "library": "QAccelPlot",
///   "version": "0.1.0",
///   "timestamp": "...",
///   "system": { "os": "...", "cpu_arch": "...", "cpu_cores": N, "qt_version": "...", "graphics_api": "..." },
///   "scenarios": [ { "name": "...", "results": { ... } }, ... ]
/// }
/// \endcode
///
/// \sa BenchmarkMetrics, BenchmarkScenario
class BenchmarkReporter {
public:
    /// \brief A completed scenario paired with its results.
    struct Entry {
        BenchmarkScenario scenario;
        BenchmarkMetrics::Results results;
    };

    /// \brief Constructs a reporter with the given library version string.
    explicit BenchmarkReporter(const QString& libraryVersion = QStringLiteral("0.1.0"));

    /// \brief Overrides the graphics API name reported in the system-info block.
    ///
    /// If not set, defaults to "Unknown". Typically set from
    /// \c QSGRendererInterface::graphicsApi() after the first frame.
    void setGraphicsApi(const QString& api);

    /// \brief Adds a completed scenario result to the report.
    void addResult(const BenchmarkScenario& scenario, const BenchmarkMetrics::Results& results);

    /// \brief Writes all results to \a filePath as a JSON document.
    /// \return \c true on success, \c false if the file could not be written.
    bool writeJson(const QString& filePath) const;

    /// \brief Returns all results as a formatted JSON string (UTF-8).
    QString toJson() const;

    /// \brief Prints a human-readable summary table to stdout.
    void printConsoleTable() const;

private:
    QString buildSystemInfoJson() const;
    QString formatNumber(double value, int decimals) const;

    QString libraryVersion_;
    QString graphicsApi_{QStringLiteral("Unknown")};
    std::vector<Entry> entries_;
};

} // namespace QAccelPlot
