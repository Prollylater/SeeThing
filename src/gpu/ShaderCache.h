#ifndef GPU_SHADERCACHE_H
#define GPU_SHADERCACHE_H

#include "Program.h"

#include <filesystem>
#include <map>
#include <string>

namespace core
{
    class Log;
}

namespace gpu
{
    // Programs built once and looked up by name. Compile and link errors go to the log (stderr without one).
    class ShaderCache
    {
    public:
        explicit ShaderCache(core::Log *log = nullptr) : m_log(log) {}

        bool load(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath);
        bool add(const std::string &name, const std::string &vertexCode, const std::string &fragmentCode);

        // Throws std::out_of_range for an unknown name
        const Program &get(const std::string &name) const;
        bool contains(const std::string &name) const { return m_entries.count(name) != 0; }

        // Rebuilds the programs whose files changed; a failed rebuild keeps the old program. Returns the rebuild count.
        int reloadChanged();

        const std::string &lastError() const { return m_lastError; }

    private:
        struct Entry
        {
            Program program;
            std::filesystem::path vertexPath, fragmentPath;
            std::filesystem::file_time_type vertexTime, fragmentTime;
        };

        bool build(Entry &entry, const std::string &name, const std::string &vertexCode, const std::string &fragmentCode);
        bool buildFromFiles(Entry &entry, const std::string &name);
        void report(const std::string &message) const;

        core::Log *m_log = nullptr;

        std::map<std::string, Entry> m_entries;
        std::string m_lastError;
    };
}

#endif
