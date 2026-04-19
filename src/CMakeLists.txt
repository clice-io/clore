file(GLOB_RECURSE CLORE_MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cppm"
)

add_library(clore-core STATIC)
add_library(clore::core ALIAS clore-core)
target_sources(clore-core
    PUBLIC FILE_SET CXX_MODULES FILES ${CLORE_MODULE_SOURCES}
)
target_compile_features(clore-core PUBLIC cxx_std_23)
target_include_directories(clore-core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
        "${CLORE_LLVM_INCLUDE_DIR}"
)
target_link_libraries(clore-core PUBLIC
    "${CLORE_CLANG_CPP_LIBRARY}"
    "${CLORE_LLVM_SHARED_LIBRARY}"
    CURL::libcurl
    spdlog::spdlog
    kota::async
    kota::codec::toml
    kota::codec::json
    simdjson::simdjson
)

add_executable(clore "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(clore PRIVATE clore::core kota::deco)
