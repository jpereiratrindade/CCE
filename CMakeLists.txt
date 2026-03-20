cmake_minimum_required(VERSION 3.20)
project(cce VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(cce_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(cce_ENABLE_SANITIZERS "Enable address and undefined sanitizers" OFF)

find_program(CLANG_FORMAT_EXE NAMES clang-format)
find_program(CLANG_TIDY_EXE NAMES clang-tidy)

function(configure_project_target target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    if(cce_WARNINGS_AS_ERRORS)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()
    if(cce_ENABLE_SANITIZERS)
        target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

add_library(cce_core
    src/App.cpp
    src/ConceptMemory.cpp
    src/SemanticAnalyzer.cpp
)
target_include_directories(cce_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
configure_project_target(cce_core)

add_executable(cce
    src/main.cpp
)
target_link_libraries(cce PRIVATE cce_core)
target_include_directories(cce PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
configure_project_target(cce)

add_executable(cce_tests
    tests/SmokeTests.cpp
)
target_link_libraries(cce_tests PRIVATE cce_core)
target_include_directories(cce_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
configure_project_target(cce_tests)

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE FORMAT_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${FORMAT_SOURCES}
        COMMENT "Checking code format"
        VERBATIM
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME cce_tests COMMAND cce_tests)
endif()
