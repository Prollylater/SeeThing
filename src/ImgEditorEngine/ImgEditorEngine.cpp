#include "ImgEditorEngine.h"

// Define instance of the app object
namespace appobj
{
    Canvas canvas;
    // Opengl Engine declared elsewhere
    OpenGLEngine glengine;

}

OpenGLEngine::OpenGLEngine() : activevao(0), activetexture(0)
{
    loadedvao.resize(1);
    imageress.resize(1);
    // Create a buffers for the first and  only vao in currrent version

    /*// Context initialization
    if (!initImrender())
     {
         std::cerr << "Buffers Initialization failed" << std::endl;
     }*/
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

    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glBindVertexArray(loadedvao[activevao].m_vao_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
     GLenum error = glGetError();

        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error durings engline render texture function: " << error << std::endl;
        }
    // Take care of each Ui components
}

// TOODO
// Layer choice
// Handling current vector states
// Lot of thing to modify
// Load an image
bool OpenGLEngine::ouputImg(const char *datapath, GLuint *out_texture, int *out_width, int *out_height)
{
    std::cout << "Entering function output" << std::endl;
    // Will always add at the specified Layer
    // Load image in main canvas, actually, should be Load to main canvas somewhere else -> Dispatch to some disparche engine
    //->Dispatch to some other thing Thus Canvas don't need any
    // TODO Canvas set up Should not be handled here


    // Simplify the call on the two line under
    Mat<uint8_t> tmpmat;
    // Charge image in texture and in the mat object
    std::cout << datapath << std::endl;
    std::cout << datapath << std::endl;
    std::cout << datapath << std::endl;


    initTextRess(imageress[activetexture], read_texture<textuc>(0, datapath,
                                                                tmpmat));
    std::cout << "Texture from image function output at " << std::endl;

    Layer new_img(tmpmat);
    if (appobj::canvas.getLayerNb() > activevao)
    {
        appobj::canvas.addAtLayer(new_img, activevao);
    }
    else
    {
        appobj::canvas.addLayer(new_img);
    }

    std::cout << "Canvas etup " << std::endl;
    
    // TODO Canvas set up Should not be handled here
    prog.reset();
    prog.init("./ressources/shaders/imgoutput.vs", "./ressources/shaders/imgoutput.fs");
    // TODO, only init for the first texture in the vector
    // Implement mangament for multiple layers
    std::cout << "Init Program" << std::endl;

    if (fbo == 0)
    {
        fbo = createFBO(imageress[activetexture]);
    }
    std::cout << "FBO created" << std::endl;

    // TODO, not sure of those assignment
    *out_width = tmpmat.getCols();
    *out_height = tmpmat.getRows();
    *out_texture = imageress[activetexture].texture_id;

    renderTexture();

    return 1;
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
