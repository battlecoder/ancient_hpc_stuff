/*
 * Epson S1D13506 (aka SED1356) access library for HP Jornada 720
 * Copyright (C) 2002 Wild West Software
 *
 * References: http://www.erd.epson.com/vdc/pdf/1356/TM/s1d13506tm.pdf
 */

#ifndef __S1D13506_H__
#define __S1D13506_H__ 1

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BltData BltData;
typedef struct _BltRect BltRect;

enum {
	REV_S1D13506_0       = 0,
	REV_S1D13506_1       = 1,
	REV_S1D13506_UNKNOWN = 0xFFFFFFFF
};

enum {
	DRAM_EDO_CAS = 0,
	DRAM_FPM_CAS = 1,
	DRAM_EDI_WE  = 2,
	DRAM_FPM_WE  = 3
};

enum {
	BPP_4  = 2,
	BPP_8  = 3,
	BPP_15 = 4,
	BPP_16 = 5,
	BPP_X  = 7
};

enum {
	REG_REVISION_CODE             = 0x00,
	REG_MISC                      = 0x01,
	REG_GPIO_CONFIG               = 0x04,
	REG_GPIO_CTRL                 = 0x08,
	REG_MD_CFG_STATUS0            = 0x0c,
	REG_MD_CFG_STATUS1            = 0x0d,
	REG_MEM_CLOCK_CFG             = 0x10,
	REG_LCD_PCLK_CFG              = 0x14,
	REG_CRTTV_PCLK_CFG            = 0x18,
	REG_MPCLK_CFG                 = 0x1c,
	REG_CPU2MEM_WAIT_SEL          = 0x1e,
	REG_MEM_CFG                   = 0x20,
	REG_DRAM_REFRESH              = 0x21,
	REG_DRAM_TIMINGS_CTRL0        = 0x2a,
	REG_DRAM_TIMINGS_CTRL1        = 0x2b,
	REG_PANEL_TYPE                = 0x30,
	REG_MOD_RATE                  = 0x31,
	REG_LCD_HDP                   = 0x32,
	REG_LCD_HNDP                  = 0x34,
	REG_TFT_FPLINE_START          = 0x35,
	REG_TFT_FPLINE_PULSE          = 0x36,
	REG_LCD_VDP0                  = 0x38,
	REG_LCD_VDP1                  = 0x39,
	REG_LCD_VNDP                  = 0x3a,
	REG_TFT_FPFRAME_START         = 0x3b,
	REG_TFT_FPFRAME_PULSE         = 0x3c,
	REG_LCD_LINE_COUNT0           = 0x3e,
	REG_LCD_LINE_COUNT1           = 0x3f,
	REG_LCD_DISPLAY_MODE          = 0x40,
	REG_LCD_MISC                  = 0x41,
	REG_LCD_START_ADDR0           = 0x42,
	REG_LCD_START_ADDR1           = 0x43,
	REG_LCD_START_ADDR2           = 0x44,
	REG_LCD_MEM_ADDR_OFFSET0      = 0x46,
	REG_LCD_MEM_ADDR_OFFSET1      = 0x47,
	REG_LCD_PIXEL_PANNING         = 0x48,
	REG_LCD_FIFO_HIGH_THRESHOLD   = 0x4a,
	REG_LCD_FIFO_LOW_THRESHOLD    = 0x4b,
	REG_CRTTV_HDP                 = 0x50,
	REG_CRTTV_HNDP                = 0x52,
	REG_CRTTV_HRTC_START          = 0x53,
	REG_CRT_HRTC_PULSE            = 0x54,
	REG_CRTTV_VDP0                = 0x56,
	REG_CRTTV_VDP1                = 0x57,
	REG_CRTTV_VNDP                = 0x58,
	REG_CRTTV_VRTC_START          = 0x59,
	REG_CRT_VRTC_PULSE            = 0x5a,
	REG_TV_OUTPUT_CTRL            = 0x5b,
	REG_CRTTV_LINE_COUNT0         = 0x5e,
	REG_CRTTV_LINE_COUNT1         = 0x5f,
	REG_CRTTV_DISPLAY_MODE        = 0x60,
	REG_CRTTV_START_ADDR0         = 0x62,
	REG_CRTTV_START_ADDR1         = 0x63,
	REG_CRTTV_START_ADDR2         = 0x64,
	REG_CRTTV_MEM_ADDR_OFFSET0    = 0x66,
	REG_CRTTV_MEM_ADDR_OFFSET1    = 0x67,
	REG_CRTTV_PIXEL_PANNING       = 0x68,
	REG_CRTTV_FIFO_HIGH_THRESHOLD = 0x6a,
	REG_CRTTV_FIFO_LOW_THRESHOLD  = 0x6b,
	REG_LCD_INK_CURS_CTRL         = 0x70,
	REG_LCD_INK_CURS_START_ADDR   = 0x71,
	REG_LCD_CURSOR_X_POS0         = 0x72,
	REG_LCD_CURSOR_X_POS1         = 0x73,
	REG_LCD_CURSOR_Y_POS0         = 0x74,
	REG_LCD_CURSOR_Y_POS1         = 0x75,
	REG_LCD_INK_CURS_BLUE0        = 0x76,
	REG_LCD_INK_CURS_GREEN0       = 0x77,
	REG_LCD_INK_CURS_RED0         = 0x78,
	REG_LCD_INK_CURS_BLUE1        = 0x7a,
	REG_LCD_INK_CURS_GREEN1       = 0x7b,
	REG_LCD_INK_CURS_RED1         = 0x7c,
	REG_LCD_INK_CURS_FIFO         = 0x7e,
	REG_CRTTV_INK_CURS_CTRL       = 0x80,
	REG_CRTTV_INK_CURS_START_ADDR = 0x81,
	REG_CRTTV_CURSOR_X_POS0       = 0x82,
	REG_CRTTV_CURSOR_X_POS1       = 0x83,
	REG_CRTTV_CURSOR_Y_POS0       = 0x84,
	REG_CRTTV_CURSOR_Y_POS1       = 0x85,
	REG_CRTTV_INK_CURS_BLUE0      = 0x86,
	REG_CRTTV_INK_CURS_GREEN0     = 0x87,
	REG_CRTTV_INK_CURS_RED0       = 0x88,
	REG_CRTTV_INK_CURS_BLUE1      = 0x8a,
	REG_CRTTV_INK_CURS_GREEN1     = 0x8b,
	REG_CRTTV_INK_CURS_RED1       = 0x8c,
	REG_CRTTV_INK_CURS_FIFO       = 0x8e,
	REG_BITBLT_CTRL0              = 0x100,
	REG_BITBLT_CTRL1              = 0x101,
	REG_BITBLT_ROP_CODE           = 0x102,
	REG_BITBLT_OPERATION          = 0x103,
	REG_BITBLT_SRC_START_ADDR0    = 0x104,
	REG_BITBLT_SRC_START_ADDR1    = 0x105,
	REG_BITBLT_SRC_START_ADDR2    = 0x106,
	REG_BITBLT_DEST_START_ADDR0   = 0x108,
	REG_BITBLT_DEST_START_ADDR1   = 0x109,
	REG_BITBLT_DEST_START_ADDR2   = 0x10a,
	REG_BITBLT_MEM_ADDR_OFFSET0   = 0x10c,
	REG_BITBLT_MEM_ADDR_OFFSET1   = 0x10d,
	REG_BITBLT_WIDTH0             = 0x110,
	REG_BITBLT_WIDTH1             = 0x111,
	REG_BITBLT_HEIGHT0            = 0x112,
	REG_BITBLT_HEIGHT1            = 0x113,
	REG_BITBLT_BACKGND_COLOR0     = 0x114,
	REG_BITBLT_BACKGND_COLOR1     = 0x115,
	REG_BITBLT_FOREGND_COLOR0     = 0x118,
	REG_BITBLT_FOREGND_COLOR1     = 0x119,
	REG_LUT_MODE                  = 0x1e0,
	REG_LUT_ADDR                  = 0x1e2,
	REG_LUT_DATA                  = 0x1e4,
	REG_PWR_SAVE_CFG              = 0x1f0,
	REG_PWR_SAVE_STATUS           = 0x1f1,
	REG_CPU2MEM_WATCHDOG          = 0x1f4,
	REG_DISPLAY_MODE              = 0x1fc,
	REG_MEDIAPLUG_LCMD            = 0x1000,
	REG_MEDIAPLUG_RESERVED_LCMD   = 0x1002,
	REG_MEDIAPLUG_CMD             = 0x1004,
	REG_MEDIAPLUG_RESERVED_CMD    = 0x1006,
	REG_MEDIAPLUG_DATA            = 0x1008,
	REG_MAX = REG_MEDIAPLUG_DATA
};

enum BltROPCode {
	/* Write/Move blits */
	ROP_BLACKNESS       = 0,
	ROP_NOT_S_AND_NOT_D = 1,
	ROP_S_OR_D_NOT      = ROP_NOT_S_AND_NOT_D,
	ROP_NOT_S_AND_D     = 2,
	ROP_NOT_S           = 3,
	ROP_S_AND_NOT_D     = 4,
	ROP_NOT_D           = 5,
	ROP_S_XOR_D         = 6,
	ROP_NOT_S_OR_NOT_D  = 7,
	ROP_S_AND_D_NOT     = ROP_NOT_S_OR_NOT_D,
	ROP_S_AND_D         = 8,
	ROP_S_XOR_D_NOT     = 9,
	ROP_D               = 10,
	ROP_NOT_S_OR_D      = 11,
	ROP_S               = 12,
	ROP_S_OR_NOT_D      = 13,
	ROP_S_OR_D          = 14,
	ROP_WHITENESS       = 15,

	/* Pattern fill */
	ROP_PAT_BLACKNESS       = 0,
	ROP_PAT_NOT_P_AND_NOT_D = 1,
	ROP_PAT_P_OR_D_NOT      = ROP_PAT_NOT_P_AND_NOT_D,
	ROP_PAT_NOT_P_AND_D     = 2,
	ROP_PAT_NOT_P           = 3,
	ROP_PAT_P_AND_NOT_D     = 4,
	ROP_PAT_NOT_D           = 5,
	ROP_PAT_P_XOR_D         = 6,
	ROP_PAT_NOT_P_OR_NOT_D  = 7,
	ROP_PAT_P_AND_D_NOT     = ROP_PAT_NOT_P_OR_NOT_D,
	ROP_PAT_P_AND_D         = 8,
	ROP_PAT_P_XOR_D_NOT     = 9,
	ROP_PAT_D               = 10,
	ROP_PAT_NOT_P_OR_D      = 11,
	ROP_PAT_P               = 12,
	ROP_PAT_P_OR_NOT_D      = 13,
	ROP_PAT_P_OR_D          = 14,
	ROP_PAT_WHITENESS       = 15,
};

/* BitBLT operations; see table 10-2, page 64 (298) */
enum {
	BLT_OP_WRITE_ROP              = 0,  /* write blit with ROP */
	BLT_OP_READ                   = 1,  /* read blit */
	BLT_OP_MOVE_ROP               = 2,  /* move blit in positive direction with ROP */
	BLT_OP_MOVE_NEG_ROP           = 3,  /* move blit in negative direction with ROP */
	BLT_OP_WRITE_TRANSPARENT      = 4,  /* transparent write blit */
	BLT_OP_MOVE_TRANSPARENT       = 5,  /* transparent move blit in positive direction */
	BLT_OP_PAT_FILL_ROP           = 6,  /* pattern fill with ROP */
	BLT_OP_PAT_FILL_TRANSPARENT   = 7,  /* pattern fill with transparency */
	BLT_OP_COLORX                 = 8,  /* color expansion */
	BLT_OP_COLORX_TRANSPARENT     = 9,  /* color expansion with transparency */
	BLT_OP_MOVE_COLORX            = 10, /* move blit with color expansion */
	BLT_OP_MOV_COLORX_TRANSPARENT = 11, /* move blit with color expansion and transparency */
	BLT_OP_SOLID_FILL             = 12, /* solid fill */
};


int   epson_init(void);
void  epson_reset(void);
void* epson_get_regbase(void);
void* epson_get_membase(void);
void* epson_get_bltbase(void);
int   epson_read_reg(int reg_offs);
int   epson_read_wreg(int reg_offs);
void  epson_write_reg(int reg_offs, int val);
void  epson_write_wreg(int reg_offs, int val);
void  epson_write_dreg(int reg_offs, int val);
int   epson_get_revision(void);
int   epson_get_bpl(void);
int   epson_get_mem_type(void);
int   epson_get_panel_type(void);
int   epson_is_color(void);
int   epson_is_dual();
int   epson_get_w(void);
int   epson_get_h(void);
int   epson_get_bpp(void);
int   epson_get_dithering(void);
int   epson_set_dithering(int dithering);
int   epson_get_display_addr(void);
int   epson_set_display_addr(int addr);
int   epson_get_mem_size();
void  epson_vsync();


void epson_blt_wait_start(void);
void epson_blt_wait_end(void);
void epson_blt_write(BltData* blt);

struct _BltRect {
	int left;
	int top;
	int width;
	int height;
};

struct _BltData {
	BltRect src;
	BltRect dest;
	int     ROP; /* see BltROPCode */
	POINT   pattern_coords;
	union _attrs {
		int all;
		int transparent : 1;
	} attrs;
	int     bg_color;
	int     fg_color;
	void* mem; /* for read/write BLTs */
};

#ifdef __cplusplus
}
#endif

#endif /* __S1D13506_H__ */
