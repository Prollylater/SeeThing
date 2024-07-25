#include "ImgEditorEngine.h"

// Define instance of the app object
namespace appobj
{
    Canvas canvas;
    // Opengl Engine declared elsewhere
    OpenGLEngine glengine;
    // Imgeditor Engine declared elsewhere
    // IFdefine module rg
    RGEngine rgengine;

}

OpenGLEngine::OpenGLEngine() //: activevao(0), activetexture(0)
    {
        // loadedvao.resize(1);
        // imageress.resize(1);
        //  Create a buffers for the first and  only vao in currrent version

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

    std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0};

    loadedvao = createBuffers(canvas, indices, canvastext);
    // TODO Array of shader for each change
    // Init program with the generic loading shader
    prog.init("./ressources/shaders/imgoutput.vs", "./ressources/shaders/imgoutput.fs");

    return true;
}
/*
void OpenGLEngine::renderTexture()
{

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.use();
    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glBindVertexArray(loadedvao[activevao].m_vao_id);
    glDrawElements(GL_TRIANGLES, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engline render texture function: " << error << std::endl;
    }
    // Take care of each Ui components
}
*/
// Opengl rendering of the current base texture to the fbo

bool OpenGLEngine::outputText(Mat<uint8_t> &image, GLuint *out_texture)
{

    *out_texture = makeTextureMat<uint8_t, textuc>(0, image);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engine texture loading function: " << error << std::endl;
    }
    return 1;
}

// TOODO
// Layer choice
// Handling current vector states
// Lot of thing to modify
// Load an image
bool OpenGLEngine::outputImg(const char *datapath, GLuint *out_texture, int *out_width, int *out_height)
{
    // Will always add at the specified Layer
    // Load image in main canvas, actually, should be Load to main canvas somewhere else -> Dispatch to some disparche engine
    //->Dispatch to some other thing Thus Canvas don't need any
    // TODO Canvas set up Should not be handled here

    // Simplify the call on the two line under
    Mat<uint8_t> tmpmat;

    // Load image in two texture and in the mat object
    // Second texture may have no purpose out in the current build
    initTextRess(imageress, read_texture<textuc>(0, datapath, tmpmat));

    initTextRess(copytext, makeTextureMat<uint8_t, textuc>(0, tmpmat));

    *out_width = tmpmat.getCols();
    *out_height = tmpmat.getRows();

    Layer new_img(std::move(tmpmat));

    // Currently only deal with the layer 0
    if (appobj::canvas.getLayerNb() > 0)
    {
        appobj::canvas.addAtLayer(new_img, 0);
    }
    else
    {
        appobj::canvas.addLayer(new_img);
    }


    // TODO Canvas set up Should not be handled here
    // TODO, only init for the first texture in the vector
    // Implement mangament for multiple layers

    if (fbo == 0)
    { // Create a fbo for other renderings
        fbo = createFBO(imageress);
    }
    // glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // copyTextureToFBO(fbo,*out_width,*out_height );
    //  FBO texture
    *out_texture = imageress.texture_id;
    // NO rendering is necessary here

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engine texture loading function: " << error << std::endl;
    }
    return 1;
}

// Render a full texture to the main texture
void OpenGLEngine::copyTextureToFBO(GLuint &p_fbo, int width, int height)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set viewport to correct size

    glViewport(0, 0, width, height);

    prog.reset();
    prog.init("./ressources/shaders/imgoutput.vs", "./ressources/shaders/imgoutput.fs");

    prog.use();
    glBindFramebuffer(GL_FRAMEBUFFER, p_fbo);
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete: " << fboStatus << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindVertexArray(loadedvao.m_vao_id);

    bindTexture(copytext, prog.shader_id, "image_color");
    std::cout << "id" << prog.shader_id << std::endl;
    glDrawElements(GL_TRIANGLES, loadedvao.m_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engine update texture function: " << error << std::endl;
    }
    // Take care of each Ui components
}

/////////////////////////////////////Dispatch

// Rendering directly to FBO with colour change
/*
void OpenGLEngine::renderColchange(const Vec<GLfloat> &new_coord, const GLuint &textsize, const Vec<GLfloat> &color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    // TODO yknow
    prog.reset();
    prog.init("./ressources/shaders/colorchange.vs", "./ressources/shaders/colorchange.fs");
    prog.use();
    bindTexture(imageress, prog.shader_id, "image_color");
    glBindVertexArray(loadedvao.getVao());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glDrawElements(GL_TRIANGLES, loadedvao.m_count, GL_UNSIGNED_INT, 0);
    prog.addUniform2("pixel_coord", new_coord);
    prog.addUniform3("new_color", color);
    prog.addUniform1("texture_size", textsize);

    // Take care of each Ui components
}*/

// TODO Look into the performance and overhead cost here
void OpenGLEngine::renderColChange(std::vector<Vec<GLfloat>> &to_color, const Vec<GLfloat> &color, int width, int height, int pt_size)
{
    glEnable(GL_PROGRAM_POINT_SIZE);

    BufferIDsGroups work_vao = createBuffers(to_color);

    // Set viewport to correct size
    glViewport(0, 0, width, height);

    prog.reset();
    prog.init("./ressources/shaders/colchange.vs", "./ressources/shaders/colchange.fs");

    prog.use();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << " durdfdfdf update texture function: " << error << std::endl;
    }
    // TODP
    //  Correct fbo could be passed by a "dispatcher" handling the current layer

    // glBindFramebuffer(GL_FRAMEBUFFER, p_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete: " << fboStatus << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindVertexArray(work_vao.m_vao_id);
    prog.addUniform3("new_color", color);
    prog.addUniform1("text_height", height);
    prog.addUniform1("text_width", width);
    prog.addUniform1("pt_size", pt_size);

    // Directly use the size of the to color arrray sinnce no ebo were crearted for this
    // glDrawElements(GL_POINTS, to_color.size(), GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_POINTS, 0, to_color.size()); // Draw 1 point

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << " durings rendercolor  update texture function: " << error << std::endl;
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_PROGRAM_POINT_SIZE);

    freeBuffers(work_vao);
    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error during color update  function: " << error << std::endl;
    }
    // Take care of each Ui components
}

// TODO delete thsi

// OLD TEst
/*   GLint format;
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

switch (format) {
     case GL_RGB:
     case GL_RGB8:
         std::cout << "Texture format is RGB." << std::endl;
         break;
     case GL_RGBA:
     case GL_RGBA8:
         std::cout << "Texture format is RGBA." << std::endl;
         break;
     case GL_RGB16F:
         std::cout << "Texture format is RGB16F (16-bit floating point RGB)." << std::endl;
         break;
     case GL_RGBA16F:
         std::cout << "Texture format is RGBA16F (16-bit floating point RGBA)." << std::endl;
         break;
     case GL_RGB32F:
         std::cout << "Texture format is RGB32F (32-bit floating point RGB)." << std::endl;
         break;
     case GL_RGBA32F:
         std::cout << "Texture format is RGBA32F (32-bit floating point RGBA)." << std::endl;
         break;
     // Add more formats as needed
     default:
         std::cout << "Unknown or unsupported texture format." << std::endl;
         break;
 }*/

/*
// Assuming 3 bytes per pixel for GL_RGB
int size = width * height * 3;
std::cout << width << "Image in out " << height << std::endl;
uint8_t* pixels = new uint8_t[size];

glGetTexImage(GL_TEXTURE_2D,0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
uint8_t* pixelsv = new uint8_t[size];
std::memcpy(pixelsv,pixels, size);


std::cout << width << "Image iddn out " << height << std::endl;

Mat<uint8_t>outtexsez(pixels, height, width, 3);
writeImgJpg("./trucssd.jpg",outtexsez ,true);

if (width > 0 && height > 0)
{
    std::cout << "Texture is valid with width: " << width << " and height: " << height << std::endl;
}
else
{
    std::cout << "Texture is not valid or not properly set with width: " << width << " and height: " << height << std::endl;
}
if (glIsTexture(imageress.texture_id))
{
    std::cout << "Texture ID is valid." << std::endl;
}
else
{
    std::cout << "Texture ID is not valid." << std::endl;
}
  GLint width, height;
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);



*/