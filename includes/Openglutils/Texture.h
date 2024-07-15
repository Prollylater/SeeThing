#ifndef _TEXTURE_H
#define _TEXTURE_H

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
void initTextRess(TextureResource &texResource, GLuint p_texture_id);

void bindTexture(const TextureResource &texResource, const GLuint &program, const char *uniform);

void release( TextureResource &texResource);

void freeTexture(TextureResource &texResource);

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
    static constexpr int data_type = GL_UNSIGNED_BYTE;
};

template <>
struct TextTraits<TextType::MT_UINT>
{
    typedef unsigned int Type;
    static constexpr int data_type = GL_UNSIGNED_INT;
};

template <>
struct TextTraits<TextType::MT_FLOAT>
{
    typedef float Type;
    static constexpr int data_type = GL_FLOAT;
};

using textuc = TextTraits<TextType::MT_UC>;
using textfloat = TextTraits<TextType::MT_UINT>;
using textuint = TextTraits<TextType::MT_FLOAT>;

template <typename T, typename U>
GLuint makeTextureMat(const int unit,
                      const Mat<T> &im,
                      const GLenum texel_type)
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
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        break;
    }
    int data_type = U::data_type;

    GLuint texture;
    glGenTextures(1, &texture);
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "Failed to generate texture." << std::endl;
        return 0;
    }

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, texel_type, im.getCols(), im.getRows(), 0,
                 format, data_type, im.getData());
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "Failed to create texture image." << std::endl;
        return 0;
    }

    // prefiltre la texture
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

template <typename T>
GLuint read_texture(const int unit,
                    const char *filename, Mat<uint8_t> &img,
                    const GLenum texel_type = GL_RGB)
{

    img = loadImg(filename, true);

    GLuint texture = makeTextureMat<uint8_t, T>(unit, img, texel_type);

    return texture;
}

#endif
