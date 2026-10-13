#include "RuntimePaths.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void Touch(const fs::path& file)
{
    fs::create_directories(file.parent_path());
    std::ofstream stream(file);
    Require(stream.good(), "Failed to create a test file");
}

int main()
{
    const auto originalDirectory = fs::current_path();
    const auto originalEnvironment = luacef::paths::HostEnvironmentDirectory();
    const auto temporaryDirectory = fs::temp_directory_path();
    const auto testDirectory = temporaryDirectory /
                               (L"LuaCEF-path-tests-" + std::to_wstring(GetCurrentProcessId()) + L"-" +
                                std::to_wstring(GetTickCount64()));
    auto cleanup = [&]()
    {
        fs::current_path(originalDirectory);
        SetEnvironmentVariableW(luacef::paths::HostEnvironmentVariable,
                                originalEnvironment.empty() ? nullptr : originalEnvironment.c_str());
        Require(testDirectory.is_absolute() && fs::equivalent(testDirectory.parent_path(), temporaryDirectory) &&
                    testDirectory.filename().wstring().starts_with(L"LuaCEF-path-tests-"),
                "Invalid test cleanup directory");
        fs::remove_all(testDirectory);
    };
    try
    {
        const auto game = testDirectory / L"game";
        const auto shared = testDirectory / L"shared runtime \u0422\u0435\u0441\u0442";
        const auto localHost = game / L"luacef" / L"cef-host.exe";
        const auto sharedHost = shared / L"cef-host.exe";
        Touch(localHost);
        Touch(game / L"cef-host.exe");
        Touch(sharedHost);

        Require(luacef::paths::FindHost(shared, game) == sharedHost, "Environment must take priority");
        Require(luacef::paths::FindHost({}, game) == localHost, "Missing variable must use game/luacef");
        Require(luacef::paths::FindHost(testDirectory / L"missing", game) == localHost,
                "Missing environment directory must fall back");
        fs::remove(sharedHost);
        Require(luacef::paths::FindHost(shared, game) == localHost,
                "Environment directory without the host must fall back");
        fs::create_directory(sharedHost);
        Require(luacef::paths::FindHost(shared, game) == localHost,
                "A directory named cef-host.exe must not be accepted");
        fs::remove(localHost);
        Require(luacef::paths::FindHost({}, game) == game / L"cef-host.exe", "Game root fallback failed");
        fs::remove(game / L"cef-host.exe");
        Require(luacef::paths::FindHost(shared, game).empty(), "Missing hosts must return an empty path");
        Require(luacef::paths::FindHost({}, {}).empty(), "An unknown game directory must not use the CWD");

        SetEnvironmentVariableW(luacef::paths::HostEnvironmentVariable, shared.c_str());
        Require(luacef::paths::HostEnvironmentDirectory() == shared, "Unicode environment path changed");
        const auto quoted = L"\"" + shared.wstring() + L"\"";
        SetEnvironmentVariableW(luacef::paths::HostEnvironmentVariable, quoted.c_str());
        Require(luacef::paths::HostEnvironmentDirectory() == shared, "Quoted environment path failed");
        const std::wstring longValue(500, L'x');
        SetEnvironmentVariableW(luacef::paths::HostEnvironmentVariable, longValue.c_str());
        Require(luacef::paths::HostEnvironmentDirectory().wstring() == longValue,
                "Environment value must not be truncated at MAX_PATH");
        SetEnvironmentVariableW(luacef::paths::HostEnvironmentVariable, nullptr);
        Require(luacef::paths::HostEnvironmentDirectory().empty(), "Unset environment variable was not empty");

        const auto executableDirectory = luacef::paths::ExecutableDirectory();
        Require(!executableDirectory.empty(), "Executable directory could not be resolved");
        const auto unrelated = testDirectory / L"launcher working directory";
        Touch(unrelated / L"luacef" / L"cef-host.exe");
        Touch(localHost);
        fs::current_path(unrelated);
        Require(luacef::paths::ExecutableDirectory() == executableDirectory,
                "Executable directory must be independent of the CWD");
        Require(luacef::paths::FindHost({}, game) == localHost, "CWD must not override the game directory");
        cleanup();
        std::cout << "All runtime path tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        cleanup();
        return 1;
    }
}
