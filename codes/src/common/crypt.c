#include <string.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <openssl/err.h>

#include "common/log.h"
#include "common/crypt.h"

#define BUFFER_MAX_SIZE 512
//#define IV      "qazqazxswOKMNwsxcPOdeerf"

char buffer[BUFFER_MAX_SIZE] = {0};
  
int decrypt_text(const EVP_CIPHER *type, const char* key, unsigned char *ciphertext, int ciphertext_len, unsigned char* plaintext, int plaintext_len) 
{
    EVP_CIPHER_CTX ctx;  
    EVP_CIPHER_CTX_init(&ctx);  
    int bytes_written = 0;  
    int update_len = 0;  
    if (!EVP_DecryptInit_ex(&ctx, type, NULL, (unsigned char*)key, NULL))
    {  
        log_error("ERROR in EVP_DecryptInit_ex \n");  
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }
    // EVP_CIPHER_CTX_set_padding(&ctx, 0);
    if (!EVP_DecryptUpdate(&ctx, plaintext, &update_len, ciphertext, ciphertext_len))
    {  
        log_error("ERROR in EVP_DecryptUpdate\n");
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }
    bytes_written += update_len;
  
    if(!EVP_DecryptFinal_ex(&ctx, plaintext + bytes_written, &update_len))
    { 
        ERR_print_errors_fp(stderr);
        log_error("ERROR in EVP_DecryptFinal_ex\n");  
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }  
    bytes_written += update_len;  
    EVP_CIPHER_CTX_cleanup(&ctx);
    return bytes_written;
}  
  
int encrypt_text(const EVP_CIPHER *type, const char* key, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int ciphertext_len) 
{
    int bytes_written = 0;  
    int update_len = 0;  
    EVP_CIPHER_CTX ctx;  
    EVP_CIPHER_CTX_init(&ctx);
    if (!EVP_EncryptInit_ex(&ctx, type, NULL, (unsigned char*)key, NULL))
    {  
        log_error("ERROR in EVP_EncryptInit_ex \n");
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }  
    // EVP_CIPHER_CTX_set_padding(&ctx, 0);
    if(!EVP_EncryptUpdate(&ctx, ciphertext, &update_len, (unsigned char *) plaintext, plaintext_len)) 
    {  
        log_error("ERROR in EVP_EncryptUpdate \n");
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }  
    bytes_written += update_len;  
  
    if(!EVP_EncryptFinal_ex(&ctx, ciphertext + bytes_written, &update_len))
    {  
        ERR_print_errors_fp(stderr);
        log_error("ERROR in EVP_EncryptFinal_ex \n");
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }  
    bytes_written += update_len;
  
    EVP_CIPHER_CTX_cleanup(&ctx);
  
    return bytes_written;
}  
