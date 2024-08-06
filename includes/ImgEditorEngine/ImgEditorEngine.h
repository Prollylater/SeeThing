#ifndef EDITOR_H
#define EDITOR_H
#include "StructParam.h"

// Engines
#include "RegionGrowingEngine.h"

#include <vector>
#include <iostream>

#include "glcore.h"
#include "Buffers.h"
#include "Program.h"
#include "Texture.h"

#include "Canvas.h"

// TODO This might be in need of a lot of refactoring

class OpenGLEngine;

namespace appobj
{
    extern OpenGLEngine glengine;
    extern Canvas canvas;
    // Create an engine that withold
}

// TODO Change directory ?
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
    // Texture used to render to fbo without frame
    // TextureResource copytext;
    // Store main Shader
    Program prog;

    // TODO Change active vao or texture
public:
    OpenGLEngine();

    // TODO remove remnant of singleton patter here

    OpenGLEngine(const OpenGLEngine &) = delete;
    OpenGLEngine &operator=(const OpenGLEngine &) = delete;
    void copyTextureToFBO(GLuint &fbo, int width, int height);
    bool initImrender();
    bool outputImg(const char *datapath, GLuint *out_texture, int *out_width, int *out_height);
    bool outputText(Mat<uint8_t> &image, GLuint *out_texture);
    bool deleteTexture(GLuint &tex);
    bool saveTextInst(GLuint &out_texture, const char *filename);

    void renderTexture();

    void renderColChange(std::vector<Vec<GLfloat>> &to_color, const Vec<GLfloat> &color, int width, int height, int pt_size = 5);
};

#endif