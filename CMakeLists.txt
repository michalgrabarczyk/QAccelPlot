#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
cmake_minimum_required(VERSION 3.16)

project(QAccelPlot
    VERSION 0.1.0
    DESCRIPTION "High-performance Qt Quick plotting library"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Options
# Default to ON only when QAccelPlot is the top-level project so that consumers
# embedding it via add_subdirectory / FetchContent don't build examples or
# generate install rules unless they explicitly opt in.
# ---------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(_QACCELPLOT_DEFAULT ON)
else()
    set(_QACCELPLOT_DEFAULT OFF)
endif()

option(QACCELPLOT_BUILD_EXAMPLES "Build the QAccelPlot example applications" ${_QACCELPLOT_DEFAULT})
option(QACCELPLOT_INSTALL        "Generate install targets for QAccelPlot"   ${_QACCELPLOT_DEFAULT})
option(QACCELPLOT_DEPLOY_EXAMPLES "Deploy Qt runtime files beside Windows examples" ON)

# ---------------------------------------------------------------------------
# Language standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS_DEBUG   " /Od /Zi")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /Zi /Zo")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE    " /DEBUG")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE " /DEBUG")
else()
    string(APPEND CMAKE_CXX_FLAGS_DEBUG   " -O0 -g")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Quick ShaderTools)

# Qt 6.11 packages private modules as separately discoverable components.
# Earlier Qt releases expose Qt6::QuickPrivate when the public Quick component
# is loaded, and do not provide a standalone QuickPrivate package. Avoid even
# probing for the private component when the user explicitly disables it.
set(_QACCELPLOT_QT_PRIVATE_API_AVAILABLE OFF)
if(TARGET Qt6::QuickPrivate)
    set(_QACCELPLOT_QT_PRIVATE_API_AVAILABLE ON)
elseif((NOT DEFINED QACCELPLOT_USE_QT_PRIVATE_API OR QACCELPLOT_USE_QT_PRIVATE_API)
       AND Qt6_VERSION VERSION_GREATER_EQUAL 6.11)
    find_package(Qt6 QUIET COMPONENTS QuickPrivate)
    if(TARGET Qt6::QuickPrivate)
        set(_QACCELPLOT_QT_PRIVATE_API_AVAILABLE ON)
    endif()
endif()

option(QACCELPLOT_USE_QT_PRIVATE_API
    "Use Qt QuickPrivate for optimized live-data texture updates"
    ${_QACCELPLOT_QT_PRIVATE_API_AVAILABLE}
)

if(QACCELPLOT_USE_QT_PRIVATE_API AND NOT TARGET Qt6::QuickPrivate)
    message(FATAL_ERROR
        "QACCELPLOT_USE_QT_PRIVATE_API is ON, but Qt6::QuickPrivate is not available. "
        "Install the matching Qt private development package or configure with "
        "-DQACCELPLOT_USE_QT_PRIVATE_API=OFF."
    )
endif()

if(Qt6_VERSION VERSION_LESS 6.3)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
else()
    qt_standard_project_setup(REQUIRES 6.2)
endif()

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------
add_subdirectory(QAccelPlot)

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(QACCELPLOT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
option(QACCELPLOT_BUILD_TESTS "Build QAccelPlot unit tests" ${_QACCELPLOT_DEFAULT})
if(QACCELPLOT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(QAccelPlot/test)
    if(QACCELPLOT_BUILD_EXAMPLES)
        add_subdirectory(examples/test)
    endif()
    if(QACCELPLOT_BUILD_BENCHMARKS)
        add_subdirectory(benchmarks/test)
    endif()
endif()

# ---------------------------------------------------------------------------
# Benchmarks (opt-in)
# ---------------------------------------------------------------------------
option(QACCELPLOT_BUILD_BENCHMARKS "Build QAccelPlot benchmarks" ${_QACCELPLOT_DEFAULT})
if(QACCELPLOT_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ---------------------------------------------------------------------------
# Documentation (optional)
# Enable with: cmake -DQACCELPLOT_BUILD_DOCS=ON ..
# Then build:  cmake --build . --target qaccelplot_docs
# ---------------------------------------------------------------------------
option(QACCELPLOT_BUILD_DOCS "Generate the MkDocs guide and Doxygen API reference" OFF)
if(QACCELPLOT_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)

    add_custom_target(qaccelplot_docs
        COMMAND "${CMAKE_COMMAND}" -E env
            "QACCELPLOT_DOXYGEN=$<TARGET_FILE:Doxygen::doxygen>"
            "QACCELPLOT_DOCS_VERSION=${PROJECT_VERSION}"
            "QACCELPLOT_DOCS_CHANNEL=local"
            "QACCELPLOT_DOCS_TITLE=QAccelPlot v${PROJECT_VERSION} Documentation"
            "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build-docs.py" build
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating the QAccelPlot documentation site with MkDocs and Doxygen"
        VERBATIM
    )
endif()
