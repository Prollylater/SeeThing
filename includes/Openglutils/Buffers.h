#ifndef _BUFFERS_H
#define _BUFFERS_H
// https://www.khronos.org/opengl/wiki/Common_Mistakes
#include "glcore.h" //Include correct thing
#include <vector>
#include "Vec.h"
#include "Texture.h"


// Structure holding a vao and it's attached vbo and ebo
struct BufferIDsGroups
{
public:
    GLuint m_vao_id, m_vbo_id, m_ebo_id;
    int m_count;
    BufferIDsGroups() = default;
    BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id);
    BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id, int count);
    GLuint getVao();
    int getCount();

friend void freeBuffers(BufferIDsGroups &bufferid);
private:
    void zeros();
};


//Friend function of BufferIDsGroups
void freeBuffers(BufferIDsGroups &bufferid);


// TODO REBIND FUNCTION
BufferIDsGroups createBuffers(const std::vector<Vec<float>> &verticestab, const std::vector<GLuint> &indices = {},
                              const std::vector<Vec<float>> &texcoords = {});

// Create and additional FBO with a binded texture
GLuint createFBO(TextureResource &texress);
void useFbo(GLuint fbo);
void attachFbo(GLuint fbo, TextureResource &texress);
#endif
