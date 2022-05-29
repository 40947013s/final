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

#define range(x) x >= 0 && x <= 255
#define comma(x) if((x) != ',') exit_program;
#define exit_program printf("Invaild input.\n"),\
exit(0);
#define FILE_NULL printf("File not found.\n"),\
exit(0);
#define NOT_BMP printf("This is not bmp file.\n"),\
exit(0);

uint8_t Q[4][3];

double frac = 1.0/(1024*768);
char ask[4][20] = {"top left", "top right", "bottom left", "bottom right"};

uint8_t bit(int i, int j, int flag)
{
    uint8_t ans = (Q[2][flag]*frac)*(768-i)*(1024-j);
    ans += (Q[0][flag]*frac)*(i-0)*(1024-j);
    ans += (Q[3][flag]*frac)*(768-i)*(j-0);
    ans += (Q[1][flag]*frac)*(i-0)*(j-0);
    return ans;
}

uint8_t *Bilinear(int i, int j)
{
    uint8_t *res = malloc(3);
    for(int k = 0; k < 3; k++)
        res[k] = bit(i, j, k);
    return res;
}

int main()
{
    sBmpHeader header;   
    FILE *in = NULL, *out = NULL;
    char *input = malloc(128), *output = malloc(128);
    
    printf("Please enter the file: ");
    scanf("%s", input);
    if((in = fopen(input, "rb")) == NULL) FILE_NULL;
    fread(&header, sizeof(sBmpHeader), 1, in);
    if(strncmp(header.bm, "BM", 2) != 0) NOT_BMP;
    
    fclose(in); 
    out = fopen(input, "wb");
    while ((getchar()) != '\n');   
    
    header.width = 1024;
    header.height = 768;
    header.size = 54 + 768*(1024*3);
    fwrite(&header, sizeof(sBmpHeader), 1, out);
    for(int i = 0; i < 4; i++)
    {
        char str[128], *endptr = NULL;
        printf("Please enter (R,G,B) in the %s pixel: ", ask[i]);
        fgets(str, 128, stdin);
        Q[i][0] = strtol(str, &endptr, 10);
        comma(endptr[0]); endptr++;
        Q[i][1] = strtol(endptr, &endptr, 10);
        comma(endptr[0]); endptr++;
        Q[i][2] = strtol(endptr, &endptr, 10);
        
        /*if(!range(Q[i][0]) || !range(Q[i][1]) ||
        !range(Q[i][2]) || strcmp(endptr, "\n") != 0)
            exit_program;*/
        
    }
    
    for(int i = 0; i < 768; i++)
    {
        uint8_t **res = calloc(1024, sizeof(uint8_t*));
        res = calloc(3, sizeof(uint8_t));
        for(int j = 0; j < 1024; j++)
        {
            res[j] = Bilinear(i, j);
            
            fwrite(res[j], 3, 1, out);
        }
    } 
    return 0;
}
