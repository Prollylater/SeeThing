#include "ImgEditorEngine.h"

OpenGLEngine::OpenGLEngine() : activevao(0), activetexture(0)
{
    loadedvao.resize(1);
    imageress.resize(1);
    // Create a buffers for the first and  only vao in currrent version

     // Context initialization
    if (!initImrender())
    {
        std::cerr<<"Buffers Initialization failed"<<std::endl;
    }

};


bool OpenGLEngine::initImrender()
{
     // Initialize the Vao allowing to render the image as a texture
    // This vao is always a plane taking a Mat<uint8> data as texture.
    // Create an unique fbo and it's designated rendering texture
    // fbo = createFBO(fbotext);
    // prog.init("shader.vs", "shader.fs");
    // prog.init("colorchange.vs", "colorchange.fs");
    std::vector<Vec<float>> canvas = {
        Vec(-1.0f, -1.0f, 0.0f),
        Vec(-1.0f, 1.0f, 0.0f),
        Vec(1.0f, 1.0f, 0.0f),
        Vec(1.0f, -1.0f, 0.0f)};

    std::vector<Vec<float>> canvastext = {
        Vec(0.0f, 0.0f, 0.0f),
        Vec(0.0f, 1.0f, 0.0f),
        Vec(1.0f, 1.0f, 0.0f),
        Vec(1.0f, 0.0f, 0.0f)

    };

    std::vector<unsigned> indices = {0, 1, 2,
                                     2, 3, 0};

    loadedvao[activevao] = createBuffers(canvas, indices, canvastext);
   
    return true;
}


// Actual opengl rendering in viewport with new Texture , maybe useless
void OpenGLEngine::renderTexture()
{
    glClear(GL_COLOR_BUFFER_BIT);
    prog.use();
    glBindVertexArray(loadedvao[activevao].m_vao_id);
    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
    // Take care of each Ui components
}

// Rendering directly to FBO with colour change
void OpenGLEngine::renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    // TODO yknow
    prog.reset();
    prog.init("./ressources/shaders/colorchange.vs", "./ressources/shaders/colorchange.fs");
    prog.use();
    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glBindVertexArray(loadedvao[activevao].getVao());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
    prog.addUniform2("pixel_coord", new_coord);
    prog.addUniform3("new_color", color);
    prog.addUniform1("texture_size", textsize);

    // Take care of each Ui components
}

// TODO delete thsi
/*
void OpenGLEngine::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // TODO yknow
    prog.reset();
    prog.init("colorchange.vs", "colorchange.fs");
    prog.use();
    glBindVertexArray(loadedvao[activevao].getVao());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
}


*/
