add_executable(LuaCEFSubprocess main.cpp)

target_compile_features(LuaCEFSubprocess PRIVATE cxx_std_20)

target_link_libraries(LuaCEFSubprocess
    cef-common
)

set_target_properties(LuaCEFSubprocess PROPERTIES
    OUTPUT_NAME "cef-subprocess"
    WIN32_EXECUTABLE TRUE 
    LINK_FLAGS "/ENTRY:WinMainCRTStartup"
)
