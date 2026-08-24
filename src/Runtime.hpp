#pragma once

#include <filesystem>

namespace accurate_block_placement {

class Runtime {
public:
    static Runtime& instance();

    bool load();
    bool enable();
    bool disable();
    bool unload();

    void onMinecraftLoaded();

private:
    Runtime() = default;

    bool resolveAndInstall();
    void installDlopenHook();
    void uninstallDlopenHook();

    std::filesystem::path mModDirectory;
};

} // namespace accurate_block_placement
