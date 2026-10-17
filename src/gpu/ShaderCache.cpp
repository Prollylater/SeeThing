#include "ShaderCache.h"
#include "core/Log.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace fs = std::filesystem;

namespace gpu
{
    namespace
    {
        bool readFile(const fs::path &path, std::string &out)
        {
            std::ifstream in(path, std::ios::binary);
            if (!in)
                return false;
            std::stringstream buffer;
            buffer << in.rdbuf();
            out = buffer.str();
            return true;
        }

        fs::file_time_type modified(const fs::path &path)
        {
            std::error_code ec;
            fs::file_time_type t = fs::last_write_time(path, ec);
            return ec ? fs::file_time_type::min() : t;
        }
    }

    void ShaderCache::report(const std::string &message) const
    {
        if (m_log)
            m_log->add(core::LogLevel::Error, message);
        else
            std::cerr << message << "\n";
    }

    bool ShaderCache::build(Entry &entry, const std::string &name, const std::string &vertexCode, const std::string &fragmentCode)
    {
        std::string log;
        Program program = Program::link({{GL_VERTEX_SHADER, vertexCode, name + " vertex"},
                                         {GL_FRAGMENT_SHADER, fragmentCode, name + " fragment"}},
                                        &log);
        if (program.empty())
        {
            m_lastError = log;
            report("Shader \"" + name + "\" failed to build:\n" + log);
            return false;
        }
        entry.program = std::move(program);
        m_lastError.clear();
        return true;
    }

    bool ShaderCache::buildFromFiles(Entry &entry, const std::string &name)
    {
        entry.vertexTime = modified(entry.vertexPath);
        entry.fragmentTime = modified(entry.fragmentPath);
        std::string vertexCode, fragmentCode;
        for (const auto &[path, code] : {std::pair{&entry.vertexPath, &vertexCode}, std::pair{&entry.fragmentPath, &fragmentCode}})
            if (!readFile(*path, *code))
            {
                m_lastError = "cannot read " + path->string();
                report("Shader \"" + name + "\": " + m_lastError);
                return false;
            }
        return build(entry, name, vertexCode, fragmentCode);
    }

    bool ShaderCache::load(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
        Entry entry;
        entry.vertexPath = vertexPath;
        entry.fragmentPath = fragmentPath;
        if (!buildFromFiles(entry, name))
            return false;
        m_entries[name] = std::move(entry);
        return true;
    }

    bool ShaderCache::add(const std::string &name, const std::string &vertexCode, const std::string &fragmentCode)
    {
        Entry entry;
        if (!build(entry, name, vertexCode, fragmentCode))
            return false;
        m_entries[name] = std::move(entry);
        return true;
    }

    const Program &ShaderCache::get(const std::string &name) const
    {
        auto it = m_entries.find(name);
        if (it == m_entries.end())
            throw std::out_of_range("ShaderCache: no program named " + name);
        return it->second.program;
    }

    int ShaderCache::reloadChanged()
    {
        int rebuilt = 0;
        for (auto &[name, entry] : m_entries)
        {
            if (entry.vertexPath.empty())
                continue;
            if (modified(entry.vertexPath) == entry.vertexTime && modified(entry.fragmentPath) == entry.fragmentTime)
                continue;
            Entry fresh;
            fresh.vertexPath = entry.vertexPath;
            fresh.fragmentPath = entry.fragmentPath;
            const bool ok = buildFromFiles(fresh, name);
            // Remember the times even on failure, so a broken file is reported once, not every frame
            entry.vertexTime = fresh.vertexTime;
            entry.fragmentTime = fresh.fragmentTime;
            if (ok)
            {
                entry.program = std::move(fresh.program);
                ++rebuilt;
            }
        }
        return rebuilt;
    }
}
