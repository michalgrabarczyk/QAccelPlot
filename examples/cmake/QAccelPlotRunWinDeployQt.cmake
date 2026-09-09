#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#

foreach(REQUIRED_VARIABLE IN ITEMS
    QACCELPLOT_WINDEPLOYQT_EXECUTABLE
    QACCELPLOT_WINDEPLOYQT_QML_DIR
    QACCELPLOT_WINDEPLOYQT_TARGET
    QACCELPLOT_WINDEPLOYQT_LOCK
)
    if(NOT DEFINED ${REQUIRED_VARIABLE} OR "${${REQUIRED_VARIABLE}}" STREQUAL "")
        message(FATAL_ERROR "${REQUIRED_VARIABLE} must be set")
    endif()
endforeach()

file(LOCK "${QACCELPLOT_WINDEPLOYQT_LOCK}"
    GUARD PROCESS
    TIMEOUT 300
    RESULT_VARIABLE LOCK_RESULT
)
if(NOT LOCK_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Could not acquire the windeployqt lock '${QACCELPLOT_WINDEPLOYQT_LOCK}': ${LOCK_RESULT}"
    )
endif()

execute_process(
    COMMAND "${QACCELPLOT_WINDEPLOYQT_EXECUTABLE}"
        --qmldir "${QACCELPLOT_WINDEPLOYQT_QML_DIR}"
        "${QACCELPLOT_WINDEPLOYQT_TARGET}"
    RESULT_VARIABLE WINDEPLOYQT_RESULT
)
if(NOT WINDEPLOYQT_RESULT EQUAL 0)
    message(FATAL_ERROR "windeployqt exited with code ${WINDEPLOYQT_RESULT}")
endif()
