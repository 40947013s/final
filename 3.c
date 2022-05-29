#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct wav_header 
{
    char riff_header[4];
    int wav_size;
    char wave_header[4];
    char fmt_header[4];
    int fmt_chunk_size;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short sample_alignment;
    short bit_depth;
    char data_header[4];
    int data_bytes;
} wav_header;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void mix_wav(char *n1, char *n2, char *n3)
{
    FILE *fp1 = fopen(n1, "r");
    FILE *fp2 = fopen(n2, "r");
    wav_header input1, input2;    
    
    fread(&input1, sizeof(input1), 1, fp1);
    fread(&input2, sizeof(input2), 1, fp2);
    
    int ori_data_bytes = input1.data_bytes;
    input1.data_bytes += input2.data_bytes;
    input1.wav_size = input1.data_bytes+36;
    
    FILE *out = fopen(n3, "w");
    fwrite(&input1, sizeof(input1), 1, out);
    int i = 0;
    while(i < ori_data_bytes+input2.wav_size)
    {
        char c;
        if(i < ori_data_bytes)
            fread(&c, sizeof(c), 1, fp1);
        else            
            fread(&c, sizeof(c), 1, fp2);
        fwrite(&c, sizeof(c), 1, out);
        i++;
    }    
}

#define range(x) ('1' <= x && x <= '8') && x != '6'
#define ptk(x) x == 'p' || x == 't' || x == 'k'
#define h(x) x == 'k'

int change(char *str, char ***sep)
{
    int size = 1;
    for(int i = 0; i < strlen(str); i++)
        if(str[i] == ' ' && str[i-1] != ' ')
            size++;
    
    char *copy = malloc(strlen(str));
    strcpy(copy, str);
    *sep = calloc(size, sizeof(char*));
    size = 0;
    
    char *token = strtok(copy, " ");
    while(token != NULL)
    {        
        char *copy_token = calloc(strlen(token)+1, sizeof(char));
        for(int i = 0; i < strlen(token); i++)
            copy_token[i] = token[i];
        copy_token[strlen(token)] == 0;
        *(*sep+size) = copy_token;
        token = strtok(NULL, " ");
        size++;
    }
    return size;
}

int word(char *str, char ***sep)
{
    int size = 1;
    for(int i = 0; i < strlen(str); i++)
        if(str[i] == '-' && str[i-1] != '-')
            size++;
    
    char *copy = malloc(strlen(str));
    strcpy(copy, str);
    *sep = calloc(100, sizeof(char*));
    size = 0;
    
    char *token = strtok(copy, "-");
    while(token != NULL)
    {        
        char *copy_token = calloc(strlen(token)+1, sizeof(char));
        for(int i = 0; i < strlen(token); i++)
            copy_token[i] = token[i];
        copy_token[strlen(token)] == 0;
        *(*sep+size) = copy_token;
        token = strtok(NULL, "-");
        size++;
    }
    return size;
}

char *plus(char *str)
{
    char *p = malloc(strlen(str));
    memset(p, 0, strlen(str));
    for(int i = 0; i < strlen(str)-1; i++)
        p[i] = str[i] == ' ' ? '+' : str[i];    
    return p;
}

bool IsTai(char *str, bool endofend)
{
    if(str[strlen(str)-1] == '.' && range(str[strlen(str)-2]) && endofend) 
    {
        for(int i = 0; i < strlen(str)-2; i++)
            if(!isalpha(str[i])) return false;
        return true;
    }
    if(range(str[strlen(str)-1]) && !endofend)
    {
        for(int i = 0; i < strlen(str)-1; i++)
            if(!isalpha(str[i])) return false;
        return true;
    }
    return false;
}

char *modify(char *str, bool endofend, int *size)
{
    if(endofend) return str;
    else
    {
        char *res = calloc(strlen(str)+2, sizeof(char));
        strncpy(res, str, strlen(str)-1);
        char endalpha = str[strlen(str)-2];
        char num = str[strlen(str)-1];
        if(num == '1') num = '7';
        else if(num == '2') num = '1';
        else if(num == '3') num = '2';
        else if(num == '4' && ptk(endalpha)) num = '8';
        else if(num == '4' && h(endalpha)) num = '2';
        else if(num == '5') num = '7';
        else if(num == '7') num = '3';
        else if(num == '8' && ptk(endalpha)) num = '4';
        else if(num == '8' && h(endalpha)) num = '3';
        
        res[strlen(str)-1] = num;
        res[strlen(str)] = '.';
        (*size) = strlen(str);
        return res;
    }  
}

int main() 
{
    char *input = malloc(4096), *copy = malloc(4096);
    printf("Input : ");
    fgets(input, 4096, stdin);
    input[strlen(input)-1] = 0;
    if(input[strlen(input)-1] != '.')
    {
        printf("Invaild Input!\n");
        exit(0);
    } strcpy(copy ,input);
    
    char **res1, **res2, *res3 = malloc(1024);
    int size, size2, count = 0;
    size = change(copy, &res1);
    res3 = plus(input);
    
    printf("Output : ");
    for(int i = 0; i < size; i++)
    {
        size2 = word(res1[i], &res2);
        for(int j = 0; j < size2; j++) 
        {
            CURL *curl;
            FILE *fp;
            CURLcode res;
            char error[CURL_ERROR_SIZE], outfilename[FILENAME_MAX], url[1024];
            char replace[strlen(res2[j])];
            int mod_size = 0;
            strcpy(replace, res2[j]);
            bool endofend = (j == size2-1 && i == size-1) ? true : false;
            if(!IsTai(replace, endofend)) 
            {
                printf("Invaild Input!\n");
                for(int k = 0; k < count; k++)
                {
                    char outfilename[FILENAME_MAX]; 
                    sprintf(outfilename, "%d.wav", i);
                    remove(outfilename);
                }
                exit(0);
            }
            printf("%s ", modify(replace, endofend,&mod_size));
            sprintf(url, "https://hapsing.ithuan.tw/bangtsam?taibun=%s", modify(replace, endofend,&mod_size));
        
            sprintf(outfilename, "%d.wav", count);            
        	curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();
            if (curl) 
            {
                fp = fopen(outfilename, "wb");
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                fclose(fp);
            }
            count++;
        }       
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;
    char error[CURL_ERROR_SIZE], outfilename[FILENAME_MAX], url[1024];
    sprintf(url, "https://hapsing.ithuan.tw/bangtsam?taibun=%s", res3);
        
    sprintf(outfilename, "%d.wav", count);  
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) 
    {
        fp = fopen(outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    count++;
    if(count >= 2)
        mix_wav("0.wav", "1.wav", "out.wav");
    for(int i = 2; i < count; i++)
    {
        char outfilename[FILENAME_MAX]; 
        sprintf(outfilename, "%d.wav", i);
        mix_wav("out.wav", outfilename, "temp.wav");
        rename("temp.wav", "out.wav");        
    }
       
    printf("%s\n", copy);
    return 0;
}