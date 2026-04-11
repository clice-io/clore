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
set(ETD_ENABLE_ASYNC OFF CACHE BOOL "" FORCE)
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

find_package(CURL REQUIRED)

if(NOT TARGET CURL::libcurl)
    message(FATAL_ERROR "find_package(CURL) succeeded but CURL::libcurl target is missing")
endif()
