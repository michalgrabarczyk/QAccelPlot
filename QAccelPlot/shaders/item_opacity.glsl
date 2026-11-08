//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Opacity of one item while a transition fades it, matching
// QAccelPlot::Internal::writeTransitionFrame() and itemProgress(), and under a FadeTrail effect.
// Requires: a `ubuf` uniform block with these members, laid out as
// QAccelPlot::Internal::ItemOpacityUniforms at a 16-byte aligned offset:
//     vec4 itemFadeRange;     // first item, item count (0: no fade), progress, stagger
//     vec4 itemFadeParams;    // x: 1 when the items exit; FadeTrail length, minimum opacity, and item count
//     vec4 itemFadeEasing[4]; // the easing curve at 16 evenly spaced progress values

const int kItemFadeEasingSamples = 16;

// Progress of item `item` of `itemCount`, before easing; see itemProgress() in TransitionFrame.cpp.
float itemFadeProgress(float progress, float item, float itemCount, float stagger) {
    float clamped = clamp(progress, 0.0, 1.0);
    if (itemCount <= 1.0 || stagger <= 0.0) {
        return clamped;
    }
    float delay = stagger * item / (itemCount - 1.0);
    float span = 1.0 - stagger;
    if (span <= 0.0) {
        return clamped >= delay ? 1.0 : 0.0;
    }
    return clamp((clamped - delay) / span, 0.0, 1.0);
}

float itemFadeEased(float progress) {
    float position = clamp(progress, 0.0, 1.0) * float(kItemFadeEasingSamples - 1);
    int lower = min(int(floor(position)), kItemFadeEasingSamples - 1);
    int upper = min(lower + 1, kItemFadeEasingSamples - 1);
    float a = ubuf.itemFadeEasing[lower / 4][lower % 4];
    float b = ubuf.itemFadeEasing[upper / 4][upper % 4];
    return mix(a, b, position - float(lower));
}

// Returns the opacity a running transition gives item `index`: 1 outside the fading items.
float itemTransitionOpacity(float index) {
    float count = ubuf.itemFadeRange.y;
    float item = index - ubuf.itemFadeRange.x;
    if (count < 0.5 || item < 0.0 || item >= count) {
        return 1.0;
    }
    bool exiting = ubuf.itemFadeParams.x > 0.5;
    // Removed items leave the last one first.
    float order = exiting ? count - 1.0 - item : item;
    float eased = clamp(itemFadeEased(itemFadeProgress(ubuf.itemFadeRange.z, order, count, ubuf.itemFadeRange.w)), 0.0, 1.0);
    return exiting ? 1.0 - eased : eased;
}

// Returns the opacity a FadeTrail gives item `index`: the oldest items fade, the first one most.
float itemTrailOpacity(float index) {
    float length = ubuf.itemFadeParams.y;
    float count = ubuf.itemFadeParams.w;
    if (length <= 0.0 || count < 2.0) {
        return 1.0;
    }
    return mix(ubuf.itemFadeParams.z, 1.0, clamp(index / (length * (count - 1.0)), 0.0, 1.0));
}

// Returns the opacity of item `index`.
float itemOpacity(float index) {
    return itemTransitionOpacity(index) * itemTrailOpacity(index);
}
