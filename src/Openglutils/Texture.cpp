#include "Texture.h"

  //Initialize the texture managment structure
  void initTextRess (TextureResource &texResource, GLuint p_texture_id )
    {
        glGenSamplers(1, &texResource.sampler_id);
        texResource.texture_id = p_texture_id;
        
    }

void bindTexture(const TextureResource &texResource, const GLuint & program, const char *uniform)
{

    glActiveTexture(GL_TEXTURE0 + texResource.unit);
    glBindTexture(GL_TEXTURE_2D, texResource.texture_id);
    glBindSampler(texResource.unit, texResource.sampler_id);
    // Assuming shaderProgram is the active program and uniformName is the texture uniform name
    int uniformLocation = glGetUniformLocation(program, uniform);
    glUniform1i(uniformLocation, texResource.unit);
}

void release( TextureResource &texResource)
{
    glActiveTexture(GL_TEXTURE0 + texResource.unit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(texResource.unit, 0);
}

void freeTexture(TextureResource &texResource)
{
    glDeleteTextures(1, &texResource.texture_id);
    glDeleteSamplers(1, &texResource.sampler_id);
}
