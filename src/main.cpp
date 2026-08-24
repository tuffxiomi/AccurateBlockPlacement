#include "Runtime.hpp"

#include <pl/Mod.hpp>

class AccurateBlockPlacementMod {
public:
    static AccurateBlockPlacementMod& instance() {
        static AccurateBlockPlacementMod mod;
        return mod;
    }

    bool load(pl::mod::ModContext&) {
        return accurate_block_placement::Runtime::instance().load();
    }

    bool enable(pl::mod::ModContext&) {
        return accurate_block_placement::Runtime::instance().enable();
    }

    bool disable(pl::mod::ModContext&) {
        return accurate_block_placement::Runtime::instance().disable();
    }

    bool unload(pl::mod::ModContext&) {
        return accurate_block_placement::Runtime::instance().unload();
    }
};

PL_REGISTER_MOD(AccurateBlockPlacementMod, AccurateBlockPlacementMod::instance())
