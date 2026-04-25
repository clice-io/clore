cmake_minimum_required(VERSION 4.3.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(clore LANGUAGES CXX)

include(FetchContent)

find_package(spdlog REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

set(KOTA_ENABLE_ASYNC ON)
set(KOTA_ENABLE_HTTP ON)
set(KOTA_ENABLE_OPTION ON)
set(KOTA_ENABLE_DECO ON)
set(KOTA_ENABLE_ZEST ON)
set(KOTA_ENABLE_CCACHE OFF)
set(KOTA_ENABLE_EXCEPTIONS ON)
set(KOTA_ENABLE_EXAMPLES OFF)
set(KOTA_ENABLE_RTTI ON)
set(KOTA_ENABLE_TEST OFF)
set(KOTA_BUILD_ALL_TESTS OFF)
set(KOTA_USE_CPM_FOR_TESTS OFF)
set(KOTA_SANITIZE_ADDRESS OFF)
set(KOTA_SANITIZE_MEMORY OFF)
set(KOTA_SANITIZE_THREAD OFF)
set(KOTA_SANITIZE_UNDEFINED OFF)
set(KOTA_CODEC_ENABLE_SIMDJSON ON)
set(KOTA_CODEC_ENABLE_TOML ON)
set(KOTA_CODEC_ENABLE_FLATBUFFERS OFF)

set(CURL_USE_PKGCONFIG OFF)
set(USE_LIBIDN2 OFF)
set(CURL_BROTLI OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_Brotli ON)
set(BROTLI_FOUND OFF)
set(USE_NGHTTP2 OFF)
set(CURL_USE_LIBSSH2 OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-ftrivial-auto-var-init=pattern)
    if(NOT WIN32)
        add_compile_options(-fsanitize=address,undefined)
        add_compile_options(-fno-sanitize-recover=undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_error LANGUAGES CXX)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endif()

FetchContent_Declare(
    kotatsu
    GIT_REPOSITORY https://github.com/fogsong233/eventide.git
    GIT_TAG        add-curl
)
FetchContent_MakeAvailable(kotatsu)

add_subdirectory(src)
add_subdirectory(tests)