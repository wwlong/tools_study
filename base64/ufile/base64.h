#ifndef _UFILESDK_C_UCLOUD__BASE64_
#define _UFILESDK_C_UCLOUD__BASE64_

#include <stdio.h>
#include <string.h>
#include <ctype.h> 

enum {
    BASE64_URL = 0,
    BASE64_STD,
};


int base64_encode_t(unsigned char const* , unsigned int len, int type, char *signature);
int base64_decode_t(const char *s, int type, char *signature);

#endif
