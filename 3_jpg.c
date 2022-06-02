#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

#define FILE_NULL printf("File not found.\n"),\
exit(0);
#define FILE_EXCEED printf("File exceed.\n"),\
exit(0);
#define THUMBNAIL printf("Thumbnail exist.\n"),\
exit(0);

struct option long_options[] = 
{  
    {"write", 0, NULL, 'w'},
    {"extract", 0, NULL, 'e'},    
    {0 , 0 , 0 , 0},  
};  

typedef struct _sheader_b
{
    char bm[2];
    uint32_t size;
    uint32_t reserve;
    uint32_t offset;
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t bitmap_size;
    uint32_t hres;
    uint32_t vres;
    uint32_t used;
    uint32_t important;
}__attribute__((__packed__)) Bmpheader;

typedef struct _sJpgheader
{
    char SOI[2];
    char APP0[2];
    uint16_t Length;
    uint8_t identifier[5];
    uint16_t Version;
    uint8_t Units;
    uint16_t Xdensity;
    uint16_t Ydensity;
    uint8_t XThumbnail;
    uint8_t YThumbnail;
}__attribute__((__packed__)) Jpgheader;

int main(int argc, char *argv[])
{
    int32_t c = 0, func = -1;
    FILE *f1, *f2, *f3;
    Bmpheader header_b;
    Jpgheader header_j;
    
    while((c = getopt_long(argc, argv, "we", long_options, 0)) != -1)
    {
        switch(c)
        {
            case 'w': func = 1; break;
            case 'e': func = 2; break;
        }        
    }

    if(func == 1)
    {        
        rename(argv[optind], "rm.jpg");

    	if((f1 = fopen("rm.jpg", "rb")) == NULL) FILE_NULL;
    	if((f2 = fopen(argv[optind], "wb")) == NULL) FILE_NULL;
        if((f3 = fopen(argv[optind+1], "rb")) == NULL) FILE_NULL;

    	fread(&header_j, sizeof(Jpgheader), 1, f1);
    	fread(&header_b, sizeof(Bmpheader), 1, f3);

    	uint32_t X = header_b.width, Y = header_b.height;
		uint32_t rgb = X*Y;
        uint32_t size = header_j.Length + 3*rgb;
        
        if(X > 255 || Y > 255 || size > 65535) FILE_EXCEED;        
        if(header_j.XThumbnail != 0 || header_j.YThumbnail != 0) THUMBNAIL;
		header_j.XThumbnail = X, header_j.YThumbnail = Y;
        header_j.Length = size;

		fwrite(&header_j, sizeof(Jpgheader), 1, f2);		
		uint8_t **mem = calloc(rgb, sizeof(uint8_t*));
    	for(int i = 0; i < rgb; i++)
    		mem[i] = calloc(3, sizeof(uint8_t));
        
    	for(int i = 0; i < rgb; i++) 
        {
    		uint8_t	*bmp = malloc(3);
    		fread(bmp, 3, 1, f3); 
            for(int j = 0; j < 3; j++)
    		    mem[rgb-1-i][j] = bmp[2-j];    		
    	}
        
    	for(int i = 0; i < rgb; i++) 
    		fwrite(mem[i], 3, 1, f2);

        while(!feof(f1)) 
        {
            uint8_t *ori = malloc(1024);
            fread(ori, 1024, 1, f1);
            fwrite(ori, 1024, 1, f2);
        }
        fclose(f1); fclose(f2); fclose(f3);
		remove("rm.jpg");
    }
    
    else if(func == 2)
    {
        if((f1 = fopen(argv[optind], "rb")) == NULL) FILE_NULL;
        if((f2 = fopen(argv[optind+1], "wb")) == NULL) FILE_NULL;

    	fread(&header_j, sizeof(Jpgheader), 1, f1);

        uint32_t X = header_j.XThumbnail, Y = header_j.YThumbnail;
		uint32_t rgb = X*Y;
        uint32_t bits = ((X*3)/4+(X*3)%4)*4;
    	uint32_t allbits = Y*bits;
		uint32_t modbits = bits-X*3;

    	header_b.bm[0] = 'B', header_b.bm[1] = 'M';
    	header_b.size = 54+allbits;
    	header_b.reserve = 0;
   		header_b.offset = 54, header_b.header_size = 40;
    	header_b.width = X, header_b.height = Y;
    	header_b.planes = 1;
    	header_b.bpp = 24;
    	header_b.compression = 0;
    	header_b.bitmap_size = allbits;
    	header_b.hres = 0, header_b.vres = 0;
    	header_b.used = 0, header_b.important = 0;

    	fwrite(&header_b, sizeof(Bmpheader), 1, f2);
    	
    	uint8_t **mem = calloc(rgb, sizeof(uint8_t*));
    	for(int i = 0; i < rgb; i++)
    		mem[i] = calloc(3, sizeof(uint8_t));

        for(int i = 0; i < rgb; i++) 
        {
    		uint8_t	*jpg = malloc(3);
    		fread(jpg, 3, 1, f1); 
            for(int j = 0; j < 3; j++)
    		    mem[rgb-1-i][j] = jpg[2-j];    		
    	}
        
        for(int i = 0; i < Y; i++) 
        {
            uint8_t *mod = malloc(3);
    		for(int j = 0; j < X; j++) 
            {
        		uint8_t	*bmp = malloc(3);
                for(int k = 0; k < 3; k++)
        		    bmp[k] = mem[(i+1)*X-1-j][k];
    			fwrite(bmp, 3, 1, f2);
        	} fwrite(mod, 1, modbits, f2);	
    	}

        fclose(f1); fclose(f2);
    }
    return 0;
}