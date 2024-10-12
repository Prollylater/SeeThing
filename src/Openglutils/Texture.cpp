#include "Texture.h"

//TODO Better setting unitg
  //Initialize the texture managment structure
  void initTextRess (TextureResource &tex_resource, GLuint p_texture_id, int p_unit )
    {
        glGenSamplers(1, &tex_resource.sampler_id);
        tex_resource.texture_id = p_texture_id;
        tex_resource.unit = p_unit;
        
    }


//TODO Let user set unit here rather than prior
void bindTexture(const TextureResource &tex_resource, const GLuint & program, const char *uniform_name)
{
    glActiveTexture(GL_TEXTURE0 + tex_resource.unit);
    glBindTexture(GL_TEXTURE_2D, tex_resource.texture_id);
    glBindSampler(tex_resource.unit, tex_resource.sampler_id);
    // Assuming shader_program is the active program and uniform_name is the texture uniform name
    int unifrm_loc = glGetUniformLocation(program, uniform_name);
    glUniform1i(unifrm_loc, tex_resource.unit);
}

void release( TextureResource &tex_resource)
{
    glActiveTexture(GL_TEXTURE0 + tex_resource.unit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(tex_resource.unit, 0);
}

void freeTexture(TextureResource &tex_resource)
{
    glDeleteTextures(1, &tex_resource.texture_id);
    glDeleteSamplers(1, &tex_resource.sampler_id);
}

void freeTexture(GLuint &tex_resource)
{
    glDeleteTextures(1, &tex_resource);
}


