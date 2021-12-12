#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define QOI_IMPLEMETATION
#include "qoi.h"

#define STR_ENDS_WITH(S, E) (strcmp(S + strlen(S) - (sizeof(E)-1), E) == 0)

int main(int argc, char **argv) {
    if(argc < 3) {
		printf("Usage: qoiconv <infile> <outfile>\n");
		printf("Examples:\n");
		printf("  qoiconv input.png output.qoi\n");
		printf("  qoiconv input.qoi output.png\n");
		exit(1);
    }
    void *pixels = NULL;
    int w, h, channels;
    if(STR_ENDS_WITH(argv[1], ".png")) {
        pixels = (void *)stbi_load(argv[1], &w, &h, &channels, 0);
    }
    else if(STR_ENDS_WITH(argv[1], ".qoi")) {
        qoi_desc desc;
        pixels = qoi_read(argv[1], &desc, 0);
        channels = desc.channels;
        w = desc.width;
        h = desc.height;
    }

    if(pixels == NULL) {
		printf("Couldn't load/decode %s\n", argv[1]);
        exit(1);
    }

    int encoded = 0;
    
    if(STR_ENDS_WITH(argv[2], ".png")) {
        encoded = stbi_write_png(argv[2], w, h, channels, pixels, 0);
    }
    else if(STR_ENDS_WITH(argv[2], ".qoi")) {
        encoded = qoi_write(argv[2], pixels, &(qoi_desc){
            .width = w,
            .height = h,
            .channels = channels,
            .colorspace = QOI_SRGB
        });
    }
    if(!encoded) {
		printf("Couldn't write/encode %s\n", argv[2]);
		exit(1);
    }
    free(pixels);
    return 0;
}