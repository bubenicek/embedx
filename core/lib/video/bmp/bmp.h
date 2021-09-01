
#ifndef __BMP_H
#define __BMP_H

#define BMP_TYPE_IDENT      0x4D42

#define BMP_SIZE(w, h) ((h) * ((w) * 3 + (((w) * -3UL) & 3)) + 14 + 40)

/**
 * Pad the given width to the nearest 32-bit boundary
 */
#define BMP_PAD(width)  (((width) + 3) & (~3))

typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;

}  __attribute__ ((__packed__)) BITMAP_FILE_HEADER;

typedef struct
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

}  __attribute__ ((__packed__)) BITMAP_INFO_HEADER;

typedef struct
{
    BITMAP_FILE_HEADER file;
    BITMAP_INFO_HEADER info;

}  __attribute__ ((__packed__)) BITMAP_HEADER;

#define BITMAP_HEADER_SIZE  sizeof(BITMAP_HEADER)

/** Create bitmap header in buffer */
int bmp_create(uint8_t *buf, int width, int height);

/** Get bitmap header */
BITMAP_HEADER *bmp_get_header(uint8_t *buf);


#endif   // __BMP_H
