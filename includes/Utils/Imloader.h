#pragma once
#include <iostream>
#include "Mat.h"


//Charge une image .png, .bmp, jpeg
Mat<uint8_t> loadImg(const char* imagePath, bool flip);


template <typename T>
void writeImgPng(const char* outputPath,  const Mat<T>& image, bool flip ) ;

template <typename T>
void writeImgJpg(const char* outputPath,  const Mat<T>& image, bool flip );


template <typename T>
void writeImgBmp(const char* outputPath,  const Mat<T>& image, bool flip ) ;






