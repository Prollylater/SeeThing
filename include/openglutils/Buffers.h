
#ifndef _BUFFERS_H
#define _BUFFERS_H

#include "glcore.h" //Include correct thing
#include <vector>
#include "Vec.h"



// Structure holding a vao and it's attached vbo and ebo
struct BufferIDsGroups
{
    GLuint m_vao_id, m_vbo_id, m_ebo_id;
    BufferIDsGroups() = default;
    BufferIDsGroups(GLuint vao_id, GLuint vbo_id, GLuint ebo_id) : m_vao_id(vao_id), m_vbo_id(vao_id), m_ebo_id(vao_id){};
    void zeros()
    {

        m_vao_id = 0;
        m_vbo_id = 0;
        m_ebo_id = 0;
    }
};

//TODO REBIND FUNCTION
BufferIDsGroups createBuffers(const std::vector<Vec<float>> &verticestab, const std::vector<GLuint> &indices,
                                const std::vector<Vec<float>> &texcoords)
{
    if (verticestab.size() <= 0)
    {
        // Simply add Exception here
        return BufferIDsGroups();
    }

    GLuint vao, vbo,vbotex, ebo;
    // Generate all vertex
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &vbotex);
    glGenBuffers(1, &ebo);

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
        glBindBuffer(GL_ARRAY_BUFFER, vbotex);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vec<float>), texcoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        glEnableVertexAttribArray(1);
    }
    // TODO Add normal in case going to 3D
    // Create Read to mesh

    if (indices.size())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }
    glBindVertexArray(0);

    return BufferIDsGroups(vao, vbo, ebo);
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

#endif
