#ifndef _TEXTURE_H
#define _TEXTURE_H
// https://www.khronos.org/opengl/wiki/Common_Mistakes
#include "glcore.h"
#include <cstdio>
#include <algorithm>
#include "Imloader.h"
// TODO Separate into cpp and h file
struct TextureResource
{
    GLuint texture_id;
    GLuint sampler_id;
    int unit;
    TextureResource() = default;
};
// Initialize the texture managment structure
void initTextRess(TextureResource &texResource, GLuint p_texture_id, int p_unit = 0);

void bindTexture(const TextureResource &texResource, const GLuint &program, const char *uniform);

void release(TextureResource &texResource);

void freeTexture(TextureResource &texResource);
void freeTexture(GLuint &texResource);

enum class TextType
{
    MT_UC,
    MT_UINT,
    MT_INT,
    MT_FLOAT,
};

template <TextType T>
struct TextTraits;

// Specializations for each PxlTextType
template <>
struct TextTraits<TextType::MT_UC>
{
    typedef unsigned char Type;
    static constexpr GLenum data_type = GL_UNSIGNED_BYTE;
};

template <>
struct TextTraits<TextType::MT_UINT>
{
    typedef unsigned int Type;
    static constexpr GLenum data_type = GL_UNSIGNED_INT;
};

template <>
struct TextTraits<TextType::MT_FLOAT>
{
    typedef float Type;
    static constexpr GLenum data_type = GL_FLOAT;
};

//Make texture funciton should ony be templated with the following
using textuc = TextTraits<TextType::MT_UC>;
using textfloat = TextTraits<TextType::MT_UINT>;
using textuint = TextTraits<TextType::MT_FLOAT>;

template <typename T, typename U>
GLuint makeTextureMat(const int unit,
                      const Mat<T> &im)
{
    if (im.getCols() == 0 && im.getRows() == 0)
    {
        std::cerr << " Image has no columns and rows (both are zero)." << std::endl;
        return 0;
    }
    int channels = im.getChannels();
    if (channels < 1 || channels > 4)
    {
        std::cerr << "Invalid number of channels. Must be between 1 and 4." << std::endl;
        return 0;
    }
    GLenum format = GL_RGB;
    GLenum texel_type = GL_RGB;

    switch (channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        if (im.getCols() % 3 != 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        break;
    case 4:
        format = GL_RGBA;
        texel_type = GL_RGBA ;
        break;
    default:
        break;
    }
    GLenum data_type = U::data_type;

    GLuint texture;
    glGenTextures(1, &texture);
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "Failed to generate texture." << std::endl;
        return 0;
    }
    // TODO NO use here remove

    glActiveTexture(GL_TEXTURE0 + unit);

    glBindTexture(GL_TEXTURE_2D, texture);

   
    glTexImage2D(GL_TEXTURE_2D, 0, texel_type, im.getCols(), im.getRows(), 0,
                 format, data_type, im.getData());
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "Failed to create texture image." << std::endl;
        return 0;
    }

    // prefiltre la texture
    //glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // BY default this use a MIPMAP, hence yhou need to define the two parameter LEVEL
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //LImit the number of mipmap
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error after making texture: " << error << std::endl;
        return 0;
    }

    return texture;
}

// Read Texture froma a file with load img
// Texture is by default 3 channels RGB as this was the intended format at the start
// RGBA is still usable in most situtaiton and preeferred whilde dealing with texture
// TODO track function that can't handle RGBA
//Can't handle both RGBA Picture and RGB at the same time
template <typename T>
GLuint read_texture(const int unit,
                    const char *filename, Mat<uint8_t> &img, int nbchannels = 0)
{

    img = loadImg(filename, true, nbchannels);

    GLuint texture = makeTextureMat<uint8_t, T>(unit, img);

    return texture;
}

#endif
