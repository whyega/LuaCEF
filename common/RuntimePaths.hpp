#pragma once

#include <Windows.h>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

namespace luacef::paths
{
inline constexpr wchar_t HostEnvironmentVariable[] = L"LUACEF_HOST_PATH";

inline std::filesystem::path ExecutableDirectory()
{
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;)
    {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
            return {};
        if (length < buffer.size())
        {
            buffer.resize(length);
            return std::filesystem::path(std::move(buffer)).parent_path();
        }
        if (buffer.size() >= 32768)
            return {};
        buffer.resize(buffer.size() * 2);
    }
}

inline std::filesystem::path HostEnvironmentDirectory()
{
    DWORD size = GetEnvironmentVariableW(HostEnvironmentVariable, nullptr, 0);
    while (size != 0)
    {
        std::wstring buffer(size, L'\0');
        const DWORD length = GetEnvironmentVariableW(HostEnvironmentVariable, buffer.data(), size);
        if (length == 0)
            return {};
        if (length >= size)
        {
            size = length;
            continue;
        }
        buffer.resize(length);
        if (buffer.size() >= 2 && buffer.front() == L'"' && buffer.back() == L'"')
            buffer = buffer.substr(1, buffer.size() - 2);
        return std::filesystem::path(std::move(buffer));
    }
    return {};
}

inline std::filesystem::path HostExecutable(const std::filesystem::path& directory)
{
    if (directory.empty())
        return {};
    std::error_code error;
    const auto candidate = directory / L"cef-host.exe";
    if (!std::filesystem::is_regular_file(candidate, error))
        return {};
    const auto absolute = std::filesystem::absolute(candidate, error);
    return error ? std::filesystem::path{} : absolute;
}

inline std::filesystem::path FindHost(const std::filesystem::path& environmentDirectory,
                                     const std::filesystem::path& gameDirectory)
{
    if (auto host = HostExecutable(environmentDirectory); !host.empty())
        return host;
    if (gameDirectory.empty())
        return {};
    if (auto host = HostExecutable(gameDirectory / L"luacef"); !host.empty())
        return host;
    return HostExecutable(gameDirectory);
}
}
