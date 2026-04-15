cmake_minimum_required(VERSION 3.30)

project(clore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CheckIPOSupported)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

option(CLORE_ENABLE_LTO "Enable ThinLTO for all targets" ON)
option(CLORE_OFFLINE_BUILD "Disable network downloads during configuration" OFF)
option(CLORE_ENABLE_TEST "Build unit tests" ON)

if(WIN32 AND (MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
                      (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR
                       CMAKE_CXX_COMPILER_LINKER_FRONTEND_VARIANT STREQUAL "MSVC"))))
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING
        "Use the release CRT to stay ABI-compatible with the bundled Windows LLVM artifacts."
        FORCE)
endif()

# Global flags that apply to all targets (including FetchContent dependencies).
if(NOT MSVC)
    add_compile_options(-ffunction-sections -fdata-sections)
endif()

if(APPLE)
    # https://conda-forge.org/docs/maintainer/knowledge_base/#newer-c-features-with-old-sdk
    add_compile_definitions(_LIBCPP_DISABLE_AVAILABILITY=1)
endif()

if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
            (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR
             CMAKE_CXX_COMPILER_LINKER_FRONTEND_VARIANT STREQUAL "MSVC")))
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,/OPT:REF")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,/OPT:REF")
elseif(APPLE)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,-dead_strip")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,-dead_strip")
else()
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -static-libstdc++ -static-libgcc -Wl,--gc-sections")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--gc-sections")
endif()

set(CLORE_ENABLE_IPO OFF)
set(CLORE_USE_LTO_ARTIFACT OFF)
if(CLORE_ENABLE_LTO)
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        check_ipo_supported(RESULT CLORE_IPO_SUPPORTED OUTPUT CLORE_IPO_ERROR LANGUAGES C CXX)
        if(CLORE_IPO_SUPPORTED)
            set(CLORE_ENABLE_IPO ON)
            set(CLORE_USE_LTO_ARTIFACT ON)
            message(STATUS "Interprocedural optimization enabled for ${CMAKE_BUILD_TYPE} builds.")
        else()
            message(WARNING "LTO requested but IPO is not supported by the active toolchain: ${CLORE_IPO_ERROR}")
        endif()
    else()
        message(STATUS "Interprocedural optimization disabled for non-RelWithDebInfo builds.")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(WIN32)
        message(STATUS "AddressSanitizer disabled on Windows Debug builds: the current MSVC-targeting clang toolchain and prebuilt LLVM static libraries are not ASan-safe in this repo.")
    else()
        add_compile_options(-fsanitize=address)
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -fsanitize=address")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -fsanitize=address")
    endif()
endif()

include("${PROJECT_SOURCE_DIR}/cmake/package.cmake")

# Project-specific options (not applied to third-party deps).
add_library(clore_options INTERFACE)

if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
         CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    target_compile_options(clore_options INTERFACE /GR- /EHs-c- /Zc:preprocessor)
else()
    target_compile_options(clore_options INTERFACE
        -fno-rtti
        -fno-exceptions
        -Wno-deprecated-declarations
        $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-undefined-inline>
    )
endif()

if(WIN32)
    target_link_libraries(clore_options INTERFACE version ntdll)
endif()

if(CLORE_ENABLE_TEST)
    target_compile_definitions(clore_options INTERFACE CLORE_ENABLE_TEST=1)
endif()

add_subdirectory(src)
