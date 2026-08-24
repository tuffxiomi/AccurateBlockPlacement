#include "GameHook.hpp"

#include "Placement.hpp"
#include "Signatures.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace accurate_block_placement::game_hook {
namespace {

using UseItemOnFn = InteractionResultValue (*)(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent);

UseItemOnFn gGameModeOriginal = nullptr;
UseItemOnFn gSurvivalOriginal = nullptr;

pl::memory::HookHandle gGameModeHook;
pl::memory::HookHandle gSurvivalHook;

std::mutex gInstallMutex;
bool gInstalled = false;

InteractionResultValue gameModeUseItemOnDetour(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent) {

    std::uint8_t correctedFace = face;

    if (firstEvent && position && hit) {
        const auto* blockPos = reinterpret_cast<const BlockPos*>(position);
        const auto* hitPos = reinterpret_cast<const Vec3*>(hit);

        correctedFace = static_cast<std::uint8_t>(
            chooseAccurateFace(*blockPos, *hitPos, static_cast<Face>(face)));
    }

    return gGameModeOriginal
        ? gGameModeOriginal(gameMode, item, position, correctedFace, hit, block, firstEvent)
        : InteractionResultValue{};
}

InteractionResultValue survivalModeUseItemOnDetour(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent) {

    std::uint8_t correctedFace = face;

    if (firstEvent && position && hit) {
        const auto* blockPos = reinterpret_cast<const BlockPos*>(position);
        const auto* hitPos = reinterpret_cast<const Vec3*>(hit);

        correctedFace = static_cast<std::uint8_t>(
            chooseAccurateFace(*blockPos, *hitPos, static_cast<Face>(face)));
    }

    return gSurvivalOriginal
        ? gSurvivalOriginal(gameMode, item, position, correctedFace, hit, block, firstEvent)
        : InteractionResultValue{};
}

bool installOne(std::string_view signature, void* detour, void** original, pl::memory::HookHandle& handle) {
    const auto address = pl::memory::resolveSignature(signature, "libminecraftpe.so");
    if (!address) return false;

    handle = pl::memory::HookHandle(
        reinterpret_cast<void*>(address),
        detour,
        original,
        pl::memory::HookPriority::Normal);

    return handle.installed();
}

} // namespace

bool install() {
    std::lock_guard lock(gInstallMutex);
    if (gInstalled) return true;

    const bool gameModeOk = installOne(
        signatures::kGameModeUseItemOn,
        reinterpret_cast<void*>(gameModeUseItemOnDetour),
        reinterpret_cast<void**>(&gGameModeOriginal),
        gGameModeHook);

    const bool survivalOk = installOne(
        signatures::kSurvivalModeUseItemOn,
        reinterpret_cast<void*>(survivalModeUseItemOnDetour),
        reinterpret_cast<void**>(&gSurvivalOriginal),
        gSurvivalHook);

    // At least one mode must hook. In practice both resolve on supported
    // Bedrock builds.
    gInstalled = gameModeOk || survivalOk;
    return gInstalled;
}

void uninstall() {
    std::lock_guard lock(gInstallMutex);
    gGameModeHook.reset();
    gSurvivalHook.reset();
    gGameModeOriginal = nullptr;
    gSurvivalOriginal = nullptr;
    gInstalled = false;
}

} // namespace accurate_block_placement::game_hook
