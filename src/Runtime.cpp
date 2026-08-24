#include "Runtime.hpp"

#include "GameHook.hpp"

#include <pl/memory/Hook.hpp>

#include <dlfcn.h>

#include <atomic>
#include <cstring>
#include <mutex>

namespace accurate_block_placement {
namespace {

using DlopenFn = void* (*)(const char*, int);

std::mutex gRuntimeMutex;
std::atomic_bool gMinecraftReady{false};
std::atomic_bool gEnabled{false};

DlopenFn gDlopenOriginal = nullptr;
pl::memory::HookHandle gDlopenHook;

thread_local bool gResolving = false;

void* dlopenDetour(const char* filename, int flags) {
    void* handle = gDlopenOriginal ? gDlopenOriginal(filename, flags) : nullptr;

    if (handle &&
        filename &&
        !gResolving &&
        std::strstr(filename, "libminecraftpe.so") != nullptr) {
        Runtime::instance().onMinecraftLoaded();
    }

    return handle;
}

bool minecraftAlreadyLoaded() {
    void* handle = dlopen("libminecraftpe.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) return false;
    dlclose(handle);
    return true;
}

} // namespace

Runtime& Runtime::instance() {
    static Runtime runtime;
    return runtime;
}

bool Runtime::load() {
    return true;
}

bool Runtime::resolveAndInstall() {
    std::lock_guard lock(gRuntimeMutex);

    if (!gEnabled.load(std::memory_order_acquire)) return false;
    if (!minecraftAlreadyLoaded()) return false;
    if (gMinecraftReady.load(std::memory_order_acquire)) return true;

    const bool installed = game_hook::install();
    gMinecraftReady.store(installed, std::memory_order_release);
    return installed;
}

void Runtime::installDlopenHook() {
    void* target = dlsym(RTLD_DEFAULT, "dlopen");
    if (!target) return;

    gDlopenHook = pl::memory::HookHandle(
        target,
        reinterpret_cast<void*>(dlopenDetour),
        reinterpret_cast<void**>(&gDlopenOriginal),
        pl::memory::HookPriority::Low);
}

void Runtime::uninstallDlopenHook() {
    gDlopenHook.reset();
    gDlopenOriginal = nullptr;
}

void Runtime::onMinecraftLoaded() {
    if (!gEnabled.load(std::memory_order_acquire)) return;

    std::lock_guard resolveGuard(gRuntimeMutex);
    if (gMinecraftReady.load(std::memory_order_acquire)) return;

    // Guard against recursive dlopen calls made by the resolver/hooker.
    if (gResolving) return;
    gResolving = true;
    const bool installed = game_hook::install();
    gResolving = false;

    if (installed) {
        gMinecraftReady.store(true, std::memory_order_release);
        uninstallDlopenHook();
    }
}

bool Runtime::enable() {
    gEnabled.store(true, std::memory_order_release);

    if (minecraftAlreadyLoaded()) {
        return resolveAndInstall();
    }

    installDlopenHook();
    return true;
}

bool Runtime::disable() {
    gEnabled.store(false, std::memory_order_release);

    std::lock_guard lock(gRuntimeMutex);
    if (gMinecraftReady.exchange(false, std::memory_order_acq_rel)) {
        game_hook::uninstall();
    }
    uninstallDlopenHook();
    return true;
}

bool Runtime::unload() {
    return disable();
}

} // namespace accurate_block_placement
