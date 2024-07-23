#ifndef _BUFFERS_H
#define _BUFFERS_H
//https://www.khronos.org/opengl/wiki/Common_Mistakes
#include "glcore.h" //Include correct thing
#include <vector>
#include "Vec.h"
#include "Texture.h"

// Structure holding a vao and it's attached vbo and ebo
struct BufferIDsGroups
{
    GLuint m_vao_id, m_vbo_id, m_ebo_id;
    int m_count;
    BufferIDsGroups() = default;
    BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id);
    BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id, int count);
    void zeros();
    GLuint getVao();
};

// TODO REBIND FUNCTION
BufferIDsGroups createBuffers(const std::vector<Vec<float>> &verticestab, const std::vector<GLuint> &indices,
                              const std::vector<Vec<float>> &texcoords);

GLuint createFBO(TextureResource &texress);
void useFbo(GLuint fbo);
void attachFbo(GLuint fbo, TextureResource &texress);

void freeBuffers(BufferIDsGroups &bufferid);
#endif
