# LuaCEF - Chromium Embedded Framework для Lua

[🇷🇺 Русский](#русский) | [🇬🇧 English](#english)

---

## English

### Overview

**LuaCEF** is a powerful integration library that brings the Chromium Embedded Framework (CEF) to Lua-based applications. It enables seamless integration of web browsers into Lua scripts, particularly designed for applications like GTA San Andreas multiplayer mods using SA-MP framework.

### ✨ Key Features

- **Web Browser Integration**: Embed a fully-featured web browser powered by Chromium
- **Lua API**: Simple and intuitive Lua API for browser control and interaction
- **Direct3D Support**: Native rendering to Direct3D textures for game integration
- **JavaScript-Lua Communication**: Seamless bidirectional communication between JavaScript and Lua code
- **Input Management**: Support for keyboard and mouse input handling
- **Offscreen Rendering**: Flexible rendering modes for optimal performance
- **Multi-browser Support**: Create and manage multiple browser instances simultaneously
- **Focus Management**: Built-in focus mode for UI interactions
- **Page Navigation**: Full browser history support with back/forward functionality
- **Dynamic FPS Control**: Adjustable frame rate for browser rendering

### 🏗️ Architecture

LuaCEF consists of four main components:

1. **Plugin** (`plugin/`) - The main plugin that interfaces with Lua
   - Web browser core and texture management
   - Direct3D integration
   - Process management
   
2. **Host** (`host/`) - IPC communication host
   - Browser instance management
   - IPC message handling
   - CEF process coordination

3. **Subprocess** (`subprocess/`) - CEF subprocess
   - Renderer process
   - GPU process handling
   - Additional browser processes

4. **Common** (`common/`) - Shared utilities
   - IPC (Inter-Process Communication) infrastructure
   - Shared memory management
   - Common data structures

### 🚀 Quick Start

#### Creating a Simple Browser Window

```lua
local LuaCEF = require("luacef")

-- Create a browser window at position (100, 100) with size 800x600
local browser = LuaCEF.create(
    "https://google.com",
    100, 100,
    800, 600,
    function(view)
        print("Browser created successfully!")
    end
)

-- Set up a callback when page finishes loading
browser:addOnLoadingCallback(function(view, httpStatusCode)
    print("Page loaded with status:", httpStatusCode)
end)

-- Execute JavaScript code
browser:executeJS("console.log('Hello from Lua!')")

-- Get current URL
local currentURL = browser:getURL()
print("Current URL:", currentURL)

-- Load a new URL
browser:loadURL("https://github.com")

-- Close the browser when done
browser:close()
```

#### Creating a Fullscreen Browser

```lua
local browser = LuaCEF.createFullscreen(
    "https://example.com",
    function(view)
        print("Fullscreen browser created")
    end
)
```

### 📚 API Reference

#### Module Functions

##### `LuaCEF.inited()`
Check if LuaCEF is initialized.
```lua
if LuaCEF.inited() then
    print("LuaCEF is ready!")
end
```

##### `LuaCEF.isAnyInputActive()`
Check if input is active in any browser window.
```lua
if LuaCEF.isAnyInputActive() then
    print("User is interacting with a browser")
end
```

##### `LuaCEF.create(url, x, y, width, height, onCreate)`
Create a new browser window.

**Parameters:**
- `url` (string): URL to load
- `x` (integer): X position
- `y` (integer): Y position  
- `width` (integer): Window width
- `height` (integer): Window height
- `onCreate` (function, optional): Callback when browser is created

**Returns:** RemoteWebView instance

##### `LuaCEF.createFullscreen(url, onCreate)`
Create a fullscreen browser window.

**Parameters:**
- `url` (string): URL to load
- `onCreate` (function, optional): Callback when browser is created

**Returns:** RemoteWebView instance

#### RemoteWebView Methods

##### Navigation Methods

```lua
-- Get current URL
local url = view:getURL()

-- Load a URL
view:loadURL("https://example.com")

-- Reload current page
view:reloadPage(false)  -- false to use cache, true to ignore cache

-- Navigate back
view:goBack()

-- Navigate forward
view:goForward()
```

##### Input & Focus Management

```lua
-- Check if input is active (has focus)
if view:isInputActive() then
    print("Browser has input focus")
end

-- Enable/disable input
view:setInput(true)   -- enable
view:setInput(false)  -- disable

-- Check if browser is focused
if view:isFocused() then
    print("Browser window is focused")
end

-- Set focus state
view:setFocused(true)

-- Enable/disable focus mode
view:setEnabledFocusMode(true)
if view:isEnabledFocusMode() then
    print("Focus mode is enabled")
end
```

##### Rendering Control

```lua
-- Enable/disable rendering
view:setRenderEnabled(true)
if view:isRenderEnabled() then
    print("Rendering is enabled")
end

-- Enable/disable offscreen rendering
view:setOffscreenRenderingEnabled(true)
if view:isOffscreenRenderingEnabled() then
    print("Offscreen rendering is active")
end

-- Get Direct3D texture
local texture = view:getTexture()

-- Set browser FPS
view:setFPS(60)
```

##### Event Callbacks

```lua
-- Called after browser is created
view:addOnAfterCreateCallback(function(v)
    print("Browser is ready")
end)

-- Called when page finishes loading
view:addOnLoadingCallback(function(v, httpStatusCode)
    print("Page loaded with status:", httpStatusCode)
end)

-- Called when browser window closes
view:addOnCloseCallback(function(v)
    print("Browser closed")
end)
```

##### JavaScript Communication

```lua
-- Execute JavaScript code
view:executeJS("document.title = 'New Title'")

-- Bind a Lua function to JavaScript
view:addBindFunction("myLuaFunction", function(view, args)
    print("Arguments from JavaScript:", args)
    -- args is a table with arguments passed from JS
    for i, arg in ipairs(args) do
        print(i, arg)
    end
end)
```

**Using bound functions from JavaScript:**
```javascript
// Call Lua function from JavaScript
lua.call("myLuaFunction", "hello", 123, true)
```

##### Lifecycle Management

```lua
-- Clear all callbacks
view:clearCallbacks()

-- Close the browser
view:close()
```

### 🔧 Building from Source

#### Prerequisites

- Windows 10 or later
- CMake 3.15 or higher
- Visual Studio 2022 or later with C++20 support
- CEF binaries (see Configuration)

#### Build Steps

1. **Download CEF binaries:**
   - Visit [CEF Builds](https://cef-builds.spotifycdn.com/index.html#windows32)
   - Download CEF version: **130.1.7+g59f3b14+chromium-130.0.6723.59**
   - Extract to `third_party/cef/`

2. **Configure and build:**
   ```bash
   mkdir build
   cd build
   cmake .. -A Win32
   cmake --build . --config Release
   ```

3. **Deployment:**
   - Built files are deployed to `build/deploy/`
   - Plugin file: `build/deploy/Release/luacef.asi`
   - Host: `build/deploy/Release/luacef/cef-host.exe`
   - Subprocess: `build/deploy/Release/luacef/cef-subprocess.exe`
   - Lua module: `build/deploy/Release/moonloader/lib/luacef.lua`
   - Installer: `build/deploy/Release/Install-LuaCEF.cmd`

### Installation and Host Location

Close the game and run `Install-LuaCEF.cmd` from the unpacked release. Select the
folder containing `gta_sa.exe`. The installer copies `luacef.asi` and the Lua
module to the game, installs the shared CEF runtime in
`%LOCALAPPDATA%\LuaCEF\runtime`, and sets the **user** environment variable
`LUACEF_HOST_PATH` to that directory. Administrator rights are not required for
the default runtime location; the game folder must be writable. An ASI loader
and MoonLoader must already be installed. Restart the game launcher after installation
so it inherits the updated environment.

The plugin searches for a regular `cef-host.exe` file in this order:

1. The directory in `LUACEF_HOST_PATH` (the directory containing the executable,
   not the executable path itself).
2. `<game directory>\luacef\cef-host.exe`.
3. `<game directory>\cef-host.exe`.

A missing/empty variable or a directory without the host automatically falls back
to the game. The game directory is determined from the running executable,
independently of the launcher's working directory. The subprocess, CEF DLLs,
resources and `locales` must be placed alongside the selected host. Caches and
logs remain in `<game directory>\luacef` and are separate for each game installation.

For a portable installation or a custom shared runtime location, run PowerShell
from the release directory:

```powershell
.\Install-LuaCEF.ps1 -GamePath 'D:\Games\GTA San Andreas' -Portable
.\Install-LuaCEF.ps1 -GamePath 'D:\Games\GTA San Andreas' -RuntimePath 'D:\LuaCEF\runtime'
```

`-Portable` leaves environment variables unchanged; an existing valid
`LUACEF_HOST_PATH` still takes priority. To configure a shared runtime manually:

```powershell
[Environment]::SetEnvironmentVariable('LUACEF_HOST_PATH', 'D:\LuaCEF\runtime', 'User')
```

When running the installer from the repository, pass
`-SourcePath '.\build\deploy\Release'`. `-WhatIf` validates the package and previews
installation without copying files or changing the environment.

The path lookup tests can run without CEF or the game SDKs:

```powershell
cmake -S tests/native -B build/path-tests
cmake --build build/path-tests --config Release
ctest --test-dir build/path-tests -C Release --output-on-failure
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Install-LuaCEF.Tests.ps1
```

### 📦 Directory Structure

```
LuaCEF/
├── common/                 # Shared components
│   └── ipc/               # Inter-process communication
├── host/                  # IPC host process
│   ├── Host.cpp
│   ├── BrowserView.cpp
│   └── IPCBrowserHandler.cpp
├── plugin/                # Main plugin (Lua interface)
│   ├── Plugin.cpp
│   ├── web/              # Web core functionality
│   ├── render/           # Rendering system
│   ├── process/          # Process management
│   └── utils/            # Utility functions
├── subprocess/            # CEF subprocess
├── luacef/               # Lua module
│   └── luacef.lua        # Lua API wrapper
├── tests/                # Test scripts
├── CMakeLists.txt        # CMake configuration
└── README.md             # This file
```

### 💡 Examples

#### Example 1: Dynamic UI with Web Technologies

```lua
local LuaCEF = require("luacef")

local browser = LuaCEF.create(
    "data:text/html,<h1>Hello</h1><button onclick='lua.call(\"btnClicked\")'>Click Me</button>",
    50, 50,
    600, 400
)

browser:addBindFunction("btnClicked", function(view)
    print("Button was clicked!")
    view:executeJS("alert('Lua says hello!')")
end)

browser:addOnLoadingCallback(function(view, status)
    print("UI loaded, status:", status)
end)
```

#### Example 2: Real-time Data Display

```lua
local LuaCEF = require("luacef")

local browser = LuaCEF.create(
    "https://example.com/dashboard",
    0, 0, 1024, 768
)

-- Update browser with data every second
local updateTimer = setTimer(function()
    browser:executeJS([[
        document.getElementById('stats').innerHTML = 
            'Server Time: ' + new Date().toLocaleTimeString()
    ]])
end, 1000, 0)

browser:addOnCloseCallback(function()
    killTimer(updateTimer)
end)
```

#### Example 3: Multiple Browser Windows

```lua
local LuaCEF = require("luacef")

local browsers = {}

-- Create multiple browser windows
browsers[1] = LuaCEF.create("https://google.com", 10, 10, 400, 300)
browsers[2] = LuaCEF.create("https://github.com", 420, 10, 400, 300)
browsers[3] = LuaCEF.create("https://stackoverflow.com", 10, 320, 810, 300)

-- Check if any has input
if LuaCEF.isAnyInputActive() then
    print("User is browsing")
end

-- Close all on shutdown
addEventHandler("onScriptTerminate", function(scr)
    if scr == script.this then
        for _, browser in ipairs(browsers) do
            browser:close()
        end
    end
end)
```

### 🐛 Troubleshooting

**Problem:** "plugin not found" error
- **Solution:** Ensure `luacef.asi` is properly installed and accessible from the game directory

**Problem:** Browser not rendering
- **Solution:** Check that Direct3D device is initialized and rendering is enabled with `setRenderEnabled(true)`

**Problem:** Input not working
- **Solution:** Verify input is enabled with `setInput(true)` and the browser has focus

**Problem:** JavaScript communication fails
- **Solution:** Ensure callbacks are properly stored to prevent garbage collection using `:_storeCallback()`

### 📝 Version History

- **1.1.1** - Current version with full CEF 130 support
- **1.0.0** - Initial release

### 📄 License

This project is provided as-is for use with compatible frameworks.

### 🤝 Contributing

Contributions are welcome! Please ensure:
- Code follows the existing style
- Features are well-tested
- Documentation is updated
- Commits are clearly described

### 📞 Support

For issues, questions, or contributions, please use the project's issue tracker.

---

## Русский

### Обзор

**LuaCEF** — это мощная библиотека интеграции, которая привносит Chromium Embedded Framework (CEF) в Lua-приложения. Она позволяет легко встроить полнофункциональный веб-браузер в Lua-скрипты, особенно разработана для приложений типа модов GTA San Andreas на базе SA-MP.

### ✨ Основные возможности

- **Интеграция браузера**: Встроение полнофункционального веб-браузера на базе Chromium
- **Lua API**: Простой и интуитивный API на Lua для управления браузером
- **Поддержка Direct3D**: Встроенный рендеринг в Direct3D текстуры для интеграции с играми
- **Двусторонняя коммуникация**: Беспрепятственный обмен данными между JavaScript и Lua
- **Управление вводом**: Поддержка обработки ввода с клавиатуры и мыши
- **Скрытый рендеринг**: Гибкие режимы рендеринга для оптимальной производительности
- **Поддержка нескольких окон**: Создание и управление несколькими экземплярами браузера одновременно
- **Управление фокусом**: Встроенный режим фокусировки для взаимодействия с UI
- **Навигация по страницам**: Полная поддержка истории браузера с кнопками вперед/назад
- **Динамическое управление FPS**: Регулируемая частота кадров для рендеринга браузера

### 🏗️ Архитектура

LuaCEF состоит из четырёх основных компонентов:

1. **Plugin** (`plugin/`) - Основной плагин для интерфейса с Lua
   - Ядро веб-браузера и управление текстурами
   - Интеграция с Direct3D
   - Управление процессами
   
2. **Host** (`host/`) - Хост межпроцессного взаимодействия
   - Управление экземплярами браузера
   - Обработка IPC сообщений
   - Координация процессов CEF

3. **Subprocess** (`subprocess/`) - Вспомогательный процесс CEF
   - Процесс рендера
   - Обработка GPU
   - Дополнительные процессы браузера

4. **Common** (`common/`) - Общие компоненты
   - Инфраструктура IPC (межпроцессное взаимодействие)
   - Управление общей памятью
   - Общие структуры данных

### 🚀 Быстрый старт

#### Создание простого окна браузера

```lua
local LuaCEF = require("luacef")

-- Создаём окно браузера в позиции (100, 100) размером 800x600
local browser = LuaCEF.create(
    "https://google.com",
    100, 100,
    800, 600,
    function(view)
        print("Браузер создан успешно!")
    end
)

-- Установка обработчика для завершения загрузки страницы
browser:addOnLoadingCallback(function(view, httpStatusCode)
    print("Страница загружена со статусом:", httpStatusCode)
end)

-- Выполнение JavaScript кода
browser:executeJS("console.log('Привет из Lua!')")

-- Получение текущего URL
local currentURL = browser:getURL()
print("Текущий URL:", currentURL)

-- Загрузка нового URL
browser:loadURL("https://github.com")

-- Закрытие браузера
browser:close()
```

#### Создание полноэкранного браузера

```lua
local browser = LuaCEF.createFullscreen(
    "https://example.com",
    function(view)
        print("Полноэкранный браузер создан")
    end
)
```

### 📚 Справочник API

#### Функции модуля

##### `LuaCEF.inited()`
Проверяет инициализирован ли LuaCEF.
```lua
if LuaCEF.inited() then
    print("LuaCEF готов к работе!")
end
```

##### `LuaCEF.isAnyInputActive()`
Проверяет активен ли ввод хотя бы в одном окне браузера.
```lua
if LuaCEF.isAnyInputActive() then
    print("Пользователь взаимодействует с браузером")
end
```

##### `LuaCEF.create(url, x, y, width, height, onCreate)`
Создаёт новое окно браузера.

**Параметры:**
- `url` (string): URL для загрузки
- `x` (integer): Позиция X
- `y` (integer): Позиция Y  
- `width` (integer): Ширина окна
- `height` (integer): Высота окна
- `onCreate` (function, опционально): Обработчик создания браузера

**Возвращает:** Экземпляр RemoteWebView

##### `LuaCEF.createFullscreen(url, onCreate)`
Создаёт полноэкранное окно браузера.

**Параметры:**
- `url` (string): URL для загрузки
- `onCreate` (function, опционально): Обработчик создания браузера

**Возвращает:** Экземпляр RemoteWebView

#### Методы RemoteWebView

##### Методы навигации

```lua
-- Получить текущий URL
local url = view:getURL()

-- Загрузить URL
view:loadURL("https://example.com")

-- Перезагрузить текущую страницу
view:reloadPage(false)  -- false использует кэш, true игнорирует

-- Перейти назад
view:goBack()

-- Перейти вперед
view:goForward()
```

##### Управление вводом и фокусом

```lua
-- Проверить активен ли ввод (имеет ли фокус)
if view:isInputActive() then
    print("Браузер имеет фокус ввода")
end

-- Включить/выключить ввод
view:setInput(true)   -- включить
view:setInput(false)  -- выключить

-- Проверить имеет ли браузер фокус
if view:isFocused() then
    print("Окно браузера в фокусе")
end

-- Установить состояние фокуса
view:setFocused(true)

-- Включить/выключить режим фокусировки
view:setEnabledFocusMode(true)
if view:isEnabledFocusMode() then
    print("Режим фокусировки включен")
end
```

##### Управление рендерингом

```lua
-- Включить/выключить рендеринг
view:setRenderEnabled(true)
if view:isRenderEnabled() then
    print("Рендеринг включен")
end

-- Включить/выключить скрытый рендеринг
view:setOffscreenRenderingEnabled(true)
if view:isOffscreenRenderingEnabled() then
    print("Скрытый рендеринг активен")
end

-- Получить Direct3D текстуру
local texture = view:getTexture()

-- Установить FPS браузера
view:setFPS(60)
```

##### Обработчики событий

```lua
-- Вызывается после создания браузера
view:addOnAfterCreateCallback(function(v)
    print("Браузер готов")
end)

-- Вызывается когда страница закончила загружаться
view:addOnLoadingCallback(function(v, httpStatusCode)
    print("Страница загружена со статусом:", httpStatusCode)
end)

-- Вызывается когда окно браузера закрывается
view:addOnCloseCallback(function(v)
    print("Браузер закрыт")
end)
```

##### Коммуникация с JavaScript

```lua
-- Выполнить JavaScript код
view:executeJS("document.title = 'Новый Заголовок'")

-- Привязать Lua функцию к JavaScript
view:addBindFunction("myLuaFunction", function(view, args)
    print("Аргументы из JavaScript:", args)
    -- args это таблица с аргументами переданными из JS
    for i, arg in ipairs(args) do
        print(i, arg)
    end
end)
```

**Использование привязанных функций из JavaScript:**
```javascript
// Вызвать Lua функцию из JavaScript
lua.call("myLuaFunction", "привет", 123, true)
```

##### Управление жизненным циклом

```lua
-- Очистить все обработчики
view:clearCallbacks()

-- Закрыть браузер
view:close()
```

### 🔧 Сборка из исходного кода

#### Предварительные требования

- Windows 10 или более поздняя версия
- CMake 3.15 или выше
- Visual Studio 2022 или выше с поддержкой C++20
- Бинарные файлы CEF (см. Конфигурация)

#### Шаги сборки

1. **Загрузить бинарные файлы CEF:**
   - Посетите [CEF Builds](https://cef-builds.spotifycdn.com/index.html#windows32)
   - Загрузите версию CEF: **130.1.7+g59f3b14+chromium-130.0.6723.59**
   - Распакуйте в `third_party/cef/`

2. **Конфигурация и сборка:**
   ```bash
   mkdir build
   cd build
   cmake .. -A Win32
   cmake --build . --config Release
   ```

3. **Развёртывание:**
   - Собранные файлы располагаются в `build/deploy/`
   - Файл плагина: `build/deploy/Release/luacef.asi`
   - Host: `build/deploy/Release/luacef/cef-host.exe`
   - Subprocess: `build/deploy/Release/luacef/cef-subprocess.exe`
   - Lua-модуль: `build/deploy/Release/moonloader/lib/luacef.lua`
   - Установщик: `build/deploy/Release/Install-LuaCEF.cmd`

### Установка и поиск хоста

Закройте игру и запустите `Install-LuaCEF.cmd` из распакованного релиза. Выберите
папку с `gta_sa.exe`. Установщик скопирует `luacef.asi` и Lua-модуль в игру,
разместит общий CEF в `%LOCALAPPDATA%\LuaCEF\runtime` и запишет этот каталог
в **пользовательскую** переменную среды `LUACEF_HOST_PATH`. Для стандартного
каталога CEF права администратора не нужны; папка игры должна быть доступна
для записи. ASI Loader и MoonLoader должны быть установлены заранее.
После установки перезапустите лаунчер, чтобы игра получила обновлённую переменную.

Плагин ищет файл `cef-host.exe` в следующем порядке:

1. Каталог из `LUACEF_HOST_PATH`: указывается папка с хостом, а не путь к `.exe`.
2. `<папка игры>\luacef\cef-host.exe`.
3. `<папка игры>\cef-host.exe`.

Если переменная отсутствует, пуста или в её каталоге нет файла хоста, поиск
автоматически продолжается в игре. Папка игры определяется по запущенному
исполняемому файлу и не зависит от рабочей папки лаунчера. Рядом с выбранным
хостом должны лежать subprocess, DLL CEF, ресурсы и каталог `locales`.
Кэш и логи сохраняются отдельно для каждой игры в `<папка игры>\luacef`.

Для переносимой установки или другого расположения общего CEF запустите
PowerShell из папки релиза:

```powershell
.\Install-LuaCEF.ps1 -GamePath 'D:\Games\GTA San Andreas' -Portable
.\Install-LuaCEF.ps1 -GamePath 'D:\Games\GTA San Andreas' -RuntimePath 'D:\LuaCEF\runtime'
```

Режим `-Portable` не меняет переменные среды; уже заданный корректный
`LUACEF_HOST_PATH` сохраняет приоритет. Для ручной настройки общего CEF:

```powershell
[Environment]::SetEnvironmentVariable('LUACEF_HOST_PATH', 'D:\LuaCEF\runtime', 'User')
```

При запуске установщика из репозитория добавьте
`-SourcePath '.\build\deploy\Release'`. Параметр `-WhatIf` проверяет комплект
файлов и показывает план установки без копирования и изменения переменных.

Проверки поиска хоста запускаются без CEF и SDK игры:

```powershell
cmake -S tests/native -B build/path-tests
cmake --build build/path-tests --config Release
ctest --test-dir build/path-tests -C Release --output-on-failure
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Install-LuaCEF.Tests.ps1
```

### 📦 Структура проекта

```
LuaCEF/
├── common/                 # Общие компоненты
│   └── ipc/               # Межпроцессное взаимодействие
├── host/                  # Процесс хоста IPC
│   ├── Host.cpp
│   ├── BrowserView.cpp
│   └── IPCBrowserHandler.cpp
├── plugin/                # Основной плагин (интерфейс Lua)
│   ├── Plugin.cpp
│   ├── web/              # Функциональность ядра веб-браузера
│   ├── render/           # Система рендеринга
│   ├── process/          # Управление процессами
│   └── utils/            # Вспомогательные функции
├── subprocess/            # Вспомогательный процесс CEF
├── luacef/               # Модуль Lua
│   └── luacef.lua        # Обёртка Lua API
├── tests/                # Тестовые скрипты
├── CMakeLists.txt        # Конфигурация CMake
└── README.md             # Этот файл
```

### 💡 Примеры использования

#### Пример 1: Динамический UI на веб-технологиях

```lua
local LuaCEF = require("luacef")

local browser = LuaCEF.create(
    "data:text/html,<h1>Привет</h1><button onclick='lua.call(\"btnClicked\")'>Нажми меня</button>",
    50, 50,
    600, 400
)

browser:addBindFunction("btnClicked", function(view)
    print("Кнопка была нажата!")
    view:executeJS("alert('Привет из Lua!')")
end)

browser:addOnLoadingCallback(function(view, status)
    print("UI загружен, статус:", status)
end)
```

#### Пример 2: Отображение данных в реальном времени

```lua
local LuaCEF = require("luacef")

local browser = LuaCEF.create(
    "https://example.com/dashboard",
    0, 0, 1024, 768
)

-- Обновляем браузер данными каждую секунду
local updateTimer = setTimer(function()
    browser:executeJS([[
        document.getElementById('stats').innerHTML = 
            'Время сервера: ' + new Date().toLocaleTimeString('ru-RU')
    ]])
end, 1000, 0)

browser:addOnCloseCallback(function()
    killTimer(updateTimer)
end)
```

#### Пример 3: Несколько окон браузера

```lua
local LuaCEF = require("luacef")

local browsers = {}

-- Создаём несколько окон браузера
browsers[1] = LuaCEF.create("https://google.com", 10, 10, 400, 300)
browsers[2] = LuaCEF.create("https://github.com", 420, 10, 400, 300)
browsers[3] = LuaCEF.create("https://stackoverflow.com", 10, 320, 810, 300)

-- Проверяем активен ли ввод
if LuaCEF.isAnyInputActive() then
    print("Пользователь просматривает")
end

-- Закрываем все при завершении скрипта
addEventHandler("onScriptTerminate", function(scr)
    if scr == script.this then
        for _, browser in ipairs(browsers) do
            browser:close()
        end
    end
end)
```

### 🐛 Решение проблем

**Проблема:** Ошибка "plugin not found"
- **Решение:** Убедитесь, что `luacef.asi` правильно установлен и доступен в директории игры

**Проблема:** Браузер не отображается
- **Решение:** Проверьте что Direct3D устройство инициализировано и рендеринг включен с `setRenderEnabled(true)`

**Проблема:** Ввод не работает
- **Решение:** Проверьте что ввод включен с `setInput(true)` и браузер имеет фокус

**Проблема:** Коммуникация с JavaScript не работает
- **Решение:** Убедитесь что обработчики должным образом сохраняются для предотвращения их сборки мусоропровода

### 📝 История версий

- **1.1.1** - Текущая версия с полной поддержкой CEF 130
- **1.0.0** - Первоначальный релиз

### 📄 Лицензия

Этот проект предоставляется как есть для использования с совместимыми фреймворками.

### 🤝 Вклад

Приветствуются вклады! Пожалуйста убедитесь что:
- Код соответствует существующему стилю
- Функции хорошо протестированы
- Документация обновлена
- Коммиты имеют четкое описание

### 📞 Поддержка

По вопросам, проблемам или предложениям используйте трекер проблем проекта.
