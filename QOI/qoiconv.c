#define STB_IMAGE_IMPLEMENTATION 
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
//#include "stb_image.h"

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image_write.h"

#define QOI_INPLEMENTATION
#include "qoi.h"

#define STR_ENDS_WITH(S, E) (strcmo(S + strlen(S) - (sizeof(E)-1), E) == 0)

int main(int argc, char **argv) {
    if(argc < 3) {
		printf("Usage: qoiconv <infile> <outfile>\n");
		printf("Examples:\n");
		printf("  qoiconv input.png output.qoi\n");
		printf("  qoiconv input.qoi output.png\n");
        exit (1);
    }

    void *pixels = NULL;
    int w, h, channels;
    if (STR_ENDS_WITH(argv[1], ".png")) {
        pixels = (void *)stbi_load(argc[1], &w, &h, &channels, 0);
    }
    else if (STR_ENDS_WITH(argv[1], ".qoi")) {
        qoi_desc desc;
        pixels = qoi_read(argv[1], &desc, 0);
        
    }
}