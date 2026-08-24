#pragma once

#include <cstdint>

namespace accurate_block_placement {

struct BlockPos {
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t z{};
};

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

enum class Face : std::uint8_t {
    Down = 0,
    Up = 1,
    North = 2,
    South = 3,
    West = 4,
    East = 5,
};

struct InteractionResultValue {
    std::uint8_t value{};
};

Face chooseAccurateFace(const BlockPos& block, const Vec3& hit, Face vanillaFace);
bool shouldAdjustFace(const BlockPos& block, const Vec3& hit);
Vec3 worldToLocal(const BlockPos& block, const Vec3& hit);

} // namespace accurate_block_placement
