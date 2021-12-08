#include <stdio.h>
#include <dirent.h>
#include <png.h>

#define STB_IMAGE_IMPLEMENTATION
#define SRBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMETATION
#include "stb_image_write.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"