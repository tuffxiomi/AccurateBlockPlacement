# BedrockTools Inspection Report

This report records the parts of the supplied BedrockTools tree that were used as a reference. No BedrockTools source or headers are bundled into the new mod.

## Supplied Minecraft binary

- Architecture: AArch64 / ARM64
- Android shared library: `libminecraftpe.so`
- Stripped ELF
- `.text` virtual base in the supplied file: `0x6121100`
- `.text` size: `0x0BA6F4DC`
- Build ID: `b480c79a54f33d6e4f0d63a131673e3daf749911`

All 121 BedrockTools signatures were tested against the supplied `.text` section. Every signature produced exactly one match.

## Placement-related signatures

| BedrockTools ID | Purpose | Match in supplied binary |
|---|---|---:|
| `GameModeUseItemOn` | Normal game-mode block interaction / placement path | `0xEF74F08` |
| `SurvivalModeUseItemOn` | Survival-mode block interaction / placement path | `0xEF77478` |
| `GameModeStartBuildBlock` | Game-mode block-destroy initiation | `0xEF73F28` |
| `SurvivalModeStartBuildBlock` | Survival block-destroy initiation | `0xEF773D0` |
| `LevelGetHitResult` | Level hit-test accessor | `0xF22D448` |
| `BlockSourceGetBlock` | Block lookup | `0xF2541EC` |
| `BlockSourceIsSolidBlockingBlock` | Solid-block query | `0xF255EC0` |
| `ClientInstanceGetLocalPlayer` | Local-player accessor | `0x9443404` |
| `ClientInstanceUpdate` | Client-instance update hook point | `0x943ECF4` |

The new mod only needs the two `UseItemOn` signatures. It does not hard-code these offsets; it resolves the signatures at runtime.

## Key offsets reviewed

The supplied BedrockTools SDK exposes these relevant structures:

- `HitResult::mType = 0x18`
- `HitResult::mPos = 0x2C`
- `HitResult::mStartPos = 0x00`
- `Actor::mStateVectorComponent = 0x208`
- `Actor::mActorRotationComponent = 0x218`
- `Actor::mDimension = 0x1C0`
- `Actor::mLevel = 0x1D0`
- `Dimension::mBlockSource = 0xD0`
- `ClientInstance::mLevelRenderer = 0x190`
- `VTable::ClientInstance_getRegion = 31`
- `VTable::BlockSource_getDimensionId = 18`

The accurate-placement implementation does not depend on these object offsets. It only uses the `UseItemOn` parameter layout and a local `BlockPos`/`Vec3` interpretation already reflected by BedrockTools' hook signature.

## How BedrockTools is structured

The supplied source has:

- `src/core/memory/Signatures.cpp`: signature definitions and runtime resolution.
- `src/core/GameHooks.cpp`: detours for game-mode, client, UI and rendering functions.
- `src/core/Runtime.cpp`: waits for `libminecraftpe.so`, resolves signatures, installs hooks, and manages lifecycle.
- `include/bedrocktools/sdk/...`: thin field/vtable/function wrappers.
- `src/modules/...`: feature modules grouped as visual, HUD, player and misc.
- `src/launcher/...`: launcher/mod-menu integration.
- `src/config/...`: persistent module configuration.

## Modules inspected

### Visual
breadcrumbs, chunkborder, connectedglass, fogcolor, fpsunlocker, fullbright, glintcolor, hitbox, lightoverlay, motionblur, nofog, shulkerpreview, swingmodifier, thirdpersonnametag, tnttimer, viewmodel, zoom

### HUD
breakindicator, combocounter, compass, debugmenu, keystrokes, pingcounter, playercoords, reachcounter, speeddisplay, tablist

### Player
autogg, autoreq, autosprint, nick, skinstealer, timechanger, weatherchanger

### Misc
chattimestamps, cpslimiter, forceglobalrp, nodisconnect, notouchborder

## New mod architecture

`AccurateBlockPlacement` is standalone:

1. Uses public PreLoader Android memory APIs.
2. Waits for `libminecraftpe.so` through a small `dlopen` detour when necessary.
3. Resolves only the two placement signatures.
4. Hooks both normal and survival `UseItemOn`.
5. When the hit point is close to a block boundary, derives the nearest face and passes the corrected face to the original function.
6. Leaves ordinary center-of-face interactions unchanged.
7. Does not ship any BedrockTools headers, source files, libraries, or module registry.

## Important compatibility limitation

Signature matching is exact for the supplied Minecraft binary. A different Bedrock build can change function bytes even inside the same major/minor version family. The runtime signature approach avoids fixed addresses, but the signature patterns themselves may still need maintenance for future builds.
