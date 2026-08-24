#pragma once

#include <cstdint>
#include <string_view>

namespace accurate_block_placement::signatures {

inline constexpr std::string_view kGameModeUseItemOn =
    "? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 "
    "? ? ? D1 ? ? ? F9 5C D0 3B D5 F8 03 00 AA";

inline constexpr std::string_view kSurvivalModeUseItemOn =
    "? ? ? 39 ? ? ? 34 ? ? ? F0 ? ? ? 39 ? ? ? 34 ? ? ? A9 FD 03 00 91 "
    "E1 03 1F 2A ? ? ? 97 E0 03 1F 2A ? ? ? A8 C0 03 5F D6 ? ? ? 12";

} // namespace accurate_block_placement::signatures
