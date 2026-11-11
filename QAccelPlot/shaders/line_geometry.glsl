//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Shared ribbon geometry for line vertex shaders.
// Requires: a `ubuf` uniform block with matrix, domainMin, domainMax,
// viewportSize, lineWidth, logScaleX, logScaleY, pointCount,
// antialiasingEnabled, antialiasingFeather, stepMode and glowParams members,
// followed by includes of data_texture.glsl and math_utils.glsl.
//
// Samples are read as LINE_SAMPLE_STRIDE floats each, X first. LINE_Y_BITS(base)
// returns the bits of the drawn Y for the sample starting at float `base`. Define
// both before including this file to read other layouts.

#ifndef LINE_SAMPLE_STRIDE
#define LINE_SAMPLE_STRIDE 2
#endif
#ifndef LINE_Y_BITS
#define LINE_Y_BITS(base) fetchFloatBits((base) + 1)
#endif

#include "data_mapping.glsl"
#include "glow.glsl"

struct LineVertexResult {
    vec4 position;     // clip-space position
    vec2 dataPosition; // untransformed data-space position of the sample
    float validWeight; // 1.0 for valid samples, kInvalidVertexWeight otherwise
    float extentScale; // share of lineRibbonHalfExtent() the vertex reaches; below 1 inside a tight bend
};

// Returns true when fragment antialiasing applies to the ribbon.
bool lineAntialiased() {
    return ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0;
}

// Half-width of the rasterized ribbon in pixels. With antialiasing, the ribbon
// is at least 1 px wide and grows by half the feather on each side, so the
// coverage ramp centred on the line edge is never clipped by the geometry. A glow
// widens it by its radius.
float lineRibbonHalfExtent() {
    float glow = glowActive(ubuf.glowParams.xy) ? ubuf.glowParams.x : 0.0;
    if (!lineAntialiased()) {
        return ubuf.lineWidth * 0.5 + glow;
    }
    return max(ubuf.lineWidth, 1.0) * 0.5 + ubuf.antialiasingFeather * 0.5 + glow;
}

// Fetches sample `index` (clamped to the data range) and returns whether it is
// valid: both coordinates finite and strictly positive on log-scale dimensions.
// Log-scale dimensions are never origin-shifted, so the sign test is exact.
bool fetchSample(int index, out vec2 position) {
    index = clamp(index, 0, int(ubuf.pointCount) - 1);
    int base = index * LINE_SAMPLE_STRIDE;
    uint xBits = fetchFloatBits(base);
    uint yBits = LINE_Y_BITS(base);
    position = vec2(uintBitsToFloat(xBits), uintBitsToFloat(yBits));
    if (!isFiniteBits(xBits) || !isFiniteBits(yBits)) {
        return false;
    }
    return (ubuf.logScaleX < 0.5 || position.x > 0.0) && (ubuf.logScaleY < 0.5 || position.y > 0.0);
}

// Step modes, matching QAccelPlot::StepMode.
const int kStepNone = 0;
const int kStepBefore = 1;
const int kStepAfter = 2;
const int kStepCenter = 3;

int lineStepMode() {
    return int(ubuf.stepMode + 0.5);
}

// Path points per sample: the sample and the step corners after it.
int lineStepCornerCount() {
    int mode = lineStepMode();
    if (mode == kStepCenter) {
        return 3;
    }
    return mode == kStepNone ? 1 : 2;
}

// Fetches the X of sample `index` and returns whether it can place a step
// corner: finite, and positive on a log-scale X axis.
bool fetchStepX(int index, out float x) {
    uint bits = fetchFloatBits(clamp(index, 0, int(ubuf.pointCount) - 1) * LINE_SAMPLE_STRIDE);
    x = uintBitsToFloat(bits);
    return isFiniteBits(bits) && (ubuf.logScaleX < 0.5 || x > 0.0);
}

// Fetches path point `corner` of sample `index`. Corner 0 is the sample; the
// others are the corners of the step to the next sample. A corner holds the Y
// of the sample whose flat step it ends or starts, and is valid when that
// sample is valid and the X values placing the corner are usable. The last
// sample has no step after it, so its corners repeat it. Matches
// QAccelPlot::Internal::linePathPoint().
bool fetchPathPoint(int index, int corner, out vec2 position) {
    if (corner == 0 || index < 0 || index >= int(ubuf.pointCount) - 1) {
        return fetchSample(index, position);
    }
    int mode = lineStepMode();
    bool ownerIsNext = mode == kStepBefore || corner == 2;
    vec2 owner;
    bool valid = fetchSample(ownerIsNext ? index + 1 : index, owner);
    float fromX;
    float toX;
    bool fromUsable = fetchStepX(index, fromX);
    bool toUsable = fetchStepX(index + 1, toX);
    float x;
    if (mode == kStepAfter) {
        x = toX;
        valid = valid && toUsable;
    } else if (mode == kStepBefore) {
        x = fromX;
        valid = valid && fromUsable;
    } else {
        // Halfway on screen: the geometric mean on a log axis, which is never origin-shifted.
        x = ubuf.logScaleX > 0.5 ? sqrt(fromX) * sqrt(toX) : 0.5 * (fromX + toX);
        valid = valid && fromUsable && toUsable;
    }
    position = vec2(x, owner.y);
    return valid;
}

// Places both ribbon vertices of an invalid sample on the centre of an adjacent
// valid sample, or outside the view volume inside an invalid run. This keeps
// triangles within a gap degenerate and avoids NaN clip positions.
LineVertexResult invalidLineVertex(vec2 position, bool previousValid, vec2 previous, bool nextValid, vec2 next) {
    LineVertexResult result;
    result.dataPosition = position;
    result.validWeight = kInvalidVertexWeight;
    result.extentScale = 1.0;
    if (previousValid) {
        result.position = ubuf.matrix * vec4(dataToLocal(previous), 0.0, 1.0);
    } else if (nextValid) {
        result.position = ubuf.matrix * vec4(dataToLocal(next), 0.0, 1.0);
    } else {
        result.position = kCulledClipPosition;
    }
    return result;
}

// Returns the path point before point `corner` of sample `index`, as a sample and a corner.
void previousPathPoint(int index, int corner, out int previousIndex, out int previousCorner) {
    previousIndex = corner > 0 ? index : index - 1;
    previousCorner = corner > 0 ? corner - 1 : lineStepCornerCount() - 1;
}

// Returns the path point after point `corner` of sample `index`, as a sample and a corner.
void nextPathPoint(int index, int corner, out int nextIndex, out int nextCorner) {
    int cornerCount = lineStepCornerCount();
    nextIndex = corner < cornerCount - 1 ? index : index + 1;
    nextCorner = corner < cornerCount - 1 ? corner + 1 : 0;
}

// Fetches path point `corner` of sample `index` in item pixels, or returns `fallback` when it is invalid.
vec2 fetchLocalPathPoint(int index, int corner, vec2 fallback) {
    vec2 point;
    return fetchPathPoint(index, corner, point) ? dataToLocal(point) : fallback;
}

// Returns the unit direction from `from` to `to`, or zero when they coincide.
vec2 lineDirection(vec2 from, vec2 to) {
    vec2 direction = to - from;
    float len = length(direction);
    return len > 0.0001 ? direction / len : vec2(0.0);
}

// Returns the unit miter normal at the join of the segment arriving along `dir1` and the one
// leaving along `dir2`; a zero direction marks a line end.
vec2 lineJoinNormal(vec2 dir1, vec2 dir2) {
    if (dot(dir1, dir1) == 0.0) dir1 = dir2;
    if (dot(dir2, dir2) == 0.0) dir2 = dir1;
    vec2 tangent = dir1 + dir2;
    // Safe fallback to prevent NaN
    tangent = length(tangent) > 0.0001 ? normalize(tangent) : vec2(1.0, 0.0);
    return vec2(-tangent.y, tangent.x);
}

// Returns how far the ribbon can reach from `p` along `direction` before it crosses the miter line
// of the neighbor join at `q` with normal `qNormal`. Inside a bend, the miter lines of consecutive
// joins meet near the centre of the bend; ribbon quads that reach past it fold over each other.
float lineBisectorReach(vec2 p, vec2 direction, vec2 q, vec2 qNormal) {
    float denominator = direction.x * qNormal.y - direction.y * qNormal.x;
    if (abs(denominator) < 0.0001) {
        return 1.0e30;
    }
    vec2 toQ = q - p;
    float reach = (toQ.x * qNormal.y - toQ.y * qNormal.x) / denominator;
    return reach > 0.0 ? reach : 1.0e30;
}

// Returns the miter normal of the join at `q`, path point `qCorner` of sample `qIndex`, next to
// `p`; `pFollows` is true when `p` comes after `q` on the path.
vec2 neighborJoinNormal(int qIndex, int qCorner, vec2 q, vec2 p, bool pFollows) {
    int otherIndex;
    int otherCorner;
    if (pFollows) {
        previousPathPoint(qIndex, qCorner, otherIndex, otherCorner);
    } else {
        nextPathPoint(qIndex, qCorner, otherIndex, otherCorner);
    }
    vec2 other = fetchLocalPathPoint(otherIndex, otherCorner, q);
    return pFollows ? lineJoinNormal(lineDirection(other, q), lineDirection(q, p))
                    : lineJoinNormal(lineDirection(p, q), lineDirection(q, other));
}

// Computes the miter-joined ribbon vertex on `side` (+1 or -1) of path point
// `corner` of sample `index`. A valid point next to an invalid neighbor is
// treated as a line endpoint.
LineVertexResult computeLineVertex(int index, int corner, float side) {
    // The corners of the last sample collapse onto it, so they add no triangles.
    if (index >= int(ubuf.pointCount) - 1) {
        corner = 0;
    }
    int previousIndex;
    int previousCorner;
    int nextIndex;
    int nextCorner;
    previousPathPoint(index, corner, previousIndex, previousCorner);
    nextPathPoint(index, corner, nextIndex, nextCorner);

    vec2 p = vec2(0.0);
    vec2 pr;
    vec2 nx;
    // Vertices past the last sample, reserved for appended data, collapse onto the last sample.
    bool pValid = index < int(ubuf.pointCount) && fetchPathPoint(index, corner, p);
    bool prValid = fetchPathPoint(previousIndex, previousCorner, pr);
    bool nxValid = fetchPathPoint(nextIndex, nextCorner, nx);

    if (!pValid) {
        return invalidLineVertex(p, prValid, pr, nxValid, nx);
    }
    if (!prValid) pr = p;
    if (!nxValid) nx = p;

    vec2 p_local = dataToLocal(p);
    vec2 prev_local = dataToLocal(pr);
    vec2 next_local = dataToLocal(nx);

    // Direction vectors for the miter join; a zero direction marks a line end.
    vec2 dir1 = lineDirection(prev_local, p_local);
    vec2 dir2 = lineDirection(p_local, next_local);
    vec2 normal = lineJoinNormal(dir1, dir2);

    // Miter length
    vec2 arriving = dot(dir1, dir1) > 0.0 ? dir1 : dir2;
    float miterDot = dot(normal, vec2(-arriving.y, arriving.x));
    float miterLength = 1.0;
    if (abs(miterDot) > 0.15) {
        miterLength = 1.0 / abs(miterDot);
    }
    // Offset in item-local pixel space, then use Qt's matrix to project. A glow widens the ribbon by
    // its radius; long miters at sharp turns would stretch that part into spikes, so it is not mitered.
    float glow = glowActive(ubuf.glowParams.xy) ? ubuf.glowParams.x : 0.0;
    float coreReach = (lineRibbonHalfExtent() - glow) * miterLength;
    float reach = coreReach + glow;
    vec2 direction = normal * side;
    if (glow > 0.0) {
        // Inside a bend tighter than the glow, the halo stops where the neighbor quads begin, so
        // halos don't stack; the line itself keeps its width.
        if (prValid) {
            vec2 previousNormal = neighborJoinNormal(previousIndex, previousCorner, prev_local, p_local, true);
            reach = min(reach, max(coreReach, lineBisectorReach(p_local, direction, prev_local, previousNormal)));
        }
        if (nxValid) {
            vec2 nextNormal = neighborJoinNormal(nextIndex, nextCorner, next_local, p_local, false);
            reach = min(reach, max(coreReach, lineBisectorReach(p_local, direction, next_local, nextNormal)));
        }
    }

    LineVertexResult result;
    result.position = ubuf.matrix * vec4(p_local + direction * reach, 0.0, 1.0);
    result.dataPosition = p;
    result.validWeight = 1.0;
    result.extentScale = reach / (coreReach + glow);
    return result;
}
