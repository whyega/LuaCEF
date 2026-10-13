cmake_minimum_required(VERSION 3.15)
project(LuaCEF)

add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CEF_ROOT "${CMAKE_SOURCE_DIR}/third_party/cef")
set(CEF_RUNTIME_LIBRARY_FLAG   
    /MT
    CACHE STRING ""
    FORCE)
set(USE_SANDBOX OFF CACHE BOOL "")
add_subdirectory(
    ${CEF_ROOT}
    ${CMAKE_BINARY_DIR}/cef
)
include_directories(
    ${CEF_ROOT}/include
    ${CEF_ROOT}
)
link_libraries(
    libcef_dll_wrapper
    "$<$<CONFIG:Debug>:${CEF_ROOT}/Debug/libcef.lib>"
    "$<$<CONFIG:Release>:${CEF_ROOT}/Release/libcef.lib>"
)

set(LUACEF_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")
set(LUACEF_CEF_DIR "${LUACEF_DEPLOY_DIR}/luacef")

add_subdirectory(common)
add_subdirectory(host)
add_subdirectory(plugin)
add_subdirectory(subprocess)

foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)

    set_target_properties(LuaCEFPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>"
    )

    set_target_properties(LuaCEFHost PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>/luacef"
    )

    set_target_properties(LuaCEFSubprocess PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>/luacef"
    )
endforeach()

add_custom_target(DEPLOY ALL)

add_dependencies(
    DEPLOY
    LuaCEFPlugin
    LuaCEFHost
    LuaCEFSubprocess
)

add_custom_command(TARGET DEPLOY POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<$<CONFIG:Debug>:${CEF_ROOT}/Debug>$<$<CONFIG:Release>:${CEF_ROOT}/Release>"
            "$<TARGET_FILE_DIR:LuaCEFHost>"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_ROOT}/Resources"
            "$<TARGET_FILE_DIR:LuaCEFHost>"

    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:LuaCEFPlugin>/moonloader/lib"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/luacef/luacef.lua"
            "$<TARGET_FILE_DIR:LuaCEFPlugin>/moonloader/lib/luacef.lua"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/installer/Install-LuaCEF.ps1"
            "${CMAKE_SOURCE_DIR}/installer/Install-LuaCEF.cmd"
            "$<TARGET_FILE_DIR:LuaCEFPlugin>"

    COMMENT "Preparing the LuaCEF runtime, Lua module and installer for $<CONFIG>"
)
