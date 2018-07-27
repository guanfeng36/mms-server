#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include "common/crypt.h"
#include "encrypt_tool.h"

int main(int argc, char *argv[]) {
    struct option long_opts[] = {
        {"encode", no_argument, NULL, 'e'},
        {"decode", no_argument, NULL, 'd'},
        {"infile", required_argument, NULL, 'i'},
        {"outfile", required_argument, NULL, 'o'},
        {NULL, 0, NULL, 0}
    };

    bool encode = false;
    bool decode = false;
    const char* in_file = NULL;
    const char* out_file = NULL;

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "edi:o:", long_opts, NULL)) != -1) 
    {
        switch (opt) {
        case 'e':
            encode = true;
            break;
        case 'd':
            decode = true;
            break;
        case 'i':
            in_file = strdup(optarg);
            break;
        case 'o':
            out_file = strdup(optarg);
            break;
        default:
            break;
        }
    }

    printf("encode[%d]decode[%d]infile[%s]outfile[%s]\n", encode, decode, in_file, out_file);

    if ((encode && decode) || (!encode && !decode) || !in_file || !out_file) {
        fprintf(stderr, "Invalid argument\n");
        return -1;
    }

    char* in_file_data = NULL;
    int in_file_data_len = 0;
    {
        FILE* infilefd = fopen(in_file, "rb");
        if (infilefd == NULL) {
            fprintf(stderr, "infile[%s] is not existed.\n", in_file);
            return -1;
        }

        fseek(infilefd, 0, SEEK_END);
        in_file_data_len = ftell(infilefd);
        fseek(infilefd, 0, SEEK_SET);

        in_file_data = (char *) malloc(in_file_data_len);
        memset(in_file_data, 0, in_file_data_len);

        fread(in_file_data, 1, in_file_data_len, infilefd);
        fclose(infilefd);
    }

    if (!in_file_data || in_file_data_len <= 0) {
        fprintf(stderr, "failed to read in file data\n"); 
        free(in_file_data);
        return -1;
    }

    int result_data_len_max = in_file_data_len + sizeof(HTTP_BODY_KEY);
    char* result_data = (char*) malloc(result_data_len_max);
    int result_data_len = 0;
    memset(result_data, 0, result_data_len_max);

    if (encode) {
        result_data_len = encrypt_text(EVP_AES_128_CBC, HTTP_BODY_KEY, (unsigned char*)in_file_data, in_file_data_len, (unsigned char*)result_data, result_data_len_max);
    }
    else {
        result_data_len = decrypt_text(EVP_AES_128_CBC, HTTP_BODY_KEY, (unsigned char*)in_file_data, in_file_data_len, (unsigned char*)result_data, result_data_len_max);
    }
    if (result_data_len <= 0) {
        fprintf(stderr, "failed to encrypt/decrypt input file");
        free(in_file_data);
        free(result_data);
        return -1;
    }

    {
        FILE* outfilefd = fopen(out_file, "w+");
        if (NULL == outfilefd) {
            free(in_file_data);
            fprintf(stderr, "Failed to creat outfile[%s].\n", out_file);
            free(in_file_data);
            free(result_data);
            return -1;
        }
        
        fwrite(result_data, 1, result_data_len, outfilefd);
        fclose(outfilefd);
    }

    free(in_file_data);
    free(result_data);
    return 0;
}
