#pragma once

#include "Image.h"

static const int FILE_HEADER_SIZE = 14;
static const int INFO_HEADER_SIZE = 40;

void CreateFileHeader(unsigned char *file_header, const int file_size);

void CreateInfoHeader(unsigned char *info_header, const int file_size, size_t width, size_t height, int x_pixels,
                      int y_pixels);