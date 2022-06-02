#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <math.h>

struct option long_options[] = 
{  
     {"write",  0, NULL, 'w'},  
     {"extract", 0, NULL, 'e'},
     {"bits", 1, NULL, 'b'},
     { 0, 0, 0, 0},  
};

typedef struct _sheader
{
    char        bm[2];
    uint32_t    size;
    uint32_t    reserve;
    uint32_t    offset;
    uint32_t    header_size;
    uint32_t    width;
    uint32_t    height;
    uint16_t    planes;
    uint16_t    bpp;
    uint32_t    compression;
    uint32_t    bitmap_size;
    uint32_t    hres;
    uint32_t    vres;
    uint32_t    used;
    uint32_t    important;
}__attribute__ ((__packed__)) Bmpheader;

#define Invalid printf("Invaild input.\n"),\
exit(0);
#define FILE_NULL printf("File not found.\n"),\
exit(0);
#define FILE_EXCEED printf("File exceed.\n"),\
exit(0);

unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

int main ( int argc, char *argv[]) {

	int32_t c = 0, N = 1;
    FILE *f1, *f2, *f3;
    Bmpheader header;
    bool w = false, e = false; 

    while((c = getopt_long(argc, argv, "web:", long_options, 0)) != -1) 
    {
    	switch (c)
    	{
    		case 'w':
    			w = true; break;
    		case 'e':
    			e = true; break;
            case 'b':
                N = strtol(optarg, NULL, 10);
                if(N < 0 || N > 9) Invalid;
            	break;
        }
    }
    if(!w^e) Invalid;
    if(w) 
    {
        rename(argv[optind],"rm.bmp");
        if((f1 = fopen(argv[optind], "wb")) == NULL) FILE_NULL;
        if((f2 = fopen(argv[optind+1], "rb")) == NULL) FILE_NULL;
        if((f3 = fopen("rm.bmp", "rb")) == NULL) FILE_NULL;
        
        fread(&header, sizeof(Bmpheader), 1, f3);
        fseek(f2, 0, SEEK_END);
        long size = ftell(f2);
        rewind(f2);

        if(size > header.bitmap_size*N) FILE_EXCEED;
        header.reserve = size;
        fwrite(&header, sizeof(Bmpheader), 1, f1);

        int count_src = 8, count_bmp = 0;
        uint8_t src = 0, bmp = 0, bit = 0;
        fread(&src, 1, 1, f2);
        fread(&bmp, 1, 1, f3); 
        bmp = (bmp >> N) << N;

        while(1) 
        {
            if(count_src == 0) 
            {
                if(feof(f2)) break;
                fread(&src, 1, 1, f2);
                count_src = 8;
            }
            if(count_bmp == N) 
            {
                fwrite(&bmp, 1, 1, f1);
                fread(&bmp, 1, 1, f3);
                count_bmp = 0;
                bmp = (bmp >> N) << N;
            }
            int s = count_src, b = count_bmp;
            bit = ((s > b) ? src >> (s-b-1) : src << (b-s+1)) & (1 << b);
            bmp = bit | bmp;
            count_bmp++, count_src--; 
        }
        fwrite(&bmp, 1, 1, f1);

        while(!feof(f3)) 
        {
            uint8_t c;            
            fread(&c, 1, 1, f3);
            fwrite(&c, 1, 1, f1);
        }
    
        fclose(f1); fclose(f2); fclose(f3);
        remove("rm.bmp");
    }
    
    if(e) 
    {
        if((f1 = fopen(argv[optind], "rb")) == NULL) FILE_NULL;
        if((f2 = fopen(argv[optind+1], "wb"))== NULL) FILE_NULL;
        fread(&header, sizeof(Bmpheader), 1, f1);
        long size = header.reserve;
        uint8_t src = 0, bmp = 0, bit = 0;
        int count_done = 0, count_wait = 8, count_size = size;
        fread(&bmp, 1, 1, f1);  

        while(1) 
        {
            if(count_wait == 0) 
            {
                fwrite(&src, 1, 1, f2);
                count_size--;
                if(count_size == 0) break;
                src = 0, count_wait = 8;
            }
            if(count_done == N) 
            {
                fread(&bmp, 1, 1, f1);  
                count_done = 0;
            }
            int w = count_wait, d = count_done;
            bit = ((w > d) ? bmp << (w-d-1) : bmp >> (d-w+1)) & (1 << (w-1));
            src = bit | src;
            count_done++, count_wait--;
        }

        fclose(f1); fclose(f2);
    }
}