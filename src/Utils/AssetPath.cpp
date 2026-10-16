#include "AssetPath.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

static fs::path executableDir()
{
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
    {
        return {};
    }
    return fs::path(std::wstring(buffer, length)).parent_path();
#elif defined(__linux__)
    std::error_code ec;
    fs::path exe = fs::read_symlink("/proc/self/exe", ec);
    return ec ? fs::path() : exe.parent_path();
#else
    return {};
#endif
}

static fs::path assetRoot()
{
    const char *env = std::getenv("SEETHING_ASSET_DIR");
    if (env != nullptr && *env != '\0')
    {
        return env;
    }
    fs::path dir = executableDir();
    std::error_code ec;
    if (!dir.empty() && fs::is_directory(dir / "ressources", ec))
    {
        return dir / "ressources";
    }
    return "ressources";
}

std::string assetPath(const std::string &relative)
{
    static const fs::path root = assetRoot();
    return (root / relative).string();
}
