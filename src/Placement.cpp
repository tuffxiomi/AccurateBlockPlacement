#include "Placement.hpp"

#include <algorithm>
#include <cmath>

namespace accurate_block_placement {
namespace {

constexpr float kEpsilon = 0.001f;
constexpr float kEdgeThreshold = 0.18f;

float distanceToBoundary(float v) {
    const float clamped = std::clamp(v, 0.0f, 1.0f);
    return std::min(clamped, 1.0f - clamped);
}

bool inReasonableRange(float v) {
    return std::isfinite(v) && v >= -0.05f && v <= 1.05f;
}

Face faceForX(float x) {
    return x <= 0.5f ? Face::West : Face::East;
}

Face faceForY(float y) {
    return y <= 0.5f ? Face::Down : Face::Up;
}

Face faceForZ(float z) {
    return z <= 0.5f ? Face::North : Face::South;
}

} // namespace

Vec3 worldToLocal(const BlockPos& block, const Vec3& hit) {
    return {
        hit.x - static_cast<float>(block.x),
        hit.y - static_cast<float>(block.y),
        hit.z - static_cast<float>(block.z),
    };
}

bool shouldAdjustFace(const BlockPos& block, const Vec3& hit) {
    const Vec3 local = worldToLocal(block, hit);

    if (!inReasonableRange(local.x) ||
        !inReasonableRange(local.y) ||
        !inReasonableRange(local.z)) {
        return false;
    }

    const float dx = distanceToBoundary(local.x);
    const float dy = distanceToBoundary(local.y);
    const float dz = distanceToBoundary(local.z);

    return std::min({dx, dy, dz}) <= kEdgeThreshold + kEpsilon;
}

Face chooseAccurateFace(const BlockPos& block, const Vec3& hit, Face vanillaFace) {
    if (!shouldAdjustFace(block, hit)) {
        return vanillaFace;
    }

    const Vec3 local = worldToLocal(block, hit);
    const float dx = distanceToBoundary(local.x);
    const float dy = distanceToBoundary(local.y);
    const float dz = distanceToBoundary(local.z);

    // Keep the vanilla choice when the hit is not sufficiently close to a
    // block edge. This minimizes behavior changes for ordinary placements.
    if (std::min({dx, dy, dz}) > kEdgeThreshold + kEpsilon) {
        return vanillaFace;
    }

    if (dx <= dy && dx <= dz) return faceForX(local.x);
    if (dy <= dx && dy <= dz) return faceForY(local.y);
    return faceForZ(local.z);
}

} // namespace accurate_block_placement
