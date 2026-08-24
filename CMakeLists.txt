cmake_minimum_required(VERSION 3.22)

project(AccurateBlockPlacement LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

# ---------------------------------------------------------
# fmt
# ---------------------------------------------------------
#
# PreLoader Android 0.2.3 includes fmt/format.h from its
# public Logger.hpp header.
#
# Fetch fmt explicitly before configuring PreLoader and make
# its include directory available to the nested PreLoader
# build as well.
#

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)

FetchContent_MakeAvailable(fmt)

include_directories(
    SYSTEM
    "${fmt_SOURCE_DIR}/include"
)

# ---------------------------------------------------------
# PreLoader Android
# ---------------------------------------------------------

FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 0.2.3
    SOURCE_SUBDIR cmake/preloader-sdk
)

FetchContent_MakeAvailable(preloader_android)

FetchContent_GetProperties(preloader_android)

if(NOT preloader_android_SOURCE_DIR)
    message(FATAL_ERROR
        "PreLoader Android source directory was not populated."
    )
endif()

set(
    LEVI_PRELOADER_INCLUDE_DIR
    "${preloader_android_SOURCE_DIR}/include"
)

if(NOT EXISTS "${LEVI_PRELOADER_INCLUDE_DIR}/pl")
    message(FATAL_ERROR
        "PreLoader headers were not found at: "
        "${LEVI_PRELOADER_INCLUDE_DIR}"
    )
endif()

# ---------------------------------------------------------
# Accurate Block Placement
# ---------------------------------------------------------

add_library(
    accurate_block_placement
    SHARED

    src/main.cpp
    src/Runtime.cpp
    src/GameHook.cpp
    src/Placement.cpp
)

# ---------------------------------------------------------
# Include directories
# ---------------------------------------------------------

target_include_directories(
    accurate_block_placement
    PRIVATE

    "${CMAKE_CURRENT_SOURCE_DIR}/src"

    "${LEVI_PRELOADER_INCLUDE_DIR}"

    "${fmt_SOURCE_DIR}/include"
)

# ---------------------------------------------------------
# Libraries
# ---------------------------------------------------------

target_link_libraries(
    accurate_block_placement
    PRIVATE

    preloader
    fmt::fmt
    dl
)

# ---------------------------------------------------------
# Compiler options
# ---------------------------------------------------------

target_compile_options(
    accurate_block_placement
    PRIVATE

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

# IMPORTANT:
#
# Do NOT add:
#
#     -fno-exceptions
#
# PreLoader uses exception handling in its headers.
#

# ---------------------------------------------------------
# Linker options
# ---------------------------------------------------------

target_link_options(
    accurate_block_placement
    PRIVATE

    -Wl,--gc-sections
    -Wl,--icf=all
    -Wl,-z,max-page-size=16384
)

# ---------------------------------------------------------
# Output
# ---------------------------------------------------------

set_target_properties(
    accurate_block_placement
    PROPERTIES

    OUTPUT_NAME "AccurateBlockPlacement"
)

# ---------------------------------------------------------
# Levi package
# ---------------------------------------------------------

add_custom_target(
    levi_package
    ALL

    COMMAND
        ${CMAKE_COMMAND}
        -E
        make_directory
        "${CMAKE_BINARY_DIR}/package"

    COMMAND
        ${CMAKE_COMMAND}
        -E
        copy
        "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
        "${CMAKE_BINARY_DIR}/package/manifest.json"

    COMMAND
        ${CMAKE_COMMAND}
        -E
        copy
        "$<TARGET_FILE:accurate_block_placement>"
        "${CMAKE_BINARY_DIR}/package/libAccurateBlockPlacement.so"

    DEPENDS
        accurate_block_placement
)
