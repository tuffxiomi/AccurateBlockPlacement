#include "GameHook.hpp"

#include "Placement.hpp"
#include "Signatures.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string_view>

namespace accurate_block_placement::game_hook {

namespace {

struct InteractionResultValue {
    std::uint8_t value;
};

using UseItemOnFn = InteractionResultValue (*)(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent
);

UseItemOnFn gGameModeOriginal = nullptr;
UseItemOnFn gSurvivalOriginal = nullptr;

pl::memory::HookHandle gGameModeHook;
pl::memory::HookHandle gSurvivalHook;

std::mutex gInstallMutex;
std::atomic_bool gInstalled{false};

InteractionResultValue gameModeUseItemOnDetour(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent
) {
    std::uint8_t correctedFace = face;

    /*
     * Only alter the placement face for an actual placement event.
     *
     * Keep this path intentionally small:
     * - no signature scanning
     * - no dynamic loading
     * - no allocations
     * - no logging
     */
    if (firstEvent && position && hit) {
        const auto* blockPos =
            reinterpret_cast<const BlockPos*>(position);

        const auto* hitPos =
            reinterpret_cast<const Vec3*>(hit);

        correctedFace = static_cast<std::uint8_t>(
            chooseAccurateFace(
                *blockPos,
                *hitPos,
                static_cast<Face>(face)
            )
        );
    }

    if (gGameModeOriginal) {
        return gGameModeOriginal(
            gameMode,
            item,
            position,
            correctedFace,
            hit,
            block,
            firstEvent
        );
    }

    return InteractionResultValue{};
}

InteractionResultValue survivalModeUseItemOnDetour(
    void* gameMode,
    void* item,
    const void* position,
    std::uint8_t face,
    const void* hit,
    const void* block,
    bool firstEvent
) {
    std::uint8_t correctedFace = face;

    if (firstEvent && position && hit) {
        const auto* blockPos =
            reinterpret_cast<const BlockPos*>(position);

        const auto* hitPos =
            reinterpret_cast<const Vec3*>(hit);

        correctedFace = static_cast<std::uint8_t>(
            chooseAccurateFace(
                *blockPos,
                *hitPos,
                static_cast<Face>(face)
            )
        );
    }

    if (gSurvivalOriginal) {
        return gSurvivalOriginal(
            gameMode,
            item,
            position,
            correctedFace,
            hit,
            block,
            firstEvent
        );
    }

    return InteractionResultValue{};
}

bool installOne(
    std::string_view signature,
    void* detour,
    void** original,
    pl::memory::HookHandle& handle
) {
    /*
     * Signature resolution happens only after
     * libminecraftpe.so is known to be loaded.
     */
    const auto address =
        pl::memory::resolveSignature(
            signature,
            "libminecraftpe.so"
        );

    if (!address) {
        return false;
    }

    handle = pl::memory::HookHandle(
        reinterpret_cast<void*>(address),
        detour,
        original,
        pl::memory::HookPriority::Normal
    );

    return handle.installed();
}

} // namespace

bool install() {
    std::lock_guard lock(gInstallMutex);

    if (gInstalled.load(std::memory_order_acquire)) {
        return true;
    }

    const bool gameModeOk = installOne(
        signatures::kGameModeUseItemOn,
        reinterpret_cast<void*>(
            gameModeUseItemOnDetour
        ),
        reinterpret_cast<void**>(
            &gGameModeOriginal
        ),
        gGameModeHook
    );

    const bool survivalOk = installOne(
        signatures::kSurvivalModeUseItemOn,
        reinterpret_cast<void*>(
            survivalModeUseItemOnDetour
        ),
        reinterpret_cast<void**>(
            &gSurvivalOriginal
        ),
        gSurvivalHook
    );

    const bool installed =
        gameModeOk || survivalOk;

    gInstalled.store(
        installed,
        std::memory_order_release
    );

    return installed;
}

void uninstall() {
    std::lock_guard lock(gInstallMutex);

    if (gGameModeHook.installed()) {
        gGameModeHook.reset();
    }

    if (gSurvivalHook.installed()) {
        gSurvivalHook.reset();
    }

    gGameModeOriginal = nullptr;
    gSurvivalOriginal = nullptr;

    gInstalled.store(
        false,
        std::memory_order_release
    );
}

} // namespace accurate_block_placement::game_hook
