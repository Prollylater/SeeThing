
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


class ImgEditorEngine;
class OpenGLEngine;

class OpenGLEngine
{

    // TODO
    // Loading pipeline more linked to file browsing   initTextRess(imageress, read_texture<textuc>(1, datapath, matimg));

private:
    // Store Loaded vao
    std::vector<BufferIDsGroups> loadedvao;
     int activevao;
    // Store Loaded texture
    std::vector<TextureResource> imageress;
    int activetexture;
    // FBO
    GLuint fbo;
    TextureResource fbotext;
    // Store main Shader
    Program prog;

//TODO Change active vao or texture
public:
    OpenGLEngine();
    bool initImrender();
    void renderTexture();
    void renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color);
    void draw();
};

class ImgEditorEngine
{

    void dispatch(SlicParameter &)
    {
    }
};

#endif