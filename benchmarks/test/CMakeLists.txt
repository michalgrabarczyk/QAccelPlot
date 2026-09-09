#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# Benchmark infrastructure unit tests.
# These test the shared workload logic; they do not execute full benchmarks.
#
include("${CMAKE_SOURCE_DIR}/cmake/QAccelPlotTestHelpers.cmake")

add_qaccelplot_test(tst_benchmark_workload tst_benchmark_workload.cpp)
target_link_libraries(tst_benchmark_workload PRIVATE QAccelPlotBenchmarkCommon)
target_include_directories(tst_benchmark_workload PRIVATE
    "${CMAKE_SOURCE_DIR}/benchmarks/common"
)
