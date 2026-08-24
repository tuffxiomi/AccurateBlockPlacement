# Accurate Block Placement

A standalone LeviLauncher native mod that improves edge/corner block placement by correcting the placement face passed into Bedrock's `GameMode::useItemOn` path when the hit point is very close to a block boundary.

## What was inspected

The supplied BedrockTools source uses:

- ARM64 byte-signature resolution through `pl::memory::resolveSignatures`.
- Inline detours through its hook layer.
- `GameModeStartBuildBlock` / `GameModeUseItemOn` related signatures.
- A one-byte `InteractionResultValue` return type for `UseItemOn`.
- Separate signatures for the normal and survival game-mode implementations.

This project does **not** include or link BedrockTools. It reimplements only the small amount of runtime glue it needs using the public PreLoader Android SDK.

## Target validation

The supplied `libminecraftpe.so` was inspected directly. It is an ARM64 stripped Android shared library and the two placement signatures used by this project each resolved to exactly one address in its `.text` section.

The resolved addresses in the supplied binary were:

- `GameModeUseItemOn`: `0xEF74F08`
- `SurvivalModeUseItemOn`: `0xEF77478`

These are diagnostic offsets for that exact binary, not hard-coded addresses. The mod uses signatures at runtime so ASLR and normal binary relocation do not require changing the source.

The same supplied binary also contains Minecraft 1.26.x format strings, including 1.26.20-era identifiers. The manifest therefore targets `1.26.*`.

## Build

Use Android NDK 28.2.13676358 or a compatible recent NDK, CMake 3.22+, and Ninja.

Example:

```bash
cmake -S . -B build-arm64 \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_HOME/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-28 \
  -DANDROID_STL=c++_shared

cmake --build build-arm64 --target levi_package
```

The native library is:

`build-arm64/libAccurateBlockPlacement.so`

## Compatibility note

Signature compatibility was verified against the exact `libminecraftpe.so` supplied with this project request. A different Bedrock binary may require updated signatures even when the Minecraft version family is still 1.26.x.

## No BedrockTools files

There are no `bedrocktools/` headers, BedrockTools source files, or BedrockTools libraries in this project.
