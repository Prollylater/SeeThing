#include "Buffers.h"

// Structure holding a vao and it's attached vbo and ebo

BufferIDsGroups::BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id) : m_vao_id(vao_id), m_vbo_id(vao_id), m_ebo_id(vao_id){};
BufferIDsGroups::BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id, int count) : m_vao_id(vao_id), m_vbo_id(vao_id), m_ebo_id(vao_id), m_count(count){};
void BufferIDsGroups::zeros()
{

    m_vao_id = 0;
    m_vbo_id = 0;
    m_ebo_id = 0;
}

GLuint BufferIDsGroups::getVao()
{
    return m_vao_id;
}

// TODO REBIND FUNCTION
BufferIDsGroups createBuffers(const std::vector<Vec<float>> &verticestab, const std::vector<GLuint> &indices,
                              const std::vector<Vec<float>> &texcoords)
{
    if (verticestab.size() <= 0)
    {
        // Simply add Exception here
        return BufferIDsGroups();
    }

    GLuint vao, vbo, vbotex, ebo;
    // Generate all vertex
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // Bind vao in which everything will be stored
    glBindVertexArray(vao);

    // Bind Vertex Buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticestab.size() * sizeof(Vec<float>), verticestab.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    glEnableVertexAttribArray(0);

    // Bind Texturecoordinate
    if (texcoords.size() && texcoords.size() == verticestab.size())
    {
        glGenBuffers(1, &vbotex);
        glBindBuffer(GL_ARRAY_BUFFER, vbotex);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vec<float>), texcoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        glEnableVertexAttribArray(1);
    }
    // TODO Add normal in case going to 3D

    // Create Read to mesh

    if (indices.size())
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }
    glBindVertexArray(0);
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error during vertex buffer initialization: " << error << std::endl;
        return BufferIDsGroups(0, 0, 0, 0);
    }

    return BufferIDsGroups(vao, vbo, ebo, indices.size());
}

GLuint createFBO(TextureResource &texress)
{

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error after FBO Generations: " << error << std::endl;
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error after glBindFramebuffer: " << error << std::endl;
        return 0;
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texress.texture_id, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            std::cerr << "Framebuffer undefined" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "Framebuffer incomplete attachment" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "Framebuffer incomplete  missing attachment" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "Framebuffer incomplete draw buffer" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "Framebuffer incomplete read buffer" << std::endl;
            break;
        default:
            std::cerr << "Unknown framebuffer error. Status code: " << status << std::endl;
            break;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

void useFbo(GLuint fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

// TODO Useful ?
void attachFbo(GLuint fbo, TextureResource &texress)
{

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texress.texture_id, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            std::cerr << "Framebuffer undefined" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "Framebuffer incomplete attachment" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "Framebuffer incomplete  missing attachment" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "Framebuffer incomplete draw buffer" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "Framebuffer incomplete read buffer" << std::endl;
            break;
        default:
            std::cerr << "Unknown framebuffer error. Status code: " << status << std::endl;
            break;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void freeBuffers(BufferIDsGroups &bufferid)
{
    // Not default Buffer
    GLuint &vao = bufferid.m_vao_id;
    GLuint &vbo = bufferid.m_vbo_id;
    GLuint &ebo = bufferid.m_ebo_id;

    if (!(vao <= 0))
    {
        // Simply add Exception here
        bufferid.zeros();
        return;
    }

    // Step 1: Unbind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);         // Unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

    // Step 2: Delete the buffers
    glDeleteBuffers(1, &vbo); // Delete VBO
    glDeleteBuffers(1, &ebo); // Delete EBO

    // Finally we van delete the Vao
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);

    bufferid.zeros();

    return;
}
