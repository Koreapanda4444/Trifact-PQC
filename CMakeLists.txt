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

add_library(trifact STATIC src/core.c src/trifact.c)

target_compile_features(trifact PUBLIC c_std_17)
target_include_directories(
    trifact
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(trifact PROPERTIES C_EXTENSIONS OFF)
trifact_enable_warnings(trifact)

if(BUILD_TESTING)
    add_executable(trifact_tests tests/test_toolchain.c)
    target_compile_features(trifact_tests PRIVATE c_std_17)
    target_link_libraries(trifact_tests PRIVATE trifact)
    set_target_properties(trifact_tests PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(trifact_tests)

    add_test(NAME trifact.toolchain COMMAND trifact_tests)

    add_executable(trifact_core_types_tests tests/test_core_types.c)
    target_compile_features(trifact_core_types_tests PRIVATE c_std_17)
    target_link_libraries(trifact_core_types_tests PRIVATE trifact)
    set_target_properties(trifact_core_types_tests PROPERTIES C_EXTENSIONS OFF)
    trifact_enable_warnings(trifact_core_types_tests)

    add_test(NAME trifact.core-types COMMAND trifact_core_types_tests)
endif()
