#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
file(GLOB_RECURSE QACCELPLOT_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS
    "src/*.hpp"
    "src/*.cpp"
)
list(FILTER QACCELPLOT_SOURCES EXCLUDE REGEX "^src/shaders/.*")

# Preserve directory structure in IDE project views
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${QACCELPLOT_SOURCES})

file(GLOB QACCELPLOT_SHADER_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS
    "src/shaders/*.vert"
    "src/shaders/*.frag"
)

file(GLOB QACCELPLOT_QML_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS
    "qml/*.qml"
)

set_source_files_properties(qml/Colors.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties(qml/LegendSymbol.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
qt_add_library(QAccelPlot STATIC ${QACCELPLOT_SOURCES})
set_target_properties(QAccelPlot PROPERTIES EXPORT_NAME Core)
add_library(QAccelPlot::Core ALIAS QAccelPlot)

qt_add_shaders(QAccelPlot "shaders"
    PRECOMPILE
    OPTIMIZED
    GLSL "300 es,330"
    HLSL "50"
    MSL "12"
    PREFIX "/qaccelplot"
    FILES
        ${QACCELPLOT_SHADER_FILES}
)

foreach(QML_FILE ${QACCELPLOT_QML_FILES})
    get_filename_component(QML_FILENAME ${QML_FILE} NAME)
    set_source_files_properties(${QML_FILE}
        PROPERTIES QT_RESOURCE_ALIAS ${QML_FILENAME}
    )
endforeach()

qt_add_qml_module(QAccelPlot
    OUTPUT_TARGETS _qaccelplot_qml_targets
    URI QAccelPlot
    VERSION 1.0
    DEPENDENCIES QtQuick
    RESOURCE_PREFIX /qt-project.org/imports
    QML_FILES
        ${QACCELPLOT_QML_FILES}
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlot"
)

# QAccelPlotplugin is created by qt_add_qml_module above; alias it into the
# QAccelPlot:: namespace so consumers can reference it consistently.
add_library(QAccelPlot::QAccelPlotplugin ALIAS QAccelPlotplugin)

# Present the complete static QML module as the canonical consumer target.
# The backing library and plugin remain separate internally,
# while applications receive their combined usage requirements through a single target.
add_library(QAccelPlotInterface INTERFACE)
set_target_properties(QAccelPlotInterface PROPERTIES EXPORT_NAME QAccelPlot)
target_link_libraries(QAccelPlotInterface INTERFACE
    QAccelPlot
    QAccelPlotplugin
)
add_library(QAccelPlot::QAccelPlot ALIAS QAccelPlotInterface)

target_link_libraries(QAccelPlot
    PUBLIC Qt6::Core Qt6::Gui Qt6::Quick
)

if(QACCELPLOT_USE_QT_PRIVATE_API)
    # Used only by the internal data-texture upload implementation. No private
    # Qt types are present in QAccelPlot's public headers or ABI.
    target_link_libraries(QAccelPlot PRIVATE Qt6::QuickPrivate)
    target_compile_definitions(QAccelPlot PRIVATE QACCELPLOT_USE_QT_PRIVATE_API=1)
else()
    target_compile_definitions(QAccelPlot PRIVATE QACCELPLOT_USE_QT_PRIVATE_API=0)
endif()

# Public interface: only the src/ root, so consumers use src/-relative paths.
# Private subdirectory entries are required by the auto-generated
# qaccelplot_qmltyperegistrations.cpp, which Qt emits with bare #include <Foo.hpp>.
target_include_directories(QAccelPlot
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QAccelPlot>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/annotations>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/axis>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/effects>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/formatters>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/grid>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/linestyles>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/materials>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/renderers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/series>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/shapes>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/transitions>
)

# ---------------------------------------------------------------------------
# Install rules (opt-in via QACCELPLOT_INSTALL)
# ---------------------------------------------------------------------------
if(QACCELPLOT_INSTALL)
    # Include Qt's complete list of QML helper targets, including the separate
    # QML cache object library generated by Qt 6.2.
    set(_qaccelplot_aux_targets ${_qaccelplot_qml_targets})
    # Older qt_add_shaders versions do not expose OUTPUT_TARGETS. Collect its
    # resource libraries as well, then remove overlaps with the QML helpers.
    foreach(_n RANGE 1 10)
        if(TARGET "QAccelPlot_resources_${_n}")
            list(APPEND _qaccelplot_aux_targets "QAccelPlot_resources_${_n}")
        endif()
    endforeach()
    if(TARGET QAccelPlotplugin_init)
        list(APPEND _qaccelplot_aux_targets QAccelPlotplugin_init)
    endif()
    list(REMOVE_DUPLICATES _qaccelplot_aux_targets)

    # --- Binaries ---
    install(TARGETS QAccelPlotInterface QAccelPlot QAccelPlotplugin ${_qaccelplot_aux_targets}
        EXPORT QAccelPlotTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        OBJECTS DESTINATION "${CMAKE_INSTALL_LIBDIR}/objects"
    )

    # --- Headers (preserves subdirectory structure) ---
    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QAccelPlot"
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "internal" EXCLUDE
        PATTERN "shaders" EXCLUDE
    )

    # --- QML module directory (qmldir, typeinfo, QML files) ---
    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlot/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/qml/QAccelPlot"
        FILES_MATCHING
            PATTERN "qmldir"
            PATTERN "*.qmltypes"
            PATTERN "*.qml"
    )

    # --- CMake export targets ---
    install(EXPORT QAccelPlotTargets
        FILE      QAccelPlotTargets.cmake
        NAMESPACE QAccelPlot::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QAccelPlot"
    )

    # --- Package config + version file ---
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/QAccelPlotConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlotConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QAccelPlot"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlotConfigVersion.cmake"
        VERSION     ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlotConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/QAccelPlotConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QAccelPlot"
    )

    # --- License and licensing information ---
    install(FILES
        "${PROJECT_SOURCE_DIR}/LICENSE"
        "${PROJECT_SOURCE_DIR}/LICENSING.md"
        "${PROJECT_SOURCE_DIR}/COMMERCIAL-LICENSING.md"
        "${PROJECT_SOURCE_DIR}/LICENSES/GPL-3.0-only.txt"
        "${PROJECT_SOURCE_DIR}/LICENSES/Universal-FOSS-exception-1.0.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/QAccelPlot"
    )
endif()
