#ifndef EDITOR_H
#define EDITOR_H
#include "StructParam.h"
#include "RegionGrowingAlg.h"

#include <vector>
#include <iostream>

#include "glcore.h"
#include "Buffers.h"
#include "Program.h"
#include "Texture.h"

#include "Canvas.h"

// TODO This might be in need of a lot of refactoring

class ImgEditorEngine;
class OpenGLEngine;

namespace appobj
{
    extern OpenGLEngine glengine;
    extern Canvas canvas;
    // Create an engine that withold
}

class OpenGLEngine
{

    // TODO
    // Loading pipeline more linked to file browsing   initTextRess(imageress, read_texture<textuc>(1, datapath, matimg));

private:
    // Store Loaded vao
    BufferIDsGroups loadedvao;
    // Store Loaded texture
    TextureResource imageress;
    // FBO
    GLuint fbo;
    //Texture used to render to fbo without frame 
    TextureResource copytext;
    // Store main Shader
    Program prog;

    // TODO Change active vao or texture
public:
    OpenGLEngine();

    // TODO remove remnant of singleton patter here

    OpenGLEngine(const OpenGLEngine &) = delete;
    OpenGLEngine &operator=(const OpenGLEngine &) = delete;
    void copyTextureToFBO(GLuint& fbo, int width , int height);
    bool initImrender();
    bool outputImg(const char *datapath, GLuint *out_texture, int *out_width, int *out_height);

    void renderTexture();
    void renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color);
};

class ImgEditorEngine
{

    void dispatch(SlicParameter &)
    {
    }
};

#endif