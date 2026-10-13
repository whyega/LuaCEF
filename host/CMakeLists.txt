add_executable(LuaCEFHost 
    BrowserView.cpp
    Host.cpp
    IPCBrowserHandler.cpp
    main.cpp
)

target_link_libraries(LuaCEFHost
    cef-ipc
    cef-common
)

target_compile_features(LuaCEFHost PRIVATE cxx_std_20)

set_target_properties(LuaCEFHost PROPERTIES
    OUTPUT_NAME "cef-host"
    WIN32_EXECUTABLE TRUE 
    LINK_FLAGS "/ENTRY:WinMainCRTStartup"
)
