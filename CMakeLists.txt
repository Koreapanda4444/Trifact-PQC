cmake_minimum_required(VERSION 3.24)

project(trifact LANGUAGES C)

include(CTest)
include(CheckCCompilerFlag)
include(CMakePushCheckState)

option(TRIFACT_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(TRIFACT_ENABLE_GCC_ANALYZER "Enable GCC static analysis" OFF)

if(TRIFACT_ENABLE_SANITIZERS)
    if(MSVC OR NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        message(FATAL_ERROR "TRIFACT sanitizers require a supported GCC or Clang toolchain")
    endif()
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address,undefined)
    check_c_compiler_flag(-fsanitize=address,undefined TRIFACT_HAS_SANITIZERS)
    cmake_pop_check_state()
    if(NOT TRIFACT_HAS_SANITIZERS)
        message(FATAL_ERROR "The selected toolchain cannot compile and link both sanitizers")
    endif()

    add_library(trifact_sanitizers INTERFACE)
    target_compile_options(trifact_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(trifact_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

if(TRIFACT_ENABLE_GCC_ANALYZER)
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "TRIFACT_ENABLE_GCC_ANALYZER requires GCC")
    endif()
    check_c_compiler_flag(-fanalyzer TRIFACT_HAS_GCC_ANALYZER)
    if(NOT TRIFACT_HAS_GCC_ANALYZER)
        message(FATAL_ERROR "The selected GCC toolchain does not support -fanalyzer")
    endif()
endif()

function(trifact_enable_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wconversion
                -Werror
                -Wextra
                -Wpedantic
                -Wshadow
                -Wsign-conversion
                -Wstrict-prototypes
        )
    endif()
    if(TRIFACT_ENABLE_GCC_ANALYZER)
        target_compile_options(${target_name} PRIVATE -fanalyzer)
    endif()
endfunction()

set(TRIFACT_CORE_SOURCES src/core.c src/incidence.c src/keccak.c src/relation.c src/secure.c src/shake.c src/trifact.c src/witness.c)

function(trifact_configure_library target_name)
    target_compile_features(${target_name} PUBLIC c_std_17)
    target_include_directories(
        ${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    set_target_properties(${target_name} PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(${target_name})
    if(TRIFACT_ENABLE_SANITIZERS)
        target_link_libraries(${target_name} PUBLIC trifact_sanitizers)
    endif()
endfunction()

add_library(trifact STATIC ${TRIFACT_CORE_SOURCES} src/memory.c)
trifact_configure_library(trifact)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
    add_custom_target(
        trifact-clang-analysis
        COMMAND
            ${CMAKE_C_COMPILER} --analyze -std=c17
            -I${CMAKE_CURRENT_SOURCE_DIR}/include
            --analyzer-output text -Xanalyzer -analyzer-werror
            ${TRIFACT_CORE_SOURCES} src/memory.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
endif()

if(BUILD_TESTING)
    function(trifact_add_test target_name test_name)
        add_executable(${target_name} ${ARGN})
        target_compile_features(${target_name} PRIVATE c_std_17)
        target_link_libraries(${target_name} PRIVATE trifact)
        set_target_properties(${target_name} PROPERTIES C_EXTENSIONS OFF)
        trifact_enable_warnings(${target_name})
        add_test(NAME ${test_name} COMMAND ${target_name})
    endfunction()

    trifact_add_test(trifact_tests trifact.toolchain tests/test_toolchain.c)
    trifact_add_test(trifact_core_types_tests trifact.core-types tests/test_core_types.c)
    trifact_add_test(trifact_relation_tests trifact.relation tests/test_relation.c)
    trifact_add_test(trifact_relation_oracle_tests trifact.relation-oracle tests/test_relation_oracle.c)
    trifact_add_test(trifact_incidence_tests trifact.incidence tests/test_incidence.c)
    trifact_add_test(trifact_witness_tests trifact.witness tests/test_witness.c)
    trifact_add_test(trifact_invariant_tests trifact.invariants tests/test_invariants.c)
    trifact_add_test(trifact_shake_tests trifact.shake tests/test_shake.c)
    trifact_add_test(trifact_shake_vector_tests trifact.shake-vectors tests/test_shake_vectors.c)

    add_library(trifact_fault STATIC ${TRIFACT_CORE_SOURCES} tests/failing_allocator.c)
    trifact_configure_library(trifact_fault)
    target_include_directories(trifact_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    add_executable(trifact_allocation_tests tests/test_allocation.c)
    target_link_libraries(trifact_allocation_tests PRIVATE trifact_fault)
    set_target_properties(trifact_allocation_tests PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(trifact_allocation_tests)
    add_test(NAME trifact.allocation COMMAND trifact_allocation_tests)
endif()
