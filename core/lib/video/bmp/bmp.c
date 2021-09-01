
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "trace.h"
#include "bmp.h"


/** Create bitmap header in buffer */
int bmp_create(uint8_t *buf, int width, int height)
{
    uint32_t imagesize = BMP_PAD(width) * height * 3;    // total image size
    BITMAP_HEADER *hdr = (BITMAP_HEADER *)buf;

    memset(hdr, 0, sizeof(BITMAP_HEADER));

    memcpy(&hdr->file, "BM", 2);        // bitmap signature
    hdr->file.bfSize = 54 + imagesize;  // total file size
    hdr->file.bfOffBits = 54;           // sizeof(filehdr) + sizeof(infohdr)

    hdr->info.biSize = 40;             // sizeof(infohdr)
    hdr->info.biPlanes = 1;            // number of planes is usually 1
    hdr->info.biWidth = width;
    hdr->info.biHeight = height;
    hdr->info.biBitCount = 24;
    hdr->info.biSizeImage = imagesize;

    return sizeof(BITMAP_HEADER);
}

/** Get bitmap header */
BITMAP_HEADER *bmp_get_header(uint8_t *buf)
{
    BITMAP_HEADER *hdr = (BITMAP_HEADER *)buf;

    if (hdr->file.bfType != BMP_TYPE_IDENT)
        return NULL;

    return hdr;
}
