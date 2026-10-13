cmake_minimum_required(VERSION 3.15)
project(LuaCEFRuntimePathTests LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Runtime path tests require Windows")
endif()

enable_testing()
add_executable(runtime-path-tests RuntimePathsTests.cpp)
target_compile_features(runtime-path-tests PRIVATE cxx_std_20)
target_compile_definitions(runtime-path-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(runtime-path-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../common")
add_test(NAME runtime-paths COMMAND runtime-path-tests)
