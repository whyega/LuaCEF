
add_library(LuaCEFPlugin SHARED)

target_sources(LuaCEFPlugin PRIVATE
    process/ProcessManager.cpp
    render/RenderManager.cpp
    utils/samp_utils.cpp
    web/WebCore.cpp
    web/RemoteWebView.cpp

    main.cpp
    Plugin.cpp
)

include(FetchContent)

FetchContent_Declare(
    plugintmplt
    GIT_REPOSITORY https://github.com/whyega/plugintmplt.git
    GIT_TAG v1.0.2
)

FetchContent_Declare(xbyak
  GIT_REPOSITORY https://github.com/herumi/xbyak.git
  GIT_TAG ce083a0dcc306c1717685a81f577a4e050193919
)

FetchContent_Declare(ktsignal
  GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git
  GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf
)

FetchContent_Declare(kthook
  GIT_REPOSITORY https://github.com/kin4stat/kthook.git
  GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b
)

FetchContent_Declare(sampapi
  GIT_REPOSITORY https://github.com/BlastHackNet/SAMP-API.git
  GIT_TAG 6d4db99ab41f19d1a6a7c6cd48f5878bd1e14b62
)

FetchContent_MakeAvailable(plugintmplt xbyak ktsignal kthook sampapi)

find_package(plugin-sdk REQUIRED)

target_link_libraries(LuaCEFPlugin PRIVATE
    cef-ipc
    cef-common
    plugintmplt
    pluginsdk::gtasa
    kthook
    sampapi
)

target_include_directories(LuaCEFPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(LuaCEFPlugin PRIVATE cxx_std_20)

set_target_properties(LuaCEFPlugin PROPERTIES
    SUFFIX .asi
    OUTPUT_NAME luacef
)

# --- DirectX SDK (June 2010) ---
# Можно задавать:
#  - через -DDXSDK_PATH=...
#  - или через env DXSDK_DIR (как в установщике DXSDK)
set(DXSDK_PATH "" CACHE PATH "Path to DirectX SDK (June 2010), must contain Include/ and Lib/x86")

if(NOT DXSDK_PATH)
  if(DEFINED ENV{DXSDK_DIR})
    # у DXSDK_DIR обычно со слэшем на конце, но нам это не мешает
    set(DXSDK_PATH "$ENV{DXSDK_DIR}")
  else()
    set(DXSDK_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
  endif()
endif()

# Нормализуем слэши (для Windows путей)
file(TO_CMAKE_PATH "${DXSDK_PATH}" DXSDK_PATH)

if(EXISTS "${DXSDK_PATH}/Include" AND EXISTS "${DXSDK_PATH}/Lib/x86")
  add_library(dx9sdk INTERFACE)
  target_include_directories(dx9sdk INTERFACE "${DXSDK_PATH}/Include")
  target_link_directories(dx9sdk INTERFACE "${DXSDK_PATH}/Lib/x86")
  target_link_libraries(dx9sdk INTERFACE
    d3d9
    $<$<CONFIG:Debug>:d3dx9d>
    $<$<CONFIG:Release>:d3dx9>
)
else()
  message(FATAL_ERROR "DirectX SDK (June 2010) not found. DXSDK_PATH='${DXSDK_PATH}'. Expected Include/ and Lib/x86/")
endif()

target_link_libraries(LuaCEFPlugin PUBLIC dx9sdk)
