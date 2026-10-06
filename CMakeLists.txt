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

add_library(trifact STATIC src/core.c src/relation.c src/trifact.c)

target_compile_features(trifact PUBLIC c_std_17)
target_include_directories(
    trifact
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(trifact PROPERTIES C_EXTENSIONS OFF)
trifact_enable_warnings(trifact)

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
endif()
