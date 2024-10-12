cmake_minimum_required(VERSION 3.20)

project(tas LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Static linking.
set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# CPM.
set(CPM_DOWNLOAD_VERSION 0.38.7)
set(CPM_HASH_SUM "83e5eb71b2bbb8b1f2ad38f1950287a057624e385c238f6087f94cdfc44af9c5")

if (CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif (DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else ()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif ()

get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

if (NOT EXISTS ${CPM_DL_LOCATION})
    message(STATUS "Downloading CPM.cmake to: ${CPM_DOWNLOAD_LOCATION}")

    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# External libraries.
CPMAddPackage("gh:g-truc/glm#adf31f555e73e2bd6fda373bb5d8740f9c6c17c0")
CPMAddPackage("gh:freetype/freetype#VER-2-13-2")
CPMAddPackage("gh:ocornut/imgui@1.90.3")
CPMAddPackage("gh:nemtrif/utfcpp@4.0.5")
CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:taocpp/PEGTL#3.2.7")
CPMAddPackage("gh:cursey/safetyhook#91fbc796862d80bc99294393e707db4c925dfeb1")
CPMAddPackage(
    NAME zydis
    GITHUB_REPOSITORY zyantific/zydis
    VERSION 4.0.0
    OPTIONS
    "ZYDIS_BUILD_EXAMPLES OFF"
    "ZYDIS_BUILD_TOOLS OFF"
    "ZYDIS_BUILD_DOXYGEN OFF")

# TAS DLL.
add_library(tas SHARED
    # Main
    tas/main.cpp
    tas/cheat.cpp

    # Core
    tas/core/debug/debug.cpp
    tas/core/input/input.cpp
    tas/core/interfaces/interfaces.cpp
    tas/core/menu/menu.cpp
    tas/core/netvars/netvars.cpp
    tas/core/offsets/offsets.cpp
    tas/core/player_manager/player_manager.cpp
    tas/core/prediction/prediction.cpp

    # Features
    tas/features/movement/movement.cpp
    tas/features/visual/esp.cpp
    tas/features/tas.cpp
    tas/features/compression_handler.cpp

    # Hooks
    tas/hooks/hooks.cpp

    # Library
    tas/library/math.cpp
    tas/library/render.cpp
    tas/library/utils.cpp
    tas/library/safe_imgui_draw_cmds.cpp

    # SDK
    tas/valve/entities/base_entity.cpp
    tas/valve/entities/player.cpp
    tas/valve/bsp.cpp
    tas/valve/tracing/trace_filter.cpp
)

if (imgui_ADDED)
    target_sources(tas PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
        # ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        tas/imgui_impl_win32.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
    )

    target_include_directories(tas PUBLIC
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends>)

    target_compile_definitions(tas PUBLIC
        "IMGUI_USER_CONFIG=\"${CMAKE_SOURCE_DIR}/tas/imconfig.hpp\""
        IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    )
endif ()

target_include_directories(tas PUBLIC tas)
target_compile_features(tas PUBLIC cxx_std_23)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(tas PRIVATE
        NDEBUG
        NOMINMAX
        _HAS_EXCEPTIONS=0
        _ITERATOR_DEBUG_LEVEL=0
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        GLM_FORCE_SWIZZLE
        GLM_FORCE_ALIGNED_GENTYPES
        GLM_ENABLE_EXPERIMENTAL
        JSON_NOEXCEPTION
    )

    target_compile_options(tas PRIVATE
        /O2
        /fp:precise
        /Ob2
        /GR-
        /GS-
        /EHsc
        /guard:cf-
        /clang:-mfpmath=sse
        /clang:-march=haswell # Haswell minimum.
        /clang:-mtune=generic
        /clang:-fno-ident
        /clang:-fno-unwind-tables
        /clang:-fno-asynchronous-unwind-tables
        /clang:-g0
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(tas PRIVATE
        NDEBUG
        NOMINMAX
        _HAS_EXCEPTIONS=0
        _ITERATOR_DEBUG_LEVEL=0
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        GLM_FORCE_SWIZZLE
        GLM_FORCE_ALIGNED_GENTYPES
        GLM_ENABLE_EXPERIMENTAL
        JSON_NOEXCEPTION
    )

    target_compile_options(tas PRIVATE
        /O2
        /fp:precise
        /Ob2
        /GR-
        /GS-
        /EHsc
        /guard:cf-
        /clang:-mfpmath=sse
        /clang:-march=haswell # Haswell minimum.
        /clang:-mtune=generic
        /clang:-fno-ident
        /clang:-fno-unwind-tables
        /clang:-fno-asynchronous-unwind-tables
        /clang:-g0
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(tas PRIVATE
        NOMINMAX
        GLM_FORCE_SWIZZLE
        GLM_FORCE_ALIGNED_GENTYPES
        GLM_ENABLE_EXPERIMENTAL
    )

    target_compile_options(tas PRIVATE
        /Od
        /fp:precise
        /Ob2
        /GR-
        /GS-
        /EHsc
        /guard:cf-
        /clang:-mfpmath=sse
        /clang:-march=haswell # Haswell minimum.
        /clang:-mtune=generic
        /clang:-fno-ident
        /clang:-fno-unwind-tables
        /clang:-fno-asynchronous-unwind-tables
        /clang:-g
    )
endif ()

target_link_libraries(tas PRIVATE glm::glm nlohmann_json::nlohmann_json taocpp::pegtl safetyhook::safetyhook utf8cpp freetype d3d9)
