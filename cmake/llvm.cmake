include_guard()

function(setup_llvm LLVM_VERSION)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    set(LLVM_SETUP_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/.llvm/setup-llvm.json")
    set(LLVM_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/scripts/setup-llvm.py")
    set(LLVM_SETUP_ARGS
        "--version" "${LLVM_VERSION}"
        "--build-type" "${CMAKE_BUILD_TYPE}"
        "--binary-dir" "${CMAKE_CURRENT_BINARY_DIR}"
        "--manifest" "${PROJECT_SOURCE_DIR}/config/llvm-manifest.json"
        "--output" "${LLVM_SETUP_OUTPUT}"
    )

    if(CLORE_ENABLE_LTO)
        list(APPEND LLVM_SETUP_ARGS "--enable-lto")
    endif()

    if(DEFINED LLVM_INSTALL_PATH AND NOT LLVM_INSTALL_PATH STREQUAL "")
        list(APPEND LLVM_SETUP_ARGS "--install-path" "${LLVM_INSTALL_PATH}")
    endif()

    if(DEFINED CLORE_OFFLINE_BUILD AND CLORE_OFFLINE_BUILD)
        list(APPEND LLVM_SETUP_ARGS "--offline")
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${LLVM_SETUP_SCRIPT}" ${LLVM_SETUP_ARGS}
        RESULT_VARIABLE LLVM_SETUP_RESULT
        OUTPUT_VARIABLE LLVM_SETUP_STDOUT
        ERROR_VARIABLE LLVM_SETUP_STDERR
        ECHO_OUTPUT_VARIABLE
        ECHO_ERROR_VARIABLE
        COMMAND_ERROR_IS_FATAL ANY
    )

    file(READ "${LLVM_SETUP_OUTPUT}" LLVM_SETUP_JSON)
    string(JSON LLVM_INSTALL_PATH GET "${LLVM_SETUP_JSON}" install_path)
    string(JSON LLVM_CMAKE_DIR GET "${LLVM_SETUP_JSON}" cmake_dir)
    set(LLVM_INSTALL_PATH "${LLVM_INSTALL_PATH}" CACHE PATH "Path to LLVM installation" FORCE)
    set(LLVM_CMAKE_DIR "${LLVM_CMAKE_DIR}" CACHE PATH "Path to LLVM CMake files" FORCE)

    get_filename_component(LLVM_INSTALL_PATH "${LLVM_INSTALL_PATH}" ABSOLUTE)

    if(NOT EXISTS "${LLVM_INSTALL_PATH}")
        message(FATAL_ERROR "Error: The specified LLVM_INSTALL_PATH does not exist: ${LLVM_INSTALL_PATH}")
    endif()

    add_library(llvm-libs INTERFACE IMPORTED)
    target_include_directories(llvm-libs INTERFACE "${LLVM_INSTALL_PATH}/include")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
        target_link_directories(llvm-libs INTERFACE "${LLVM_INSTALL_PATH}/lib")
        target_link_libraries(llvm-libs INTERFACE
            LLVMSupport
            LLVMFrontendOpenMP
            LLVMOption
            LLVMTargetParser
            clangAST
            clangBasic
            clangDriver
            clangFrontend
            clangLex
            clangSema
            clangSerialization
            clangTooling
            clangToolingCore
            clangToolingInclusions
            clangToolingInclusionsStdlib
        )
    else()
        file(GLOB LLVM_LIBRARIES CONFIGURE_DEPENDS "${LLVM_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LLVM[a-zA-Z]*${CMAKE_STATIC_LIBRARY_SUFFIX}")
        file(GLOB CLANG_LIBRARIES CONFIGURE_DEPENDS "${LLVM_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}clang[a-zA-Z]*${CMAKE_STATIC_LIBRARY_SUFFIX}")
        file(GLOB OTHER_REQUIRED_LIBS CONFIGURE_DEPENDS
            "${LLVM_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}"
            "${LLVM_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}"
        )
        target_link_libraries(llvm-libs INTERFACE ${LLVM_LIBRARIES} ${CLANG_LIBRARIES} ${OTHER_REQUIRED_LIBS})
        target_compile_definitions(llvm-libs INTERFACE CLANG_BUILD_STATIC=1)
    endif()
endfunction()
