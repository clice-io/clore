file(GLOB_RECURSE CLORE_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/config/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/extract/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generate/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/support/*.cpp"
)

add_library(clore-core STATIC ${CLORE_CORE_SOURCES})
add_library(clore::core ALIAS clore-core)

target_include_directories(clore-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(clore-core PUBLIC
    clore_options
    llvm-libs
    spdlog::spdlog
    eventide::serde::toml
    eventide::serde::json
    simdjson::simdjson
)

add_executable(clore "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(clore PRIVATE clore::core eventide::deco)
install(TARGETS clore RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_custom_target(copy_clang_resource ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LLVM_INSTALL_PATH}/lib/clang"
        "${PROJECT_BINARY_DIR}/lib/clang"
    COMMENT "Copying clang resource directory"
)
install(
    DIRECTORY "${LLVM_INSTALL_PATH}/lib/clang"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

if(CLORE_ENABLE_TEST)
    file(GLOB_RECURSE CLORE_TEST_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/tests/unit/*/*.cpp"
    )

    add_executable(unit_tests
        "${PROJECT_SOURCE_DIR}/tests/unit/unit_tests.cpp"
        ${CLORE_TEST_SOURCES}
    )
    target_include_directories(unit_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/tests/unit"
    )
    target_link_libraries(unit_tests PRIVATE clore::core eventide::zest eventide::deco)
endif()

if(CLORE_ENABLE_IPO)
    set_property(TARGET clore-core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET clore PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    if(CLORE_ENABLE_TEST)
        set_property(TARGET unit_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()
