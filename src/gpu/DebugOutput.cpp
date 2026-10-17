#include "DebugOutput.h"
#include "core/Log.h"

#include <glad/gl.h>

#include <atomic>
#include <cstdio>
#include <string>

namespace gpu
{
    namespace
    {
        std::atomic<int> messageCount{0};

        const char *sourceName(GLenum source)
        {
            switch (source)
            {
            case GL_DEBUG_SOURCE_API: return "api";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "window";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "third party";
            case GL_DEBUG_SOURCE_APPLICATION: return "app";
            }
            return "other";
        }

        const char *typeName(GLenum type)
        {
            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR: return "error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "undefined behavior";
            case GL_DEBUG_TYPE_PORTABILITY: return "portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return "performance";
            case GL_DEBUG_TYPE_MARKER: return "marker";
            }
            return "other";
        }

        const char *severityName(GLenum severity)
        {
            switch (severity)
            {
            case GL_DEBUG_SEVERITY_HIGH: return "high";
            case GL_DEBUG_SEVERITY_MEDIUM: return "medium";
            case GL_DEBUG_SEVERITY_LOW: return "low";
            }
            return "info";
        }

        void GLAD_API_PTR onMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar *message, const void *user)
        {
            ++messageCount;
            auto *log = static_cast<core::Log *>(const_cast<void *>(user));
            if (!log)
            {
                std::fprintf(stderr, "GL %s %s (%s, id %u): %s\n", severityName(severity), typeName(type), sourceName(source), id, message);
                return;
            }
            const core::LogLevel level = type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH ? core::LogLevel::Error : core::LogLevel::Warning;
            log->add(level, std::string("GL ") + typeName(type) + " (" + sourceName(source) + ", id " + std::to_string(id) + "): " + message);
        }
    }

    bool enableDebugOutput(core::Log *log)
    {
        GLint flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT))
            return false;
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(onMessage, log);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        // Mesa repeats compile errors here; Program already returns the info log
        glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
        return true;
    }

    int debugMessageCount() { return messageCount; }
    void resetDebugMessageCount() { messageCount = 0; }
}
