//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Guards against log(0) by clamping to a small positive value.
float safeLog10(float val) {
    return log(max(val, 1e-10)) / log(10.0);
}
