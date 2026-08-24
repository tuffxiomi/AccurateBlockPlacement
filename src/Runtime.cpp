#include "Runtime.hpp"

#include "GameHook.hpp"

#include <dlfcn.h>

#include <chrono>
#include <thread>

namespace accurate_block_placement {

Runtime& Runtime::instance() {
    static Runtime runtime;
    return runtime;
}

Runtime::~Runtime() {
    stopWatcher();
}

bool Runtime::load() {
    return true;
}

bool Runtime::minecraftLoaded() const {
    void* handle = dlopen(
        "libminecraftpe.so",
        RTLD_NOW | RTLD_NOLOAD
    );

    if (!handle) {
        return false;
    }

    dlclose(handle);
    return true;
}

bool Runtime::installWhenReady() {
    if (!mEnabled.load(std::memory_order_acquire)) {
        return false;
    }

    if (mInstalled.load(std::memory_order_acquire)) {
        return true;
    }

    if (!minecraftLoaded()) {
        return false;
    }

    /*
     * Do NOT install hooks from a dlopen callback.
     *
     * Android's dynamic linker may still hold its loader lock while
     * libminecraftpe.so is being loaded. Performing signature scanning
     * or inline hooking from that path can deadlock the process and
     * produce a startup ANR.
     *
     * The watcher calls this after libminecraftpe.so is already loaded.
     */
    if (!game_hook::install()) {
        return false;
    }

    mInstalled.store(true, std::memory_order_release);
    return true;
}

void Runtime::watcherLoop() {
    /*
     * Give the launcher and PreLoader a short amount of time to finish
     * their normal initialization before beginning the polling loop.
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(250)
    );

    while (!mStopWatcher.load(std::memory_order_acquire)) {
        if (mEnabled.load(std::memory_order_acquire) &&
            !mInstalled.load(std::memory_order_acquire)) {

            if (installWhenReady()) {
                break;
            }
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(250)
        );
    }
}

void Runtime::startWatcher() {
    if (mWatcher.joinable()) {
        return;
    }

    mStopWatcher.store(
        false,
        std::memory_order_release
    );

    mWatcher = std::thread(
        [this]() {
            watcherLoop();
        }
    );
}

void Runtime::stopWatcher() {
    mStopWatcher.store(
        true,
        std::memory_order_release
    );

    if (mWatcher.joinable()) {
        mWatcher.join();
    }
}

bool Runtime::enable() {
    mEnabled.store(
        true,
        std::memory_order_release
    );

    /*
     * Hook installation happens on the watcher thread instead of
     * the Android loader thread.
     */
    startWatcher();

    return true;
}

bool Runtime::disable() {
    mEnabled.store(
        false,
        std::memory_order_release
    );

    /*
     * Do not remove inline hooks while Minecraft could be executing
     * through them. The detours remain installed for process lifetime
     * and should be kept extremely small.
     */
    return true;
}

bool Runtime::unload() {
    mEnabled.store(
        false,
        std::memory_order_release
    );

    stopWatcher();

    /*
     * Levi normally keeps native mods alive for the lifetime of the
     * Minecraft process. Avoid removing executable patches during
     * shutdown because another thread could still be executing them.
     */
    return true;
}

} // namespace accurate_block_placement
