#include "Plugin.hpp"

#include <chrono>
#include <iostream>

#include <kthook/kthook.hpp>
#include <plugin.h>

#include "Logger.hpp"
#include "RuntimePaths.hpp"
#include "render/RenderManager.hpp"

#include "utils/samp_utils.hpp"

void Plugin::OnAttach(void* handle)
{
    Logger::Instance().SetLogFilePath(luacef::paths::ExecutableDirectory() / L"luacef" / L"plugin.log");

    Logger::Instance().Log("Plugin attached with handle: {}", handle);

    webCore_ = std::make_shared<WebCore>();
    renderManager_ = std::make_shared<RenderManager>();

    plugin::Events::initRwEvent += [this]() { OnRenderInit(); };
}

void Plugin::OnDetach()
{
    webCore_->Shutdown();
    renderManager_->Shutdown();

    Logger::Instance().Log("Plugin detached.");
}

void Plugin::InstallWndProcHook(uintptr_t address)
{
    wndProcHook_ = std::make_unique<WndProcHook>(address,
                                                 [this](auto& hook, auto&&... args) -> LRESULT
                                                 {
                                                     if (OnWndProc(std::forward<decltype(args)>(args)...))
                                                         return 1;

                                                     return hook.call_trampoline(std::forward<decltype(args)>(args)...);
                                                 });

    Logger::Instance().Log("WndProc changed");
}

bool Plugin::IsInitialized() const
{
    return webCore_ != nullptr && webCore_->IsInitialized();
}

RemoteWebView* Plugin::Create(const char* url, int x, int y, int width, int height,
                              RemoteWebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    Logger::Instance().Log("Creating web view with URL: {}, x: {}, y: {}, width: {}, height: {}", url, x, y, width,
                           height);

    auto webView = webCore_->Create(url, x, y, width, height, onCreate);
    return webView;
}

RemoteWebView* Plugin::CreateFullscreen(const char* url, RemoteWebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    Logger::Instance().Log("Creating fullscreen web view with URL: {}", url);

    int width = RsGlobal.maximumWidth;
    int height = RsGlobal.maximumHeight;

    if (auto* device = GetD3DDevice())
    {
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)) && backBuffer)
        {
            D3DSURFACE_DESC desc{};
            if (SUCCEEDED(backBuffer->GetDesc(&desc)))
            {
                width = static_cast<int>(desc.Width);
                height = static_cast<int>(desc.Height);
            }

            backBuffer->Release();
        }
    }

    Logger::Instance().Log("Fullscreen web view size: {}x{}", width, height);

    auto webView = webCore_->Create(url, 0, 0, width, height, onCreate);
    return webView;
}

void Plugin::Close(RemoteWebView* view)
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("Closing web view with id {}", view->GetBrowserId());
    webCore_->Close(view);
}

void Plugin::OnRenderInit()
{
    Logger::Instance().Log("Initializing RenderManager...");
    renderManager_->Initialize();
    Logger::Instance().Log("RenderManager initialized.");

    Logger::Instance().Log("Initializing WebCore...");
    webCore_->Initialize(GetD3DDevice(), renderManager_);
    Logger::Instance().Log("WebCore initialized.");
}

void Plugin::OnD3DLost()
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("D3D device lost.");

    webCore_->OnDeviceLost();
    renderManager_->OnDeviceLost();
}

void Plugin::OnD3DReset()
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("D3D device reset.");

    webCore_->OnDeviceReset();
    renderManager_->OnDeviceReset();
}

void Plugin::Update()
{
    if (!IsInitialized())
        return;

    webCore_->Update();
    UpdateCredits();

    static std::chrono::steady_clock::time_point lastTime;
    const auto currentTime = std::chrono::steady_clock::now();
    if (currentTime - lastTime < std::chrono::seconds(1))
        return;

    Logger::Instance().Log("Update with time");

    lastTime = currentTime;

    static auto lastWndProcAddr = 0;
    if (lastWndProcAddr != GetWindowLongPtr(RsGlobal.ps->window, GWLP_WNDPROC))
    {
        lastWndProcAddr = GetWindowLongPtrW(RsGlobal.ps->window, GWLP_WNDPROC);
        InstallWndProcHook(lastWndProcAddr);
    }
}

bool Plugin::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!IsInitialized())
        return false;

    return webCore_->OnWndProc(hwnd, msg, wParam, lParam);
}

void Plugin::UpdateCredits()
{
    static bool init{};
    static std::chrono::steady_clock::time_point lastTimeUpdate{};
    if (init && std::chrono::steady_clock::now() - lastTimeUpdate > std::chrono::seconds(4) && creditsWebView_)
    {
        Close(creditsWebView_);
        creditsWebView_ = nullptr;
    }
    if (!init && IsInitialized())
    {
        static auto UrlEncode = [](std::string_view str) -> std::string
        {
            std::ostringstream out;
            out << std::uppercase << std::hex;

            for (unsigned char c : str)
            {
                if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                {
                    out << c;
                }
                else
                {
                    out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
                }
            }

            return out.str();
        };
        lastTimeUpdate = std::chrono::steady_clock::now();
        init = true;
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
html, body {
    margin: 0;
    padding: 0;
    overflow: hidden;
    background: transparent;
    font-family: "Segoe UI", Arial, sans-serif;
}

#credits {
    position: absolute;
    right: 20px;
    top: 20px;

    padding: 14px 18px;
    border-radius: 10px;

    background: rgba(18,18,22,0.72);
    backdrop-filter: blur(8px);

    border-left: 4px solid #4da3ff;

    color: white;

    box-shadow: 0 8px 30px rgba(0,0,0,.35);

    user-select: none;
}

.title {
    font-size: 13px;
    color: #ffffff;
    font-weight: 700;
    margin-bottom: 8px;
}

.line {
    font-size: 12px;
    color: #d5d5d5;
    margin-top: 3px;
}

.author {
    color: #6fb8ff;
    font-weight: 600;
}

.provider {
    color: #8cf7c4;
    font-weight: 600;
}

.server {
    color: #ffd36e;
    font-weight: 600;
}
</style>
</head>

<body>

<div id="credits">
    <div class="title">LuaCEF Demo</div>

    <div class="line">
        Developed by <span class="author">whyega</span>
    </div>

    <div class="line">
        Provided by <span class="provider">Ethereum Longer</span>
    </div>

    <div class="line">
        For <span class="server">Advance RP</span>
    </div>
</div>

</body>
</html>
)";
        auto creditsCode = "data:text/html;charset=utf-8," + UrlEncode(html);
        creditsWebView_ = Create(creditsCode.c_str(), 10, 10, 200, 150, [](auto&&... args) {});
        creditsWebView_->SetEnabledFocusMode(false);
        creditsWebView_->SetInputEnabled(false);
    }
}
