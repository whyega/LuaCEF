add_library(cef-ipc STATIC)

target_sources(cef-ipc PRIVATE
    IPCServer.cpp
    IPCClient.cpp
    SharedMemory.cpp
)

target_compile_features(cef-ipc PRIVATE cxx_std_20)

target_link_libraries(cef-ipc
    cef-common
)

target_include_directories(cef-ipc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
