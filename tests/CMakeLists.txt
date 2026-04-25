file(GLOB_RECURSE CLORE_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
)

add_executable(unit_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp"
    ${CLORE_TEST_SOURCES}
)
target_include_directories(unit_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(unit_tests PRIVATE clore::core kota::zest kota::deco)
add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    $<TARGET_RUNTIME_DLLS:unit_tests>
    $<TARGET_FILE_DIR:unit_tests>
    COMMAND_EXPAND_LISTS
)