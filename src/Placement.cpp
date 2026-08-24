#include "Placement.hpp"

#include <algorithm>
#include <cmath>

namespace accurate_block_placement {

namespace {

constexpr float kEpsilon = 0.001f;

float distanceToLowerFace(float value) {
    return std::fabs(value);
}

float distanceToUpperFace(float value) {
    return std::fabs(1.0f - value);
}

} // namespace

Face chooseAccurateFace(
    const BlockPos& block,
    const Vec3& hit,
    Face fallback
) {
    /*
     * Convert the world-space hit position into the block's local
     * [0, 1] coordinate space.
     */
    const float localX =
        hit.x - static_cast<float>(block.x);

    const float localY =
        hit.y - static_cast<float>(block.y);

    const float localZ =
        hit.z - static_cast<float>(block.z);

    /*
     * Keep normal placement behavior when the hit position is
     * clearly inside the face instead of near an edge.
     */
    if (localX < -kEpsilon ||
        localX > 1.0f + kEpsilon ||
        localY < -kEpsilon ||
        localY > 1.0f + kEpsilon ||
        localZ < -kEpsilon ||
        localZ > 1.0f + kEpsilon) {
        return fallback;
    }

    float bestDistance = 1000000.0f;
    Face bestFace = fallback;

    const auto consider = [&](
        float distance,
        Face face
    ) {
        if (distance < bestDistance) {
            bestDistance = distance;
            bestFace = face;
        }
    };

    consider(
        distanceToLowerFace(localY),
        Face::Down
    );

    consider(
        distanceToUpperFace(localY),
        Face::Up
    );

    consider(
        distanceToLowerFace(localZ),
        Face::North
    );

    consider(
        distanceToUpperFace(localZ),
        Face::South
    );

    consider(
        distanceToLowerFace(localX),
        Face::West
    );

    consider(
        distanceToUpperFace(localX),
        Face::East
    );

    /*
     * If the hit is not actually close enough to a boundary,
     * preserve Minecraft's original face.
     */
    constexpr float kBoundaryTolerance = 0.08f;

    if (bestDistance > kBoundaryTolerance) {
        return fallback;
    }

    return bestFace;
}

} // namespace accurate_block_placement
