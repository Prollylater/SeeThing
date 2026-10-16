#include "ImageIO.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace core
{
    namespace
    {
        std::string lowerExtension(const std::string &path)
        {
            std::size_t dot = path.find_last_of('.');
            std::size_t slash = path.find_last_of("/\\");
            if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
                return {};
            std::string ext = path.substr(dot + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return std::tolower(ch); });
            return ext;
        }

        bool fail(std::string *error, std::string message)
        {
            if (error)
                *error = std::move(message);
            return false;
        }
    }

    LoadResult loadImage(const std::string &path, int channels)
    {
        LoadResult result;
        if (channels < 0 || channels > 4)
        {
            result.error = "loadImage: channels must be 0 to 4";
            return result;
        }
        int w = 0, h = 0, fileChannels = 0;
        stbi_set_flip_vertically_on_load(0);
        stbi_uc *px = stbi_load(path.c_str(), &w, &h, &fileChannels, channels);
        if (!px)
        {
            result.error = "Cannot load " + path + ": " + stbi_failure_reason();
            return result;
        }
        const int c = channels ? channels : fileChannels;
        Image<std::uint8_t> image(w, h, c);
        std::memcpy(image.data(), px, image.size());
        stbi_image_free(px);

        result.image = std::move(image);
        result.fileChannels = fileChannels;
        return result;
    }

    bool saveImage(const std::string &path, ImageView<const std::uint8_t> image, std::string *error, SaveOptions options)
    {
        if (image.empty())
            return fail(error, "saveImage: empty image");
        if (image.channels() < 1 || image.channels() > 4)
            return fail(error, "saveImage: expected 1 to 4 channels");

        const int w = image.width(), h = image.height(), c = image.channels();
        const std::string ext = lowerExtension(path);

        // Only the PNG writer takes a row stride; the others need packed rows
        Image<std::uint8_t> packed;
        const std::uint8_t *px = image.row(0);
        if (ext != "png" && image.stride() != std::ptrdiff_t(w) * c)
        {
            packed = Image<std::uint8_t>(w, h, c);
            for (int y = 0; y < h; ++y)
                std::copy(image.row(y), image.row(y) + std::size_t(w) * c, packed.row(y));
            px = packed.data();
        }

        int ok = 0;
        if (ext == "png")
            ok = stbi_write_png(path.c_str(), w, h, c, px, int(image.stride()));
        else if (ext == "jpg" || ext == "jpeg")
            ok = stbi_write_jpg(path.c_str(), w, h, c, px, std::clamp(options.jpgQuality, 1, 100));
        else if (ext == "bmp")
            ok = stbi_write_bmp(path.c_str(), w, h, c, px);
        else if (ext == "tga")
            ok = stbi_write_tga(path.c_str(), w, h, c, px);
        else
            return fail(error, "saveImage: unknown extension in " + path + " (use .png, .jpg, .bmp or .tga)");

        if (!ok)
            return fail(error, "Cannot write " + path);
        return true;
    }
}
