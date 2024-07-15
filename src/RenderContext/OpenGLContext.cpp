
#include "OpenGLContext.h"

OpenglContext::OpenglContext() : activevao(0), activetexture(0)
{
    loadedvao.resize(1);
    imageress.resize(1);
};

// Initialize the Vao allowing to render the image as a texture
// This vao is always a plane taking a Mat<uint8> data as texture.
bool OpenglContext::initImrender()
{
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

    // Create a buffers for the first and  only vao in currrent version
    loadedvao[activevao] = createBuffers(canvas, indices, canvastext);

    return true;
}

void OpenglContext::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void OpenglContext::preRender()
{
    glfwPollEvents();

 
}

void OpenglContext::postRender(GLFWwindow *window)
{

    glfwSwapBuffers(window);

    processInput(window);
       glClearColor(0.2f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
}

// Actual opengl rendering in viewport with new Texture , maybe useless
void OpenglContext::renderTexture()
{
    glClear(GL_COLOR_BUFFER_BIT);
    prog.use();
    glBindVertexArray(loadedvao[activevao].m_vao_id);
    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
    // Take care of each Ui components
}

// Rendering directly to FBO with colour change
void OpenglContext::renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color)
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
void OpenglContext::draw()
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
}*/

// Initialize GLEW, GL and GLFW
// Must be the first thing to launch to allow function to be found
bool OpenglContext::initGlContext(GLFWwindow*& window, int width, int height)
{
    // Context initialization
    // GLFW Init

    glfwSetErrorCallback([](int error, const char *description)
                         { std::cerr << "GLFW Error (" << error << "): " << description << std::endl; });

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 0;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "ImgEditor", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set the callback function for window resize

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
                                   { glViewport(0, 0, width, height); });

    //(VSync)
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1);

    // Context initialization
    if (!initImrender())
    {
        return 0;
    }

    return 1;
}

// TODO
// TODO
// TODO
// TODO

void OpenglContext::quit(GLFWwindow *window)
{ // List of each
  // Loop through this
    /*freeBuffers(model1);
    freeTexture(imageress);*/
    glfwDestroyWindow(window);
    glfwTerminate();
}
