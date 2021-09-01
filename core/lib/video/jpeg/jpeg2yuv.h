
#ifndef __JPEG2YUV_H
#define __JPEG2YUV_H


/** Convert JPG image buffer to YUV buffer */
int convert_jpeg2yuv420(const uint8_t *jpegdata, int jpegdata_size, uint8_t **yuvdata, int *yuvdata_size, int *width, int *height);

/** Convert JPG image file to YUV buffer */
int convert_jpeg2yuv420_file(const char *jpg_filename, uint8_t **yuvdata, int *yuvdata_size);

#endif   // __JPEG2YUV_H
