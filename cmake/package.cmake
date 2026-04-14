include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/llvm.cmake)
setup_llvm("21.1.4+r1")

# install dependencies
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_SHALLOW TRUE
)
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)


FetchContent_Declare(
    eventide
    GIT_REPOSITORY "https://github.com/clice-io/eventide.git"
    GIT_TAG "main"
)
set(ETD_ENABLE_ASYNC ON CACHE BOOL "" FORCE)
set(ETD_ENABLE_DECO ON CACHE BOOL "" FORCE)
set(ETD_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ETD_ENABLE_OPTION ON CACHE BOOL "" FORCE)
set(ETD_ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(ETD_ENABLE_ZEST ON CACHE BOOL "" FORCE)
set(ETD_SERDE_ENABLE_FLATBUFFERS OFF CACHE BOOL "" FORCE)
set(ETD_SERDE_ENABLE_SIMDJSON ON CACHE BOOL "" FORCE)
set(ETD_SERDE_ENABLE_TOML ON CACHE BOOL "" FORCE)
set(ETD_SERDE_ENABLE_YYJSON ON CACHE BOOL "" FORCE)
set(ETD_ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
set(ETD_ENABLE_RTTI OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(eventide spdlog)

# Prefer libcurl from the active pixi/conda environment to avoid picking up
# incompatible system libraries from /usr/lib.
set(_clore_curl_from_pixi OFF)
if(DEFINED ENV{CONDA_PREFIX} AND NOT "$ENV{CONDA_PREFIX}" STREQUAL "")
    set(_clore_pixi_prefix "$ENV{CONDA_PREFIX}")
    set(_clore_curl_library_search_paths
        "${_clore_pixi_prefix}/lib"
        "${_clore_pixi_prefix}/Library/lib"
    )
    set(_clore_curl_include_search_paths
        "${_clore_pixi_prefix}/include"
        "${_clore_pixi_prefix}/Library/include"
    )

    find_library(_clore_pixi_curl_library
        NAMES curl libcurl libcurl_imp curllib
        PATHS ${_clore_curl_library_search_paths}
        NO_DEFAULT_PATH
    )
    find_path(_clore_pixi_curl_include_dir
        NAMES curl/curl.h
        PATHS ${_clore_curl_include_search_paths}
        NO_DEFAULT_PATH
    )

    if(_clore_pixi_curl_library AND _clore_pixi_curl_include_dir)
        set(CURL_LIBRARY "${_clore_pixi_curl_library}" CACHE FILEPATH
            "libcurl library pinned to pixi/conda environment" FORCE)
        set(CURL_LIBRARY_RELEASE "${_clore_pixi_curl_library}" CACHE FILEPATH
            "libcurl release library pinned to pixi/conda environment" FORCE)
        set(CURL_LIBRARY_DEBUG "${_clore_pixi_curl_library}" CACHE FILEPATH
            "libcurl debug library pinned to pixi/conda environment" FORCE)
        set(CURL_INCLUDE_DIR "${_clore_pixi_curl_include_dir}" CACHE PATH
            "libcurl include directory pinned to pixi/conda environment" FORCE)
        set(_clore_curl_from_pixi ON)
        message(STATUS "Using pixi libcurl: ${_clore_pixi_curl_library}")
    else()
        message(WARNING
            "CONDA_PREFIX is set to '${_clore_pixi_prefix}', but pixi libcurl was not fully resolved. "
            "Falling back to default CMake CURL lookup.")
    endif()

    unset(_clore_pixi_prefix)
    unset(_clore_curl_library_search_paths)
    unset(_clore_curl_include_search_paths)
    unset(_clore_pixi_curl_library)
    unset(_clore_pixi_curl_include_dir)
endif()

if(_clore_curl_from_pixi)
    # FindCURL appends pkg-config link libraries to CURL::libcurl when
    # PkgConfig is found. In mixed environments this may pull /usr/lib/libcurl
    # back in, so disable PkgConfig for this lookup when pixi curl is pinned.
    set(CMAKE_DISABLE_FIND_PACKAGE_PkgConfig TRUE)
endif()

find_package(CURL REQUIRED)

if(_clore_curl_from_pixi)
    unset(CMAKE_DISABLE_FIND_PACKAGE_PkgConfig)
endif()

if(NOT TARGET CURL::libcurl)
    message(FATAL_ERROR "find_package(CURL) succeeded but CURL::libcurl target is missing")
endif()
