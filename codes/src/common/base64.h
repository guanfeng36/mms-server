#ifndef _BASE64_H_
#define _BASE64_H_

int base64_encode(const char* data, int data_len, char* encoded_data, int max_len);
int base64_decode(const char* data, char* decoded_data, int max_len);

//int Base64Encode(const char *decoded, int decodedLength, char *encoded);
//int Base64Decode(const char *encoded, int encodedLength, char *decoded);

#endif
