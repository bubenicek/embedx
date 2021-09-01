#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef WHITRABT_10
#define WHITRABT_10 1
#endif

#if WHITRABT_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xff, 0x3c,

    /* U+22 "\"" */
    0xb4,

    /* U+23 "#" */
    0x52, 0xbe, 0xaf, 0xa9, 0x40,

    /* U+24 "$" */
    0x27, 0xe9, 0xf2, 0xfc, 0x80,

    /* U+25 "%" */
    0xce, 0x44, 0x44, 0x4e, 0x60,

    /* U+26 "&" */
    0xe5, 0x28, 0x8a, 0xcb, 0xa0,

    /* U+27 "'" */
    0xc0,

    /* U+28 "(" */
    0xea, 0xac,

    /* U+29 ")" */
    0xd5, 0x5c,

    /* U+2A "*" */
    0x51, 0xbe, 0x65, 0x0,

    /* U+2B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+2C "," */
    0xf8,

    /* U+2D "-" */
    0xf8,

    /* U+2E "." */
    0xf0,

    /* U+2F "/" */
    0x8, 0x44, 0x44, 0x42, 0x0,

    /* U+30 "0" */
    0xfc, 0x67, 0x5c, 0xc7, 0xe0,

    /* U+31 "1" */
    0x59, 0x24, 0xb8,

    /* U+32 "2" */
    0xfc, 0x42, 0x22, 0x23, 0xe0,

    /* U+33 "3" */
    0xfc, 0x42, 0x70, 0xc7, 0xe0,

    /* U+34 "4" */
    0x14, 0xa5, 0x2f, 0x88, 0x40,

    /* U+35 "5" */
    0xfc, 0x21, 0xf0, 0x87, 0xe0,

    /* U+36 "6" */
    0xf4, 0x21, 0xf8, 0xc7, 0xe0,

    /* U+37 "7" */
    0xf8, 0x42, 0x22, 0x22, 0x0,

    /* U+38 "8" */
    0xfc, 0x63, 0xf8, 0xc7, 0xe0,

    /* U+39 "9" */
    0xfc, 0x63, 0xf0, 0x87, 0xe0,

    /* U+3A ":" */
    0xf0, 0xf0,

    /* U+3B ";" */
    0xf0, 0xf8,

    /* U+3C "<" */
    0x13, 0x6c, 0x63, 0x10,

    /* U+3D "=" */
    0xf8, 0x3e,

    /* U+3E ">" */
    0xc6, 0x21, 0x26, 0xc0,

    /* U+3F "?" */
    0xfc, 0x42, 0x22, 0x0, 0x80,

    /* U+40 "@" */
    0xfc, 0x6f, 0x5b, 0xc3, 0xe0,

    /* U+41 "A" */
    0xfc, 0x63, 0xf8, 0xc6, 0x20,

    /* U+42 "B" */
    0xfc, 0x63, 0xf8, 0xc7, 0xe0,

    /* U+43 "C" */
    0xfc, 0x61, 0x8, 0x47, 0xe0,

    /* U+44 "D" */
    0xfc, 0x63, 0x18, 0xc7, 0xe0,

    /* U+45 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+46 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+47 "G" */
    0xfc, 0x61, 0x38, 0xc7, 0xe0,

    /* U+48 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+49 "I" */
    0xe9, 0x24, 0xb8,

    /* U+4A "J" */
    0x8, 0x42, 0x10, 0xc7, 0xe0,

    /* U+4B "K" */
    0x8c, 0xa9, 0x8a, 0x4a, 0x20,

    /* U+4C "L" */
    0x84, 0x21, 0x8, 0x43, 0xe0,

    /* U+4D "M" */
    0x8e, 0xeb, 0x18, 0xc6, 0x20,

    /* U+4E "N" */
    0x8e, 0x6b, 0x38, 0xc6, 0x20,

    /* U+4F "O" */
    0xfc, 0x63, 0x18, 0xc7, 0xe0,

    /* U+50 "P" */
    0xfc, 0x63, 0xf8, 0x42, 0x0,

    /* U+51 "Q" */
    0xfc, 0x63, 0x1a, 0xcb, 0xa0,

    /* U+52 "R" */
    0xfc, 0x63, 0xf8, 0xc6, 0x20,

    /* U+53 "S" */
    0xfc, 0x61, 0xf0, 0xc7, 0xe0,

    /* U+54 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+55 "U" */
    0x8c, 0x63, 0x18, 0xc7, 0xe0,

    /* U+56 "V" */
    0x8c, 0x63, 0x18, 0xa8, 0x80,

    /* U+57 "W" */
    0x8c, 0x63, 0x5a, 0xd7, 0xe0,

    /* U+58 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+59 "Y" */
    0x8c, 0x54, 0x42, 0x10, 0x80,

    /* U+5A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+5B "[" */
    0xea, 0xac,

    /* U+5C "\\" */
    0x84, 0x10, 0x41, 0x4, 0x20,

    /* U+5D "]" */
    0xd5, 0x5c,

    /* U+5E "^" */
    0x33, 0xb2,

    /* U+5F "_" */
    0xf8,

    /* U+60 "`" */
    0xd0,

    /* U+61 "a" */
    0x78, 0x7f, 0x1f, 0x80,

    /* U+62 "b" */
    0x84, 0x3f, 0x18, 0xc7, 0xe0,

    /* U+63 "c" */
    0xfc, 0x61, 0x1f, 0x80,

    /* U+64 "d" */
    0x8, 0x7f, 0x18, 0xc7, 0xe0,

    /* U+65 "e" */
    0xfc, 0x7f, 0xf, 0x0,

    /* U+66 "f" */
    0x74, 0xe4, 0x44, 0x40,

    /* U+67 "g" */
    0xfc, 0x63, 0xf0, 0xbc,

    /* U+68 "h" */
    0x84, 0x3f, 0x18, 0xc6, 0x20,

    /* U+69 "i" */
    0x43, 0x24, 0xb8,

    /* U+6A "j" */
    0x10, 0x31, 0x11, 0x9f,

    /* U+6B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+6C "l" */
    0xc9, 0x24, 0xb8,

    /* U+6D "m" */
    0xfd, 0x6b, 0x5a, 0x80,

    /* U+6E "n" */
    0xfc, 0x63, 0x18, 0x80,

    /* U+6F "o" */
    0xfc, 0x63, 0x1f, 0x80,

    /* U+70 "p" */
    0xfc, 0x63, 0xf8, 0x40,

    /* U+71 "q" */
    0xfc, 0x63, 0xf0, 0x84,

    /* U+72 "r" */
    0xf9, 0x88, 0x80,

    /* U+73 "s" */
    0xfc, 0x3e, 0x1f, 0x80,

    /* U+74 "t" */
    0x44, 0xf4, 0x44, 0x70,

    /* U+75 "u" */
    0x8c, 0x63, 0x1f, 0x80,

    /* U+76 "v" */
    0x8c, 0x62, 0xa2, 0x0,

    /* U+77 "w" */
    0x8d, 0x6b, 0x5f, 0x80,

    /* U+78 "x" */
    0xcb, 0x8c, 0xec, 0x80,

    /* U+79 "y" */
    0x8c, 0x63, 0xf0, 0xbc,

    /* U+7A "z" */
    0xf8, 0x8c, 0xcf, 0x80,

    /* U+7B "{" */
    0x69, 0x64, 0x98,

    /* U+7C "|" */
    0xfe,

    /* U+7D "}" */
    0xc9, 0x34, 0xb0,

    /* U+7E "~" */
    0xe5, 0x4e,

    /* U+7F "" */
    0xff, 0xff, 0xff, 0xff, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 91, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 91, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 4, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 24, .adv_w = 91, .box_w = 1, .box_h = 2, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 25, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 33, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 37, .adv_w = 91, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 38, .adv_w = 91, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 39, .adv_w = 91, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 53, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 91, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 97, .adv_w = 91, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 91, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 103, .adv_w = 91, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 91, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 91, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 256, .adv_w = 91, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 257, .adv_w = 91, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 258, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 91, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 91, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 288, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 91, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 300, .adv_w = 91, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 91, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 323, .adv_w = 91, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 327, .adv_w = 91, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 91, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 91, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 358, .adv_w = 91, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 91, .box_w = 1, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 91, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 91, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 371, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0}
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
lv_font_t whitrabt_10 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 8,          /*The maximum line height required by the font*/
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



#endif /*#if WHITRABT_10*/

