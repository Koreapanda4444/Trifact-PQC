cmake_minimum_required(VERSION 3.24)

project(trifact LANGUAGES C)

include(CTest)

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
endfunction()

set(TRIFACT_CORE_SOURCES src/core.c src/incidence.c src/relation.c src/trifact.c src/witness.c)

function(trifact_configure_library target_name)
    target_compile_features(${target_name} PUBLIC c_std_17)
    target_include_directories(
        ${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    set_target_properties(${target_name} PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(${target_name})
endfunction()

add_library(trifact STATIC ${TRIFACT_CORE_SOURCES} src/memory.c)
trifact_configure_library(trifact)

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

    add_library(trifact_fault STATIC ${TRIFACT_CORE_SOURCES} tests/failing_allocator.c)
    trifact_configure_library(trifact_fault)
    target_include_directories(trifact_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    add_executable(trifact_allocation_tests tests/test_allocation.c)
    target_link_libraries(trifact_allocation_tests PRIVATE trifact_fault)
    set_target_properties(trifact_allocation_tests PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(trifact_allocation_tests)
    add_test(NAME trifact.allocation COMMAND trifact_allocation_tests)
endif()
