
#ifndef GLCTXT_H
#define GLCTXT_H
#include <vector>
#include <iostream>

#include "glcore.h"
#include "Buffers.h"
#include "Program.h"
#include "Texture.h"

class OpenglContext
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

public:
    OpenglContext();

    bool initImrender();

    void processInput(GLFWwindow * window);

    void preRender();

    void postRender(GLFWwindow *window);

    void renderTexture();

    void renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color);

    void draw();

    bool initGlContext(GLFWwindow*& window, int width, int height);

    void quit(GLFWwindow *window);
};

#endif
