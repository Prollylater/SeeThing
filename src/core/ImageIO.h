#ifndef CORE_IMAGEIO_H
#define CORE_IMAGEIO_H

#include "Image.h"

#include <cstdint>
#include <string>

namespace core
{
    struct LoadResult
    {
        Image<std::uint8_t> image; // empty on failure
        int fileChannels = 0;      // what the file holds, before forcing
        std::string error;         // empty on success

        bool ok() const { return error.empty(); }
    };

    // Rows are top-down as in the file. channels = 0 keeps the file's count; 4 is the layer format (RGBA8).
    LoadResult loadImage(const std::string &path, int channels = 4);

    struct SaveOptions
    {
        int jpgQuality = 95; // 1..100
    };

    // The extension picks the format: .png, .jpg/.jpeg, .bmp, .tga. Returns false and sets error on failure.
    bool saveImage(const std::string &path, ImageView<const std::uint8_t> image, std::string *error = nullptr,
                   SaveOptions options = {});
}

#endif
