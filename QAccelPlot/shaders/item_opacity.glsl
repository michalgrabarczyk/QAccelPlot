//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Opacity of one item while a transition fades it, matching QAccelPlot::Internal::ItemClock
// in TransitionFrame.cpp, and under a FadeTrail effect.
// Requires: a `ubuf` uniform block with these members, laid out as
// QAccelPlot::Internal::ItemOpacityUniforms at a 16-byte aligned offset:
//     vec4 itemFadeGroups[28]; // two groups of 14: range, timing, 8 of opacity curve, 4 of front starts
//     vec4 itemFadeTrail;      // FadeTrail length, minimum opacity, and item count

const int kItemFadeGroupSize = 14;
const int kItemFadeCurveSamples = 32;
const int kItemFadeStartSamples = 16;

// Returns `samples` evenly spaced values, stored from vec4 `base` of the group array, interpolated at `position` in [0, 1].
float itemFadeSample(int base, int samples, float position) {
    float scaled = clamp(position, 0.0, 1.0) * float(samples - 1);
    int lower = min(int(floor(scaled)), samples - 2);
    float a = ubuf.itemFadeGroups[base + lower / 4][lower % 4];
    float b = ubuf.itemFadeGroups[base + (lower + 1) / 4][(lower + 1) % 4];
    return mix(a, b, scaled - float(lower));
}

// Returns the opacity group `group` gives item `index`, or -1 when the item is not in the group.
float itemFadeGroupOpacity(int group, float index) {
    int base = group * kItemFadeGroupSize;
    vec4 range = ubuf.itemFadeGroups[base];
    float count = range.y;
    float item = index - range.x;
    if (count < 0.5 || item < 0.0 || item >= count) {
        return -1.0;
    }
    bool exiting = range.w > 0.5;
    // Removed items leave the last one first.
    float order = exiting ? count - 1.0 - item : item;
    vec4 timing = ubuf.itemFadeGroups[base + 1];
    float start = count > 1.5 ? timing.y * order / (count - 1.0) : 0.0;
    if (range.z > 0.5) {
        // A removed item starts so that it leaves when the front passes it.
        float share = (order + (exiting ? 1.0 : 0.0)) / count;
        start = timing.y * itemFadeSample(base + 10, kItemFadeStartSamples, share);
    }
    float local = timing.z > 0.0 ? clamp((timing.x - start) / timing.z, 0.0, 1.0) : 1.0;
    return clamp(itemFadeSample(base + 2, kItemFadeCurveSamples, local), 0.0, 1.0);
}

// Returns the opacity a running transition gives item `index`: 1 outside the fading groups.
float itemTransitionOpacity(float index) {
    float opacity = itemFadeGroupOpacity(0, index);
    if (opacity < 0.0) {
        opacity = itemFadeGroupOpacity(1, index);
    }
    return opacity < 0.0 ? 1.0 : opacity;
}

// Returns the opacity a FadeTrail gives item `index`: the oldest items fade, the first one most.
float itemTrailOpacity(float index) {
    float length = ubuf.itemFadeTrail.x;
    float count = ubuf.itemFadeTrail.z;
    if (length <= 0.0 || count < 2.0) {
        return 1.0;
    }
    return mix(ubuf.itemFadeTrail.y, 1.0, clamp(index / (length * (count - 1.0)), 0.0, 1.0));
}

// Returns the opacity of item `index`.
float itemOpacity(float index) {
    return itemTransitionOpacity(index) * itemTrailOpacity(index);
}
