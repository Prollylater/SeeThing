#pragma once
#include <iostream>
#include "Mat.h"

// Charge une image .png, .bmp, jpeg
Mat<uint8_t> loadImg(const char *imagePath, bool flip = true, int nbchanels = 0);

template <typename T>
void writeImgPng(const char *outputPath, const Mat<T> &image, bool flip = true);
void writeImgPng(const char *outputPath, uint8_t *image,  int width, int height, int channels, bool flip = true);

template <typename T>
void writeImgJpg(const char *outputPath, const Mat<T> &image, bool flip = true);
void writeImgJpg(const char *outputPath, uint8_t *image,  int width, int height, int channels, bool flip = true);

template <typename T>
void writeImgBmp(const char *outputPath, const Mat<T> &image, bool flip = true);
void writeImgBmp(const char *outputPath, uint8_t *image,  int width, int height, int channels, bool flip = true);
