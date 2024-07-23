

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"



#include "Imloader.h"

//Charge une image .png, .bmp, jpeg
Mat<uint8_t> loadImg(const char* imagePath, bool flip , int nbchan ) {
    // Load image using STB Image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(flip);
    uint8_t* image = stbi_load(imagePath, &width, &height, &channels, nbchan);
    if (!image) {
        std::cerr << "Error in opening the image " << imagePath <<std::endl;
    }
        size_t size = sizeof(image) / sizeof(image[0]);  
    std::vector<unsigned char> vecImg(image, image+ size );

    int final_chan = nbchan ? (nbchan !=0) : channels;
    Mat<unsigned char> imgMat(image,height, width, final_chan) ;
    //Considered channel is the desired number, not the actual number
    //Mat<unsigned char> imgMat(image,height, width, nbchan) ;

    stbi_image_free(image);
    return imgMat;
}



template <typename T>
void writeImgPng(const char* outputPath,  const Mat<T>& image, bool flip ) {
    // Save image using STB Image Write
    stbi_flip_vertically_on_write(flip);
    if(image.getRows()*image.getCols() == 0){
        std::cerr << "Error, no data to write " <<std::endl;
        return ;
    }
    int width, height,channels;
    width = image.getCols();
    height = image.getRows();
    channels = image.getChannels();
    int success = stbi_write_png(outputPath, width, height, channels, image.getData(), width * channels);

    if (success) {
        std::cout << "Image saved successfully to " << outputPath << std::endl;
    } else {
        std::cerr << "Error, could not save the image. Check the file path and permissions." << std::endl;
    }
}

template void writeImgPng<uint8_t>(const char* outputPath,  const Mat<uint8_t>& image, bool flip );
template void writeImgPng<float>(const char* outputPath,  const Mat<float>& image, bool flip );


template <typename T>
void writeImgJpg(const char* outputPath,  const Mat<T>& image, bool flip ) {
    // Save image using STB Image Write
    stbi_flip_vertically_on_write(flip);
    if(image.getRows()*image.getCols() == 0){
        std::cerr << "Error, no data to write " <<std::endl;
        return ;
    }

    int width, height,channels;
    width = image.getCols();
    height = image.getRows();
    channels = image.getChannels();
    int success = stbi_write_jpg(outputPath, width, height, channels, image.getData(), 90);

    if (success) {
        std::cout << "Image saved successfully to " << outputPath << std::endl;
    } else {
        std::cerr << "Error, could not save the image. Check the file path and permissions." << std::endl;
    }
}

template void writeImgJpg<uint8_t>(const char* outputPath,  const Mat<uint8_t>& image, bool flip );
template void writeImgJpg<float>(const char* outputPath,  const Mat<float>& image, bool flip );

template <typename T>
void writeImgBmp(const char* outputPath,  const Mat<T>& image, bool flip ) {
    // Save image using STB Image Write
    stbi_flip_vertically_on_write(flip);
    if(image.getRows()*image.getCols() == 0){
        std::cerr << "Error, no data to write " <<std::endl;
        return ;
    }
    const T* data = image.getData();

    int width, height,channels;
    width = image.getCols();
    height = image.getRows();
    channels = image.getChannels();
    int success = stbi_write_bmp(outputPath, width, height, channels, data);

    if (success) {
        std::cout << "Image saved successfully to " << outputPath << std::endl;
    } else {
        std::cerr << "Error, could not save the image. Check the file path and permissions." << std::endl;
    }
}

template void writeImgBmp<uint8_t>(const char* outputPath,  const Mat<uint8_t>& image, bool flip );
template void writeImgBmp<float>(const char* outputPath,  const Mat<float>& image, bool flip );