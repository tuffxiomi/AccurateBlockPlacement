#include "Runtime.hpp"

#include <pl/Mod.h>

namespace {

class AccurateBlockPlacementMod final : public pl::Mod {
public:
    AccurateBlockPlacementMod() {
        accurate_block_placement::Runtime::instance().load();
    }

    void onEnable() override {
        accurate_block_placement::Runtime::instance().enable();
    }

    void onDisable() override {
        accurate_block_placement::Runtime::instance().disable();
    }

    void onUnload() override {
        accurate_block_placement::Runtime::instance().unload();
    }
};

AccurateBlockPlacementMod gMod;

} // namespace

PL_REGISTER_MOD(AccurateBlockPlacementMod)
