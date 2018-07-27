#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "common/log.h"
#include "common/base64.h"

int base64_encode(const char* data, int data_len, char* encoded_data, int max_len) {
    assert(data && (data_len > 0));

    int ret_val = -1;
    gchar* temp_data = g_base64_encode((const guchar*)data, data_len);
    if (temp_data) {
        ret_val = strlen(temp_data);
        if (ret_val < max_len) {
            strncpy(encoded_data, temp_data, ret_val);
        }
        else {
            log_error("memory is not enough[max_len:%d][len:%d]", max_len, ret_val);
            ret_val = -1;
        }
        g_free(temp_data);
    }
    return ret_val;
}

int base64_decode(const char* data, char* decoded_data, int max_len) {

    assert(data);

    int ret_val = -1;
    gsize size = -1;
    guchar* temp_data = g_base64_decode((const gchar*)data, &size);
    if (temp_data) {
        if (size <= max_len) {
            memcpy(decoded_data, temp_data, size);
            ret_val = (int) size;
        }
        else {
            log_error("memory is not enough[max_len:%d][len:%d]", max_len, ret_val);
            ret_val = -1;
        }
        g_free(temp_data);
    }

    return ret_val;
}

/* const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";  */
/* static char find_pos(char ch);  */

/* int base64_encode(const char* data, int data_len, char* encoded_data, int max_len)  */
/* { */
/*     if (!encoded_data || max_len <= 0) { */
/*         log_info("invalid arguments"); */
/*         return -1; */
/*     } */
/*     int prepare = 0;  */
/*     int ret_len;  */
/*     int temp = 0;  */
/*     char *f = NULL;  */
/*     int tmp = 0;  */
/*     char changed[4];  */
/*     int i = 0;  */
/*     ret_len = data_len / 3;  */
/*     temp = data_len % 3;  */
/*     if (temp > 0)  */
/*     {  */
/*         ret_len += 1;  */
/*     }  */
/*     ret_len = ret_len*4 + 1;  */
/*     if (ret_len > max_len) { */
/*         log_info("Not enough memory allocated!"); */
/*         return -1; */
/*     } */

/*     f = encoded_data; */
/*     while (tmp < data_len)  */
/*     {  */
/*         temp = 0;  */
/*         prepare = 0;  */
/*         memset(changed, '\0', 4);  */
/*         while (temp < 3)  */
/*         {  */
/*             //printf("tmp = %d\n", tmp);  */
/*             if (tmp >= data_len)  */
/*             {  */
/*                 break;  */
/*             }  */
/*             prepare = ((prepare << 8) | (data[tmp] & 0xFF));  */
/*             tmp++;  */
/*             temp++;  */
/*         }  */
/*         prepare = (prepare<<((3-temp)*8));  */
/*         //printf("before for : temp = %d, prepare = %d\n", temp, prepare);  */
/*         for (i = 0; i < 4 ;i++ )  */
/*         {  */
/*             if (temp < i)  */
/*             {  */
/*                 changed[i] = 0x40;  */
/*             }  */
/*             else  */
/*             {  */
/*                 changed[i] = (prepare>>((3-i)*6)) & 0x3F;  */
/*             }  */
/*             *f = base[(unsigned char)changed[i]];  */
/*             //printf("%.2X", changed[i]);  */
/*             f++;  */
/*         }  */
/*     }  */
/*     *f = '\0';  */
/*     return ret_len; */
/* }  */
/* /\* *\/  */
/* static char find_pos(char ch)    */
/* {  */
/*     char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[]  */
/*     return (ptr - base);  */
/* }  */
/* /\* *\/  */
/* int base64_decode(const char* data, int data_len, char* decoded_data, int max_len) */
/* { */
/* //    printf("%d   %d\n",strlen(data),data_len);  */
/* //    printf("%s11111111111111\n",data); */
/*     if (!decoded_data || max_len <= 0) { */
/*         log_info("invalid arguments"); */
/*         return -1; */
/*     } */
/*     int ret_len = (data_len / 4) * 3 ;  */
/*     int equal_count = 0;  */
/*     char *f = NULL;  */
/*     int tmp = 0;  */
/*     int temp = 0;  */
/*     char need[3];  */
/*     int prepare = 0;  */
/*     int i = 0;  */
/*     if (*(data + data_len - 1) == '=')  */
/*     {  */
/*         equal_count += 1;  */
/*     }  */
/*     if (*(data + data_len - 2) == '=')  */
/*     {  */
/*         equal_count += 1;  */
/*     }  */
/*     if (*(data + data_len - 3) == '=')  */
/*     {//seems impossible  */
/*         equal_count += 1;  */
/*     }  */
/*     switch (equal_count)  */
/*     {  */
/*     case 0:  */
/*         ret_len += 4;//3  */
/*         break;  */
/*     case 1:  */
/*         ret_len += 4;//Ceil((6*3)/8)+1  */
/*         break;  */
/*     case 2:  */
/*         ret_len += 3;//Ceil((6*2)/8)+1  */
/*         break;  */
/*     case 3:  */
/*         ret_len += 2;//Ceil((6*1)/8)+1  */
/*         break;  */
/*     }  */

/*     if (ret_len > max_len) { */
/*         log_info("Not enough memory allocated!"); */
/*         return -1; */
/*     } */
    
/*     f = decoded_data; */
/*     while (tmp < (data_len - equal_count))  */
/*     {  */
/*         temp = 0;  */
/*         prepare = 0;  */
/*         memset(need, 0, 4);  */
/*         while (temp < 4)  */
/*         {  */
/*             if (tmp >= (data_len - equal_count))  */
/*             {  */
/*                 break;  */
/*             }  */
/*             prepare = (prepare << 6) | (find_pos(data[tmp]));  */
/*             temp++;  */
/*             tmp++;  */
/*         }  */
/*         prepare = prepare << ((4-temp) * 6);  */
/*         for (i=0; i<3 ;i++ )  */
/*         {  */
/*             if (i == temp)  */
/*             {  */
/*                 break;  */
/*             }  */
/*             *f = (char)((prepare>>((2-i)*8)) & 0xFF);  */
/*             f++;  */
/*         }  */
/*     }  */
/*     *f = '\0'; */

/*     return ret_len;  */
/* } */
