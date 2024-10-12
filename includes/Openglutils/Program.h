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



//Draw by choosing between an Ebo or a FBO
/*
void draw( const GLuint vao, 
    const GLenum primitives, const unsigned indices, 
    const Mat<>& model, const Mat<>& view, const Mat<>& projection )
{
    assert(vao > 0);
    glBindVertexArray(vao);
    
    // un peu de gymnastique, analyse le mesh pour recuperer les attributs des sommets
    unsigned positions= check_buffer(0);
    unsigned texcoords= check_buffer(1);
    unsigned normals= check_buffer(2);
    
    // si les buffers existent, les attributs des sommets aussi, cf create_buffers()...
    GLuint program= create_default_program(primitives, positions ? 1 : 0, texcoords ? 1 : 0, normals ? 1 : 0);
    glUseProgram(program);
    
    // transformations
    Transform mv= view * model;
    Transform normal= view * model;
    Transform mvp= projection * mv;
    program_uniform(program, "mvpMatrix", mvp);
    program_uniform(program, "mvMatrix", mv);
    program_uniform(program, "normalMatrix", normal);
    
    // mesh indexe, ou pas ?
    GLuint index_buffer= 0;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (GLint *) &index_buffer);
    
    // draw !!
    if(index_buffer)
        glDrawElements( GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays( GL_TRIANGLE_STRIP, 0, count);    
}*/


#endif
