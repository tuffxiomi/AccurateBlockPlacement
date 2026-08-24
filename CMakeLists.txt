cmake_minimum_required(VERSION 3.22)

project(AccurateBlockPlacement LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# PreLoader Android
FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 0.2.3
)

FetchContent_MakeAvailable(preloader_android)

# Accurate Block Placement
add_library(accurate_block_placement SHARED
    src/main.cpp
    src/Runtime.cpp
    src/GameHook.cpp
    src/Placement.cpp
)

target_include_directories(accurate_block_placement PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(accurate_block_placement
    PRIVATE
        preloader
        dl
)

target_compile_options(accurate_block_placement PRIVATE
    -fPIC
    -Oz
    -ffunction-sections
    -fdata-sections
    -fno-rtti
    -fvisibility=hidden
    -Wall
    -Wextra
    -Werror
)

target_link_options(accurate_block_placement PRIVATE
    -Wl,--gc-sections
    -Wl,--icf=all
    -Wl,-z,max-page-size=16384
)

set_target_properties(accurate_block_placement PROPERTIES
    OUTPUT_NAME "AccurateBlockPlacement"
)

# Create the final Levi package
add_custom_target(levi_package ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_BINARY_DIR}/package

    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
            ${CMAKE_BINARY_DIR}/package/manifest.json

    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:accurate_block_placement>
            ${CMAKE_BINARY_DIR}/package/libAccurateBlockPlacement.so

    DEPENDS accurate_block_placement
)
