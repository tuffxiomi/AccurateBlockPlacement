#pragma once

#include <atomic>
#include <thread>

namespace accurate_block_placement {

class Runtime {
public:
    static Runtime& instance();

    bool load();
    bool enable();
    bool disable();
    bool unload();

private:
    Runtime() = default;
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    void startWatcher();
    void stopWatcher();
    void watcherLoop();

    bool minecraftLoaded() const;
    bool installWhenReady();

    std::atomic_bool mEnabled{false};
    std::atomic_bool mInstalled{false};
    std::atomic_bool mStopWatcher{false};

    std::thread mWatcher;
};

} // namespace accurate_block_placement
