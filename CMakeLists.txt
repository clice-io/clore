cmake_minimum_required(VERSION 3.30)

project(clore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

option(CLORE_ENABLE_LTO "Enable ThinLTO for all targets" OFF)
option(CLORE_OFFLINE_BUILD "Disable network downloads during configuration" OFF)
option(CLORE_ENABLE_TEST "Build unit tests" OFF)

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

if(CLORE_ENABLE_LTO)
    string(APPEND CMAKE_C_FLAGS " -flto=thin")
    string(APPEND CMAKE_CXX_FLAGS " -flto=thin")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -flto=thin")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -flto=thin")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " -flto=thin")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # clang-cl (MSVC frontend): manually link ASan runtime since clang-cl
        # doesn't handle -fsanitize=address linking automatically.
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} --print-resource-dir
            OUTPUT_VARIABLE CLANG_RESOURCE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(ASAN_LIB_PATH "${CLANG_RESOURCE_DIR}/lib/windows")
        link_directories(${ASAN_LIB_PATH})

        set(ASAN_LINK_FLAGS "")
        list(APPEND ASAN_LINK_FLAGS "clang_rt.asan_dynamic-x86_64.lib")
        list(APPEND ASAN_LINK_FLAGS "/wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib")

        foreach(flag ${ASAN_LINK_FLAGS})
            string(APPEND CMAKE_EXE_LINKER_FLAGS " ${flag}")
            string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${flag}")
            string(APPEND CMAKE_MODULE_LINKER_FLAGS " ${flag}")
        endforeach()
    else()
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -fsanitize=address")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -fsanitize=address")
    endif()

    if(WIN32)
        # Disable Identical COMDAT Folding in Debug to avoid ASan ODR false positives.
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,/OPT:NOICF")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,/OPT:NOICF")
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
