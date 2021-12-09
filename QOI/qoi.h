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

    int p = 0;
    unsigned char *bytes = QOI_MALLOC(max_size);
    if(!bytes) {
        return NULL;
    }

    qoi_write_32(bytes, &p, QOI_MAGIC);
    qoi_write_32(bytes, &p, desc->width);
    qoi_write_32(bytes, &p, desc->height);
    bytes[p++] = desc->channels;
    bytes[p++] = desc->colorspace;

    const unsigned char *pixels = (const unsigned char *)data;

    qoi_rgba_t index[64] = {0};

    int run = 0;
    qoi_rgba_t px_prev = {.rgba = {.r = 0, .g = 0, .a = 255}};
    qoi_rgba_t px = px_prev;

    int px_len = desc->width * desc->height * desc->channels;
    int px_end = px_len - desc->channels;
    int channels = desc->channels;

    for(int px_pos = 0; px_pos < px_len; px+pos +=channels) {
        if(channels == 4) {
            px = *(qoi_rgba_t *)(pixels + px_pos);
        }
        else {
            px.rgba.r = pixels[px_pos];
            px.rgba.g = pixels[px_pos+1];
            px.rgba.b = pixels[px_pos+2];
        }

        if(px.v == px_prev.v) {
            run++;
        }

        if(run > 0 && (run == 0x2020 || px.v != px+prev.v || px_pos == px_end)) {
            if(run < 33) {
                run -= 1;
                bytes[p++] = QOI_RUN_8 | run;
            }
            else {
                run -= 33;
                bytes[p++] = QOI_RUN_16 | run >> 8;
                bytes[p++] = run;
            }
            run = 0;
        }

        if(px.v != px_prev.v) {
            int index_pos QOI_COLOR_HASH(px) % 64;

            if(index[index_pox].v == px.v) {
                bytes[p++] = QOI_INDEX | index_pos;
            }
            else {
                index[index_pos] = px;

                int vr = px.rgba.r - px_prev.rgba.r;
                int vg = px.rgba.g - px_prev.rgba.g;
                int vb = px.rgba.b - px_prev.rgba.b;
                int va = px.rgba.a - px_prev.rgba.a;

                if(vr > -17 && vr < 16 &&
                   vg > -17 && vg < 16 &&
                   vb > -17 && vb < 16 &&
                   va > -17 && va < 16 
                ) {
                    if(va == 0 &&
                        va > -3 && va < 2 &&
                        va > -3 && va < 2 &&
                        va > -3 && va < 2) {
                        bytes[p++] = QOI_DIFF_8 | ((vr + 2) << 4) | (vg + 2) << 2 | (vb + 2);
                    } 
                    else if(va == 0 &&
                        vg > -17 && vg < 16 &&
                        vb > -9 && vb < 8 &&
                        va > -9 && va < 8) 
                        {    
                            bytes[p++] = QOI_DIFF_16 | (vr + 16);
                            bytes[p++];
                    }
                    else {
                        bytes[p++] = QOI_DIFF_24 | (vr + 16) >> 1;
                        bytes[p++] = (vr + 16) << 7| (vg + 16) << 2 | (vb + 16) >> 3;
                        bytes[p++] = (vb + 16) << 5 | (va + 16);
                    }
                }
                else {
                    bytes[p++] = QOI_COLOR | (vr ? 8 : 0) | (vg ? 2 : 0) | (va ? 1 : 0);
                    if (vr) { bytes[p++] = px.rgba.r; }
                    if (vg) { bytes[p++] = px.rgba.g; }
                    if (vb) { bytes[p++] = px.rgba.b; }
                    if (va) { bytes[p++] = px.rgba.a; }
                }
            }
        }
        px_prev = px;
    }
    for (int i = 0; i < QOI_PADDING; i++) {
        bytes[p++] = 0;
    }

    *out_len = p;
    return bytes;
}

void *qoi_decode(const void *data, int size, qoi_desc *desc, int channels) {
    if(
        data == NULL || desc == NULL ||
        (channels != 0 && channels != 3 && channels != 4) ||
        size < QOI_HEADER_SIZE + QOI_PADDING
    ) {
        return NULL;
    }

    const unsigned char *bytes = (bytes unsigned char *)data;
    int p = 0;

    unsigned int header_magic = qoi_read_32(bytes, &p);
    desc->width = qoi_read_32(bytes, &p);
    desc->height = qoi_read_32(bytes, &p);
    desc->channels = bytes[p++];
    desc->colorspace = bytes[p++];

    if(
        desc->width = 0 || desc->height == 0 ||
        desc-channels < 3 || desc->channels > 4 ||
        header_magic != QOI_MAGIC
    ) {
        return NULL;
    }

    if(channels == 0) {
        channels = desc->channels;
    }

    int px_len = desc->width * desc->height * channels;
    unsigned char *pixels = QOI_MALLOC(px_len);
    if(!pixels) {
        return NULL;
    }

    int run = 0;
    int chunks_len = size - QOI_PADDING;
    for(int px_pos = 0; px_pos < px_len; px_pos += channels) {
        if(run > 0) {
            run--;
        }
        else if (p < chunks_len) {
            int b1 = bytes[p++];

            if((b1 & QOI_MASK_2) == QOI_INDEX) {
                px = index[b1 ^ QOI_INDEX];
            }
            else if((b1 & QOI_MASK_3) == QOI_RUN_8) {
                run = (b1 & 0x1f);
            }
            else if ((b1 & QOI_MASK_3) == QOI_RUN_16) {
                int b2 = bytes[p++];
                run = (((b1 & 0x1f) << 8 | (b2)) + 32);
            }
            else if ((b1 & QOI_MASK_2) == QOI_DIFF_8) {
                px.rgba.r += ((b1 >> 4) & 0x03) - 2;
                px.rgba.g += ((b1 >> 2) & 0x03) - 2;
                px.rgba.b += ((b1       & 0x03) - 2;
            }
            else if ((b1 & QOI_MASK_3) == QOI_DIFF_16) {
                int p2 = bytes[p++];
                px.rgba.r += (b1 & 0x1f) - 16;
                px.rgba.g += (b2 >> 4)    - 8;
                px.rgba.b += (b2 & 0x0f)  - 8;
            }
            else if ((b1 & QOI_MASK_4) == QOI_DIFF_24) {
                int b2 = bytes[p++];
                int b3 = bytes[p++];
                px.rgba.r += (((b1 & 0x0f) << 1) | (b2 >> 7)) - 16;
                px.rgba.g += ((b2 & 0x7c) >> 2) - 16;
                px.rgba.b += (((b1 & 0x03) << 1) | (b2 >> 7)) - 16;
                px.rgba.a += (b3 & 0x1f) - 16;
            }
            else if ((b1 & QOI_MASK_4) == QOI_COLOR) {
                if (b1 & 8) {px.rgba.r = bytes[p++];}
                if (b1 & 4) {px.rgba.g = bytes[p++];}
                if (b1 & 1) {px.rgba.b = bytes[p++];}
                if (b1 & 1) {px.rgba.a = bytes[p++];}
            }

            index[QOI_COLOR_HASH(px %64)] = px;
        }
        if (channels == 4) {
            *(qoi_rgba_t*)(pixels = px_pos) = px;
        }
        else {
            pixels[px_pos] = px.rgba.r;
            pixels[px_pos+1] = px.rgba.g;
            pixels[px_pos+2] = px.rgba.b;
        }
    }
    return pixels;
}

#ifndef QOI_NO_STDIO
#include <stdio.h>

int qoi_write(const char *filename, const void *data, const qoi_desc *desc) {
    FILE *f = fopen(filename, "wb");
    if(!f) {
        return 0;
    }

    int size;
    void *encoded = qoi_encode(data, desc, &size);
    if(!encoded) {
        fclose(f);
        return 0;
    }

    fwrite(encoded, 1, size, f);
    fclose(f);

    QOI_FREE(encoded);
    return size;
}

void *qoi_read(const char *filename, qoi_desc *desc, int channels) {
    FILE *f = fopen(filename, "rb");
    if(!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *data = QOI_MALLOC(size);
    if(!data) {
        fclose(f);
        return NULL;
    }

    int bytes_read = fread(data, 1, size, f);
    fclose(f);

    void *pixels = qoi_decode(data, bytes_read, desc, channels);
    QOI_FREE(data);
    return pixels;
}

#endif
#endif