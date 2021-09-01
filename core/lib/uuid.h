
#ifndef __UUID_H
#define __UUID_H

#define UUID_NBYTES           16
#define UUID_STRLEN           37


char *uuid128_make(uint8_t *data, int datalen, char *uuid, int uuidlen);
char *uuid128_make_u32(uint32_t u32, char *uuid, int uuidlen);
char *uuid128_make_random(char *uuid, int uuidlen);

int uuid128_parse(const char *uuid, uint8_t *data, int datalen);
int uuid128_parse_u32(const char *in, uint32_t *u32);


#endif // __UUID_H