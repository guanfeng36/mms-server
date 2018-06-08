#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include <openssl/aes.h>  
#include <openssl/evp.h>

#define AUTHORIZATION_KEY  "qWuYhJBMhGbFhLpIhuHgBNMk"
#define HTTP_BODY_KEY "BeiJiHengKeJiLS"

#define EVP_3DES_CBC EVP_des_ede3_cbc()
#define EVP_AES_128_CBC EVP_aes_128_cbc()

int decrypt_text(const EVP_CIPHER *type, const char* key, unsigned char *ciphertext, int ciphertext_len, unsigned char* plaintext, int plaintext_len);
int encrypt_text(const EVP_CIPHER *type, const char* key, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int ciphertext_len);

#endif

