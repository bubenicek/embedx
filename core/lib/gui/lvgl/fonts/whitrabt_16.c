#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 13 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef WHITRABT_16
#define WHITRABT_16 1
#endif

#if WHITRABT_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xff, 0xcf,

    /* U+22 "\"" */
    0x99,

    /* U+23 "#" */
    0x49, 0x2f, 0xd2, 0xfd, 0x24, 0x92,

    /* U+24 "$" */
    0x10, 0xfb, 0x43, 0xe1, 0x42, 0x9f, 0x8,

    /* U+25 "%" */
    0xce, 0x46, 0x66, 0x62, 0x31,

    /* U+26 "&" */
    0x62, 0x49, 0x3c, 0x62, 0xd8, 0xdd,

    /* U+27 "'" */
    0xc0,

    /* U+28 "(" */
    0xea, 0xab,

    /* U+29 ")" */
    0xd5, 0x57,

    /* U+2A "*" */
    0x59, 0xcf, 0xcc, 0x50, 0x0,

    /* U+2B "+" */
    0x21, 0x3e, 0x42, 0x10,

    /* U+2C "," */
    0xf8,

    /* U+2D "-" */
    0xfc,

    /* U+2E "." */
    0xf0,

    /* U+2F "/" */
    0x4, 0x10, 0x84, 0x21, 0x8, 0x20,

    /* U+30 "0" */
    0x7a, 0x18, 0xe7, 0xe7, 0x18, 0x5e,

    /* U+31 "1" */
    0x59, 0x24, 0x97,

    /* U+32 "2" */
    0x7a, 0x10, 0x43, 0x18, 0xc6, 0x3f,

    /* U+33 "3" */
    0xfa, 0x10, 0x47, 0x4, 0x18, 0x5e,

    /* U+34 "4" */
    0xa, 0x28, 0xa2, 0xfc, 0x20, 0x82,

    /* U+35 "5" */
    0xfe, 0x8, 0x3e, 0x4, 0x10, 0x7e,

    /* U+36 "6" */
    0xfa, 0x8, 0x3e, 0x86, 0x18, 0x5e,

    /* U+37 "7" */
    0xfc, 0x10, 0x43, 0x18, 0xc4, 0x20,

    /* U+38 "8" */
    0xfa, 0x18, 0x7f, 0x86, 0x18, 0x5e,

    /* U+39 "9" */
    0xfa, 0x18, 0x7f, 0x4, 0x10, 0x7f,

    /* U+3A ":" */
    0xf0, 0x3c,

    /* U+3B ";" */
    0xf0, 0x3e,

    /* U+3C "<" */
    0x19, 0x99, 0x8c, 0x30, 0xc3,

    /* U+3D "=" */
    0xfc, 0xf, 0xc0,

    /* U+3E ">" */
    0x82, 0x8, 0x31, 0x91, 0x10,

    /* U+3F "?" */
    0x7a, 0x10, 0x43, 0x18, 0x40, 0x8,

    /* U+40 "@" */
    0x7a, 0x19, 0xe9, 0x9e, 0x8, 0x3f,

    /* U+41 "A" */
    0x7a, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+42 "B" */
    0xfa, 0x18, 0x7f, 0x86, 0x18, 0x7e,

    /* U+43 "C" */
    0x7a, 0x18, 0x20, 0x82, 0x8, 0x5e,

    /* U+44 "D" */
    0xfa, 0x18, 0x61, 0x86, 0x18, 0x7e,

    /* U+45 "E" */
    0xfe, 0x8, 0x3e, 0x82, 0x8, 0x3f,

    /* U+46 "F" */
    0xfe, 0x8, 0x3e, 0x82, 0x8, 0x20,

    /* U+47 "G" */
    0x7a, 0x18, 0x23, 0x86, 0x18, 0x5e,

    /* U+48 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+49 "I" */
    0xe9, 0x24, 0x97,

    /* U+4A "J" */
    0x4, 0x10, 0x41, 0x4, 0x18, 0x5e,

    /* U+4B "K" */
    0x86, 0x6b, 0x38, 0xe2, 0xc9, 0xa1,

    /* U+4C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x3f,

    /* U+4D "M" */
    0x87, 0x3b, 0x61, 0x86, 0x18, 0x61,

    /* U+4E "N" */
    0x87, 0x9b, 0x67, 0x8e, 0x18, 0x61,

    /* U+4F "O" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+50 "P" */
    0xfa, 0x18, 0x7e, 0x82, 0x8, 0x20,

    /* U+51 "Q" */
    0x7a, 0x18, 0x61, 0x86, 0x79, 0x9d,

    /* U+52 "R" */
    0xfa, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+53 "S" */
    0xfa, 0x18, 0x3e, 0x4, 0x18, 0x5e,

    /* U+54 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x84,

    /* U+55 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+56 "V" */
    0x86, 0x18, 0x61, 0x87, 0x37, 0x8c,

    /* U+57 "W" */
    0x83, 0x6, 0xc, 0x99, 0x32, 0x64, 0xbe,

    /* U+58 "X" */
    0x86, 0x14, 0x8c, 0x31, 0x28, 0x61,

    /* U+59 "Y" */
    0x83, 0x5, 0x11, 0xc1, 0x2, 0x4, 0x8,

    /* U+5A "Z" */
    0xfc, 0x10, 0x84, 0x21, 0x8, 0x3f,

    /* U+5B "[" */
    0xea, 0xab,

    /* U+5C "\\" */
    0x82, 0x4, 0x8, 0x10, 0x20, 0x41,

    /* U+5D "]" */
    0xd5, 0x57,

    /* U+5E "^" */
    0x1, 0xcd, 0xa3,

    /* U+5F "_" */
    0xfc,

    /* U+60 "`" */
    0x19, 0x80,

    /* U+61 "a" */
    0x78, 0x17, 0xe1, 0x85, 0xf0,

    /* U+62 "b" */
    0x82, 0xf, 0xa1, 0x86, 0x18, 0x7e,

    /* U+63 "c" */
    0x7a, 0x18, 0x20, 0x85, 0xe0,

    /* U+64 "d" */
    0x4, 0x17, 0xe1, 0x86, 0x18, 0x5f,

    /* U+65 "e" */
    0x7a, 0x1f, 0xe0, 0x83, 0xe0,

    /* U+66 "f" */
    0x74, 0xe4, 0x44, 0x44,

    /* U+67 "g" */
    0xfe, 0x18, 0x5f, 0x4, 0x17, 0xc0,

    /* U+68 "h" */
    0x82, 0xf, 0xa1, 0x86, 0x18, 0x61,

    /* U+69 "i" */
    0x43, 0x24, 0x97,

    /* U+6A "j" */
    0xc, 0x1, 0x82, 0x8, 0x20, 0x92, 0x70,

    /* U+6B "k" */
    0x84, 0x27, 0x6e, 0x72, 0xd3,

    /* U+6C "l" */
    0xc9, 0x24, 0x97,

    /* U+6D "m" */
    0xfd, 0x26, 0x4c, 0x99, 0x32, 0x40,

    /* U+6E "n" */
    0xfa, 0x18, 0x61, 0x86, 0x10,

    /* U+6F "o" */
    0x7a, 0x18, 0x61, 0x85, 0xe0,

    /* U+70 "p" */
    0xfa, 0x18, 0x7e, 0x82, 0x8, 0x0,

    /* U+71 "q" */
    0xfe, 0x18, 0x5f, 0x4, 0x10, 0x40,

    /* U+72 "r" */
    0xf4, 0x61, 0x8, 0x40,

    /* U+73 "s" */
    0xfe, 0xf, 0x81, 0x7, 0xe0,

    /* U+74 "t" */
    0x44, 0xf4, 0x44, 0x43,

    /* U+75 "u" */
    0x86, 0x18, 0x61, 0x85, 0xf0,

    /* U+76 "v" */
    0x86, 0x18, 0x73, 0x78, 0xc0,

    /* U+77 "w" */
    0x83, 0x26, 0x4c, 0x99, 0x2f, 0x80,

    /* U+78 "x" */
    0x8f, 0x67, 0x1c, 0xda, 0x30,

    /* U+79 "y" */
    0x86, 0x18, 0x5f, 0x4, 0x17, 0xc0,

    /* U+7A "z" */
    0xfc, 0x63, 0x18, 0xc3, 0xf0,

    /* U+7B "{" */
    0x69, 0x64, 0x91,

    /* U+7C "|" */
    0xff,

    /* U+7D "}" */
    0xc9, 0x34, 0x94,

    /* U+7E "~" */
    0x60, 0xa9, 0x50, 0x60,

    /* U+7F "" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 119, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 119, .box_w = 4, .box_h = 2, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 4, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 10, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 119, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 119, .box_w = 1, .box_h = 2, .ofs_x = 3, .ofs_y = 6},
    {.bitmap_index = 29, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 38, .adv_w = 119, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 42, .adv_w = 119, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 43, .adv_w = 119, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 44, .adv_w = 119, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 119, .box_w = 2, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 112, .adv_w = 119, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 119, .box_w = 6, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 120, .adv_w = 119, .box_w = 5, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 119, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 119, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 119, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 304, .adv_w = 119, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 305, .adv_w = 119, .box_w = 3, .box_h = 3, .ofs_x = 3, .ofs_y = 6},
    {.bitmap_index = 307, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 119, .box_w = 4, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 344, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 119, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 360, .adv_w = 119, .box_w = 5, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 119, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 390, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 396, .adv_w = 119, .box_w = 5, .box_h = 6, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 119, .box_w = 4, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 414, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 119, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 436, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 119, .box_w = 1, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 119, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 119, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 452, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t whitrabt_16 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if WHITRABT_16*/

