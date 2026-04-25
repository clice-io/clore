file(GLOB CLORE_ROOT_MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm"
)

file(GLOB_RECURSE CLORE_MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cppm"
)

if(WIN32)
    if(NOT DEFINED ENV{VCToolsInstallDir} OR "$ENV{VCToolsInstallDir}" STREQUAL "")
        message(FATAL_ERROR
            "VCToolsInstallDir is required for Clang builds on Windows to locate std.ixx"
        )
    endif()

    set(CLORE_WINDOWS_STD_MODULE_BASE_DIR "$ENV{VCToolsInstallDir}/modules")
    set(CLORE_WINDOWS_STD_MODULE_SOURCE
        "${CLORE_WINDOWS_STD_MODULE_BASE_DIR}/std.ixx"
    )

    if(NOT EXISTS "${CLORE_WINDOWS_STD_MODULE_SOURCE}")
        message(FATAL_ERROR
            "std.ixx not found at expected path: ${CLORE_WINDOWS_STD_MODULE_SOURCE}"
        )
    endif()
endif()

add_library(clore-core STATIC)
set_target_properties(clore-core PROPERTIES CXX_SCAN_FOR_MODULES ON)
add_library(clore::core ALIAS clore-core)
if(WIN32)
    target_sources(clore-core
        PUBLIC
        FILE_SET public_modules TYPE CXX_MODULES
        BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CLORE_WINDOWS_STD_MODULE_BASE_DIR}"
        FILES
        ${CLORE_ROOT_MODULE_SOURCES}
        ${CLORE_MODULE_SOURCES}
        "${CLORE_WINDOWS_STD_MODULE_SOURCE}"
    )
else()
    target_sources(clore-core
        PUBLIC
        FILE_SET public_modules TYPE CXX_MODULES
        FILES
        ${CLORE_ROOT_MODULE_SOURCES}
        ${CLORE_MODULE_SOURCES}
    )
endif()
target_compile_features(clore-core PUBLIC cxx_std_23)
target_include_directories(clore-core
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
    "${LLVM_INCLUDE_DIRS}"
)
target_link_libraries(clore-core PUBLIC
    clangTooling
    clangFrontend
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAST
    clangLex
    clangBasic
    clangIndex
    LLVMSupport
    spdlog::spdlog
    kota::async
    kota::http
    kota::codec::toml
    kota::codec::json
)

add_executable(clore "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(clore PRIVATE clore::core kota::deco)
add_custom_command(TARGET clore POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    $<TARGET_RUNTIME_DLLS:clore>
    $<TARGET_FILE_DIR:clore>
    COMMAND_EXPAND_LISTS
)
