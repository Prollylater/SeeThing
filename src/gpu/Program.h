#ifndef GPU_PROGRAM_H
#define GPU_PROGRAM_H

#include "GlHandle.h"
#include "core/Geometry.h"

#include <initializer_list>
#include <string>

namespace gpu
{
    struct ShaderSource
    {
        GLenum stage;
        std::string code;
        std::string name; 
    };

    class Program
    {
    public:
        Program() = default;
        static Program link(std::initializer_list<ShaderSource> sources, std::string *log = nullptr);

        void use() const { glUseProgram(m_prog.get()); }
        GLuint id() const { return m_prog.get(); }
        bool empty() const { return !m_prog; }

        // No bind needed; a name the linker dropped is ignored, as GL does
        void set(const char *name, int v) const;
        void set(const char *name, float v) const;
        void set(const char *name, core::Vec2f v) const;
        void set(const char *name, core::Vec3f v) const;

    private:
        GLint location(const char *name) const { return glGetUniformLocation(m_prog.get(), name); }

        ProgramHandle m_prog;
    };
}

#endif
