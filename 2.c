#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct _sBmpHeader
{
    char		bm[2];
    uint32_t	size;
    uint32_t	reserve;
    uint32_t	offset;
    uint32_t	header_size;
    uint32_t	width;
    uint32_t	height;
    uint16_t	planes;
    uint16_t	bpp;
    uint32_t	compression;
    uint32_t	bitmap_size;
    uint32_t	hres;
    uint32_t	vres;
    uint32_t	used;
    uint32_t	important;
}__attribute__ ((__packed__));

typedef struct _sBmpHeader sBmpHeader;

uint8_t rgb_mask[12] = {
    0x00, 0xF8, 0x00, 0x00,      
 	0xE0, 0x07, 0x00, 0x00, 
	0x1F, 0x00, 0x00, 0x00 
};

#define range(x) x >= 0 && x <= 255
#define comma(x) if((x) != ',') exit_program;
#define exit_program printf("Invaild input.\n"),\
exit(0);
#define FILE_NULL printf("File not found.\n"),\
exit(0);
#define NOT_BMP printf("This is not bmp file.\n"),\
exit(0);

int main()
{
    sBmpHeader header;   
    FILE *in = NULL, *out = NULL;
    char *input = malloc(128), *output = malloc(128);
    
    printf("Please enter the input image name: ");
    scanf("%s", input);
    if((in = fopen(input, "rb")) == NULL) FILE_NULL;
    fread(&header, sizeof(sBmpHeader), 1, in);
    if(strncmp(header.bm, "BM", 2) != 0) NOT_BMP;
    
    printf("Please enter the output image name: ");
    scanf("%s", output);   
    if((out = fopen(output, "wb")) == NULL) FILE_NULL;
    while ((getchar()) != '\n');

    int width_size, head_size, data_offset;
    width_size = ((header.width*16+31) >>5)<< 2;
    header.bpp = 16;
    header.compression = 3;
    header.bitmap_size = header.height * width_size;
    header.size  = head_size+header.bitmap_size;
	header.offset = head_size;
    fwrite(&header, sizeof(sBmpHeader), 1, out);
    fwrite(rgb_mask, 12, 1, out);
    
    while(!feof(in))
    {
        uint8_t *ori = malloc(3);
        uint16_t mod;
        fread(ori, 3, 1, in);
        mod = (ori[0] >> 3) + ((ori[1] >> 2) << 5) + ((ori[2] >> 3) << 11);	         
        fwrite(&mod, sizeof(uint16_t), 1, out);
    } 
    return 0;
}
