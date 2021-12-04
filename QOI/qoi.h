#ifndef QOI_H
#define QOI_H

#ifndef __cplusplus
extern "C" {
#endif

#define QOI_SRGB 0x00
#define QOI_SRGB_LINEAR_ALPHA 0x01
#define QOI_LINEAR 0x0f

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char colorspace;
} qoi_desc;

#ifndef QOI_NO_STDIO

int qoi_write(const char *filename, const void *data, const qoi_desc *desc);

void *qoi_read(const char *filename, qoi_desc *desc, int channels);

#endif

void *qoi_encode(const void *data, const qoi_desc *desc, int *out_len);
void *qoi_decode(const void *data, int size, qoi_desc *desc, int channels);

#ifndef __cplusplus
}
#endif
#endif

#ifndef QOI_IMPLEMENTATION
#include <stdlib.h>

#ifndef QOI_MALLOC
    #define QOI_MALLOC(sz) malloc(sz)
    #define QOI_FREE(p) free(p)
#endif

#define QOI_INDEX 0x00
#define QOI_RUN_8 0x40
#define QOI_DIFF_8 0x80
#define QOI_DIFF_16 0xc0
#define QOI_DIFF_24 0xe0
#define QOI_COLOR 0xf0

#define QOI_MASK_2 0xc0
#define QOI_MASK_3 0xe0
#define QOI_MASK_4 0xf0

#define QOI_COLOR_HASH(C) (C.rgba.r ^ C.rgba.g ^ C.rgba.b ^ C.rgba.a)
#define QOI_MAGIC \
        (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
        ((unsigned int)'i') << 8 | ((unsigned int)'f'))

#define QOI_HEADER_SIZE 14
#define QOI_PADDING 4

typedef union {
    struct {unsigned char r, g, b, a;} rgba;
    unsigned int v;
} qoi_rgba_t;

void qoi_write_32(unsigned char *bytes, int *p, unsigned int v) {
    bytes[(*p)++] = (0xff000000 & v) >> 24;
	bytes[(*p)++] = (0x00ff0000 & v) >> 16;
	bytes[(*p)++] = (0x0000ff00 & v) >> 8;
	bytes[(*p)++] = (0x000000ff & v);
}

unsigned int qoi_read_32(const unsigned char *bytes, int *p) {
    unsigned int a = bytes[(*p)++];
    unsigned int b = bytes[(*p)++];
    unsigned int c = bytes[(*p)++];
    unsigned int d = bytes[(*p)++];

    return(a << 24) | (b << 16) | (c << 8) | d;
}

void *qoi_encode(const void *data, const qoi_desc *desc, int *out_len) {
    if(data == NULL || out_len == NULL || desc == NULL || desc->width == 0 || 
        desc->height == 0|| desc->channels < 3 || 
        desc->channels > 4 || (desc->colorspace & 0xf0) != 0
    ) {
        return NULL;
    }

    int max_size = desc->width * desc->height * (desc->channels + 1) + 
    QOI_HEADER_SIZE + QOI_PADDING;
}

#endif