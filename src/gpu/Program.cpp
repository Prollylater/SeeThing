#include "Program.h"

#include <vector>

namespace gpu
{
    namespace
    {
        const char *stageName(GLenum stage)
        {
            switch (stage)
            {
            case GL_VERTEX_SHADER: return "vertex";
            case GL_FRAGMENT_SHADER: return "fragment";
            case GL_COMPUTE_SHADER: return "compute";
            case GL_GEOMETRY_SHADER: return "geometry";
            }
            return "shader";
        }

        std::string shaderLog(GLuint shader)
        {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::string log(std::size_t(length > 0 ? length : 0), '\0');
            if (length > 0)
                glGetShaderInfoLog(shader, length, nullptr, log.data());
            while (!log.empty() && (log.back() == '\0' || log.back() == '\n'))
                log.pop_back();
            return log;
        }

        std::string programLog(GLuint program)
        {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::string log(std::size_t(length > 0 ? length : 0), '\0');
            if (length > 0)
                glGetProgramInfoLog(program, length, nullptr, log.data());
            while (!log.empty() && (log.back() == '\0' || log.back() == '\n'))
                log.pop_back();
            return log;
        }
    }

    Program Program::link(std::initializer_list<ShaderSource> sources, std::string *log)
    {
        std::string messages;
        std::vector<ShaderHandle> shaders;
        bool ok = true;
        for (const ShaderSource &src : sources)
        {
            ShaderHandle shader(glCreateShader(src.stage));
            const char *code = src.code.c_str();
            glShaderSource(shader.get(), 1, &code, nullptr);
            glCompileShader(shader.get());
            GLint compiled = GL_FALSE;
            glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &compiled);
            if (!compiled)
            {
                ok = false;
                messages += src.name + " (" + stageName(src.stage) + "): " + shaderLog(shader.get()) + "\n";
            }
            shaders.push_back(std::move(shader));
        }

        Program program;
        if (ok)
        {
            program.m_prog = ProgramHandle::create();
            const GLuint id = program.m_prog.get();
            for (const ShaderHandle &s : shaders)
                glAttachShader(id, s.get());
            glLinkProgram(id);
            for (const ShaderHandle &s : shaders)
                glDetachShader(id, s.get());
            GLint linked = GL_FALSE;
            glGetProgramiv(id, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                messages += "link: " + programLog(id) + "\n";
                program.m_prog.reset();
            }
        }
        if (log)
            *log = std::move(messages);
        return program;
    }

    void Program::set(const char *name, int v) const { glProgramUniform1i(m_prog.get(), location(name), v); }
    void Program::set(const char *name, float v) const { glProgramUniform1f(m_prog.get(), location(name), v); }
    void Program::set(const char *name, core::Vec2f v) const { glProgramUniform2f(m_prog.get(), location(name), v.x, v.y); }
    void Program::set(const char *name, core::Vec3f v) const { glProgramUniform3f(m_prog.get(), location(name), v.x, v.y, v.z); }
}
