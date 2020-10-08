#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 24 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#ifndef DIN1451_M_COND_24
#define DIN1451_M_COND_24 1
#endif

#if DIN1451_M_COND_24

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+30 "0" */
    0x4, 0xcf, 0xd6, 0x3, 0xb3, 0x22, 0x4a, 0xc8,
    0x4e, 0xd8, 0x4b, 0x3, 0x82, 0x1, 0x88, 0x7,
    0xff, 0xe4, 0x40, 0x3c, 0xc0, 0xe0, 0x80, 0x72,
    0x13, 0xb6, 0x12, 0x74, 0x64, 0x49, 0x50,

    /* U+31 "1" */
    0x0, 0x3f, 0xf8, 0x1, 0x30, 0x1, 0x2b, 0x10,
    0x4, 0x66, 0xc1, 0x0, 0x26, 0x8, 0x4, 0x40,
    0x1f, 0xff, 0xf0, 0x0,

    /* U+32 "2" */
    0x4, 0xcf, 0xd7, 0x0, 0x15, 0x99, 0x12, 0x18,
    0x2c, 0x1f, 0x70, 0x2c, 0x8, 0x2c, 0xc, 0x48,
    0x4, 0x84, 0x0, 0x20, 0xa, 0xd2, 0x2, 0x2,
    0x0, 0xe5, 0x5, 0x0, 0xea, 0xa, 0x0, 0xc8,
    0x48, 0x40, 0x1b, 0xc3, 0xc0, 0x31, 0xa1, 0x20,
    0x6, 0x90, 0xb0, 0xc, 0x2a, 0x2c, 0x1, 0xa4,
    0x24, 0x3, 0x9c, 0x18, 0x3, 0x38, 0x30, 0x80,
    0x6b, 0x4, 0xcc, 0xc0, 0x20, 0x46, 0x78, 0x40,

    /* U+33 "3" */
    0x5, 0xdf, 0xd6, 0x0, 0x1d, 0x11, 0x52, 0x81,
    0x21, 0x3b, 0x21, 0xc0, 0xc0, 0x80, 0x80, 0xe1,
    0x6c, 0x60, 0x60, 0x13, 0xc8, 0x81, 0x80, 0x7c,
    0xe0, 0xe0, 0x12, 0x65, 0x4, 0x80, 0x44, 0x60,
    0x44, 0x0, 0x9b, 0xdc, 0xa0, 0x3, 0xa4, 0x1c,
    0x3, 0x84, 0x4, 0x3, 0xc2, 0x0, 0xde, 0x0,
    0x84, 0x0, 0x62, 0x1, 0x84, 0x14, 0x14, 0x18,
    0x8, 0x3c, 0x2b, 0x64, 0x24, 0x12, 0x88, 0xaa,
    0x50,

    /* U+34 "4" */
    0x0, 0xc3, 0xf8, 0x1, 0xe7, 0xb, 0x0, 0xf5,
    0x2, 0x80, 0x70, 0x99, 0x88, 0x3, 0x9c, 0x2c,
    0x3, 0xd4, 0xa, 0x1, 0xc2, 0x64, 0x40, 0xe,
    0x70, 0x50, 0xf, 0x50, 0x58, 0x7, 0x9, 0x91,
    0xbd, 0x98, 0x1, 0xc1, 0x43, 0x51, 0xc0, 0x1a,
    0x16, 0x1, 0xc2, 0x82, 0x60, 0x1c, 0xa0, 0x3b,
    0xa5, 0x9, 0x60, 0x8, 0x88, 0x0, 0x21, 0x4f,
    0xfc, 0xa1, 0x6e, 0x1, 0xff, 0xc6,

    /* U+35 "5" */
    0xcf, 0xfe, 0xe0, 0x8, 0xcf, 0x28, 0x5, 0x99,
    0xac, 0x3, 0xff, 0x9e, 0x75, 0x45, 0x0, 0xd8,
    0xaa, 0xa6, 0x0, 0x9b, 0xdc, 0x28, 0x2, 0x80,
    0x90, 0x30, 0x89, 0x10, 0x10, 0x9, 0x18, 0x2,
    0x10, 0x10, 0xf, 0xd3, 0x60, 0x1c, 0x28, 0x81,
    0x0, 0x8, 0x1, 0xc1, 0xc1, 0x40, 0xc2, 0x42,
    0x76, 0x82, 0x40, 0xe8, 0x8a, 0x95, 0x0,

    /* U+36 "6" */
    0x0, 0xa3, 0xdc, 0x3, 0x98, 0x18, 0x3, 0x18,
    0xa0, 0x80, 0x6a, 0xe, 0x0, 0xe7, 0x5, 0x0,
    0xca, 0x2c, 0x1, 0xde, 0x14, 0x1, 0x85, 0x43,
    0xbe, 0x40, 0x8, 0x0, 0x61, 0x67, 0xc, 0x9,
    0xbd, 0xa, 0x5, 0x4, 0x3, 0x2, 0x1, 0x2,
    0x0, 0x8, 0x0, 0xc0, 0x3f, 0xf8, 0x66, 0x4,
    0x0, 0x10, 0x3, 0x82, 0x82, 0x1, 0x84, 0x84,
    0x6d, 0x85, 0x1, 0xd1, 0x10, 0x61, 0x80,

    /* U+37 "7" */
    0xf, 0xff, 0x88, 0x2, 0x33, 0x80, 0x6, 0x0,
    0x2c, 0xc8, 0x4, 0x40, 0x1c, 0x80, 0x80, 0x8,
    0xb0, 0x6, 0x87, 0x80, 0x1d, 0x80, 0x8, 0x8,
    0x1, 0xc4, 0x43, 0x10, 0xe, 0x40, 0x40, 0xf,
    0x68, 0x58, 0x7, 0x90, 0xc, 0x3, 0x90, 0x10,
    0x3, 0xd8, 0x1a, 0x1, 0xe5, 0x4, 0x0, 0xe1,
    0x22, 0x80, 0x39, 0x1, 0x0, 0x3d, 0xe1, 0x80,
    0x1e, 0x40, 0x50, 0xe, 0x31, 0x30, 0xe,

    /* U+38 "8" */
    0x4, 0xcf, 0xd6, 0x0, 0x1d, 0x99, 0x12, 0x54,
    0x24, 0x23, 0x68, 0x24, 0x18, 0x10, 0x10, 0x8,
    0x2, 0x10, 0x10, 0x9, 0xc0, 0x40, 0x23, 0x3,
    0x5, 0x4, 0x7, 0xa, 0x19, 0xbb, 0x4, 0x81,
    0x10, 0x10, 0x8, 0x80, 0xe6, 0xdf, 0x5, 0x1,
    0x61, 0x60, 0xe0, 0xe0, 0x40, 0x40, 0x1, 0x10,
    0x7, 0xff, 0x14, 0xc0, 0x80, 0x2, 0x0, 0x70,
    0x50, 0x50, 0x20, 0x90, 0x8d, 0xa0, 0xb0, 0x3b,
    0x32, 0x24, 0xb0, 0x0,

    /* U+39 "9" */
    0x5, 0xdf, 0xd6, 0x0, 0x1d, 0x11, 0x52, 0xa1,
    0x21, 0x1b, 0x61, 0x20, 0xe0, 0xa0, 0x80, 0x60,
    0x60, 0x60, 0x1, 0x0, 0xc2, 0x1, 0xff, 0xc1,
    0x30, 0x20, 0x0, 0x88, 0x18, 0x10, 0xc, 0x8,
    0x38, 0x26, 0xf4, 0x10, 0x16, 0x5, 0x40, 0x16,
    0x0, 0x7e, 0xf1, 0x23, 0x0, 0xcc, 0x14, 0x1,
    0xd4, 0xc, 0x1, 0x8c, 0x90, 0x40, 0x35, 0x6,
    0x80, 0x73, 0x83, 0x0, 0x65, 0x15, 0x0, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 146, .box_w = 7, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 146, .box_w = 6, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 146, .box_w = 9, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 146, .box_w = 9, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 491, .adv_w = 146, .box_w = 8, .box_h = 18, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
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
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 1
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t DIN1451_m_cond_24 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -3,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if DIN1451_M_COND_24*/