# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2018-2019 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

cmake_minimum_required(VERSION 3.5)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)  # Allow LTO.
endif()

project(ethash)
set(PROJECT_VERSION 0.5.1)

option(ETHASH_INSTALL_CMAKE_CONFIG "Install CMake configuration scripts for find_package(CONFIG)" ON)

option(ETHASH_FUZZING "Build with fuzzer instrumentation" OFF)
if(ETHASH_FUZZING)
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=fuzzer-no-link ${CMAKE_EXE_LINKER_FLAGS}")
    add_compile_options(-fno-omit-frame-pointer -fsanitize=fuzzer,undefined,integer -fno-sanitize-recover=all)
endif()

set(include_dir ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(lib)

option(ETHASH_BUILD_TESTS "Build unit tests" ON)
if(ETHASH_BUILD_TESTS)
    add_subdirectory(test)
endif()
