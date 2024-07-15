#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>

#include "glcore.h"
#include "Vec.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

GLuint compileShader(const GLuint program, const GLenum shader_type, const std::string &source);
bool compileProgram(const GLuint program);

std::string read(const std::string &filename);

struct Program
{
    GLuint shader_id;

    Program() = default;
    // constructor reads and builds the shader
    void init(const char *vertex_path, const char *fragment_path);

    // use/activate the shader
    void use();
    void load(const char *vertex_path, const char *fragment_path);
    void reset();
    void detachAllShaders(GLuint program);

    void addUniform1(const char *uniform, const unsigned int value);
    void addUniform1(const char *uniform, const int &value);

    void addUniform1(const char *uniform, const float &value);

    void addUniform2(const char *uniform, const Vec<GLfloat> &value);

    void addUniform2(const char *uniform, const Vec<GLuint> &value);
    void addUniform3(const char *uniform, const Vec<GLfloat> &value);
    void addUniform3(const char *uniform, const Vec<GLuint> &value);
};

// GL_VERTEX_SHADER
// GL_FRAGMENT_SHADER
// TODO SEg fault here
GLuint compileShader(const GLuint program, const GLenum shader_type, const std::string &shader_string);

bool compileProgram(const GLuint program);

#endif
