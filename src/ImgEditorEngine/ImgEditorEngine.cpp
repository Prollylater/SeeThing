#include "ImgEditorEngine.h"
#define GET_NUM_CHANNELS(internalFormat)                             \
    (internalFormat) == GL_RED ? 1 : (internalFormat) == GL_RG   ? 2 \
                                 : ((internalFormat) == GL_RGB)  ? 3 \
                                 : ((internalFormat) == GL_RGBA) ? 4 \
                                                                 : 0

// Define instance of the app object which each correspond to a feature
namespace appobj
{
    //Some take their data directly from this object
    //This object is not updated
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

// TODO better management of canvas and current state to avoid this treatment
// Save an image
// Invoked when the image wasn't already stored in a canvas
// Result of operation etc..
bool OpenGLEngine::saveTextInst(GLuint &out_texture, const char *filename)
{
    GLint width, height, format;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, out_texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    int channels = GET_NUM_CHANNELS(format);
    // TODO Channles data not stored
    int size = width * height * (channels);
    if (channels == 3)
    {   
        //Use Pack Alignment to mirror the UNPACK_ALIGNMENT at write
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
    }

    std::cout << "Size related to file writing (w, h, chan): " << std::endl;
    std::cout << width <<" "<< height << channels << std::endl;
    uint8_t *pixels = new uint8_t[size];

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error during saving  imagefrom texture " << error << std::endl;
        return 0;
    }
    // TODO GIve choice between mutiple format
    writeImgJpg(filename, pixels,  width, height, channels,true);
}


/*
bool OpenGLEngine::saveCurrentDisplay(GLuint &out_texture, const char *filename)
{
    GLuint textureObj = ...; // the texture object - glGenTextures

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureObj, 0);

    int data_size = mWidth * mHeight * 4;
    GLubyte* pixels = new GLubyte[mWidth * mHeight * 4];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error during saving from fbo: " << error << std::endl;
        }
        return 1;}
    */

// Create a texture using the passed image data
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

// TOODO change names of the "output" functions
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

/*
void OpenGLEngine::renderTexture()
{

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.use();
    bindTexture(imageress[activetexture], prog.shader_id, "image_color");
    glBindVertexArray(loadedvao[activevao].m_vao_id);
    glDrawElements( GL_TRIANGLE_STRIP, loadedvao[activevao].m_count, GL_UNSIGNED_INT, 0);
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engline render texture function: " << error << std::endl;
    }
    // Take care of each Ui components
}
*/

bool OpenGLEngine::deleteTexture(GLuint &tex)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    freeTexture(tex);
}

// TODO, the change here is not transfered to the canvas
//  Render a full texture to the main texture
void OpenGLEngine::copyTextureToFBO(GLuint &tex, int width, int height)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set viewport to correct size

    glViewport(0, 0, width, height);

    prog.reset();
    prog.init("./ressources/shaders/imgoutput.vs", "./ressources/shaders/imgoutput.fs");

    prog.use();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete: " << fboStatus << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindVertexArray(loadedvao.getVao());

    // Create a temporary Ressource holder
    TextureResource temp_tex_res;
    initTextRess(temp_tex_res, tex);

    bindTexture(temp_tex_res, prog.shader_id, "image_color");
    std::cout << "id" << prog.shader_id << std::endl;
    glDrawElements(GL_TRIANGLE_STRIP, loadedvao.m_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings engine update texture function: " << error << std::endl;
    }
}

/////////////////////////////////////Dispatch

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
    glBindVertexArray(work_vao.getVao());
    prog.addUniform3("new_color", color);
    prog.addUniform1("text_height", height);
    prog.addUniform1("text_width", width);
    prog.addUniform1("pt_size", pt_size);

    // Directly use the size of the to color arrray sinnce no ebo were crearted for this
    // glDrawElements(GL_POINTS, to_color.size(), GL_UNSIGNED_INT, 0);
    //TODO create draw function that cna make this choice
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