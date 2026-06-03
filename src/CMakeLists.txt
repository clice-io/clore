file(GLOB CLORE_ROOT_MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm"
)

file(GLOB_RECURSE CLORE_MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cppm"
)

add_library(clore-core STATIC)
set_target_properties(clore-core PROPERTIES CXX_SCAN_FOR_MODULES ON)
add_library(clore::core ALIAS clore-core)
target_sources(clore-core
    PUBLIC
    FILE_SET public_modules TYPE CXX_MODULES
    FILES
    ${CLORE_ROOT_MODULE_SOURCES}
    ${CLORE_MODULE_SOURCES}
)
target_compile_features(clore-core PUBLIC cxx_std_23)
if(NOT TARGET simdjson)
    message(FATAL_ERROR "simdjson target is required before configuring clore-core")
endif()
get_target_property(json_headers simdjson INTERFACE_INCLUDE_DIRECTORIES)
if(NOT json_headers)
    message(FATAL_ERROR "simdjson target does not expose include directories")
endif()
target_include_directories(clore-core
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${json_headers}
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
if(WIN32)
    add_custom_command(TARGET clore POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        $<TARGET_RUNTIME_DLLS:clore>
        $<TARGET_FILE_DIR:clore>
        COMMAND_EXPAND_LISTS
    )
endif()

install(TARGETS clore RUNTIME)
