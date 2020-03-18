/*
 * Epson S1D13506 (aka SED1356) access library for HP Jornada 720
 * Copyright (C) 2002 Wild West Software
 *
 * References: http://www.erd.epson.com/vdc/pdf/1356/TM/s1d13506tm.pdf
 */

#include "s1d13506.h"

#define EPSON_REG_BASE 0x48000000
#define EPSON_BLT_BASE EPSON_REG_BASE+0x100000
#define EPSON_MEM_BASE 0x48200000

#define EPSON_WAIT_COUNT 0x1000000

typedef WORD blt_t;

#ifndef HAS_PB_HEADERS
#ifdef __cplusplus
extern "C" {
#endif
	BOOL SetKMode(BOOL fMode);
	BOOL VirtualCopy(LPVOID lpvDest, LPVOID lpvSrc, DWORD cbSize, DWORD fdwProtect);
#ifdef __cplusplus
}
#endif
#endif /* HAS_PB_HEADERS */

#ifndef PAGE_SIZE
#	define PAGE_SIZE 0x1000
#endif


static void* reg_base = NULL;
static void* mem_base = NULL;
static void* blt_base = NULL;

void* map_phys_addr(int phys_addr, int size) {
	void* p = NULL;
	void* virt = NULL;
	int aligned_addr = phys_addr & ~(PAGE_SIZE - 1);
	if (size == 0) size = PAGE_SIZE << 1;
	virt = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
	if (virt != NULL) {
		VirtualCopy(virt, (void*)(aligned_addr >> 8), size, PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE);
		p = (void*)((unsigned)virt + (phys_addr & (PAGE_SIZE - 1)));
	} else {
		VirtualFree(virt, size, MEM_RELEASE);
	}
	return p;
}

int epson_init() {
	if (reg_base == NULL) reg_base = map_phys_addr(EPSON_REG_BASE, 0);
	if (blt_base == NULL) blt_base = map_phys_addr(EPSON_BLT_BASE, 0);
	if (mem_base == NULL) mem_base = map_phys_addr(EPSON_MEM_BASE, 2*1024*1024);
	return (reg_base != NULL && mem_base != NULL);
}

void epson_reset() {
	if (reg_base != NULL) {
		VirtualFree(reg_base, 0, MEM_RELEASE);
		reg_base = NULL;
	}
	if (blt_base != NULL) {
		VirtualFree(blt_base, 0, MEM_RELEASE);
		blt_base = NULL;
	}
	if (mem_base != NULL) {
		VirtualFree(mem_base, 0, MEM_RELEASE);
		mem_base = NULL;
	}
}

void* epson_get_regbase() {
	return reg_base;
}

void* epson_get_membase() {
	return mem_base;
}

void* epson_get_bltbase() {
	return blt_base;
}

int epson_read_reg(int reg_offs) {
	if (reg_base == NULL || (unsigned)reg_offs > REG_MAX) return 0;
	return (unsigned)((volatile BYTE*)reg_base)[reg_offs];
}

int epson_read_wreg(int reg_offs) {
	if (reg_base == NULL || (unsigned)reg_offs > REG_MAX) return 0;
	return (unsigned)*(volatile WORD*)((BYTE*)reg_base + reg_offs);
}

void epson_write_reg(int reg_offs, int val) {
	if (reg_base == NULL || (unsigned)reg_offs > REG_MAX) return;
	((BYTE*)reg_base)[reg_offs] = (BYTE)val;
}

void epson_write_wreg(int reg_offs, int val) {
	if (reg_base == NULL || (unsigned)reg_offs > REG_MAX-1) return;
	if (reg_offs & 1) {
		((BYTE*)reg_base)[reg_offs] = (BYTE)val;
		((BYTE*)reg_base)[reg_offs+1] = (BYTE)(val >> 8);
	} else {
		*(WORD*)((BYTE*)reg_base + reg_offs) = (WORD)val;
	}
}

void epson_write_dreg(int reg_offs, int val) {
	if (reg_base == NULL || (unsigned)reg_offs > REG_MAX) return;
	((BYTE*)reg_base)[reg_offs] = (BYTE)val;
	((BYTE*)reg_base)[reg_offs+1] = (BYTE)(val >> 8);
	((BYTE*)reg_base)[reg_offs+2] = (BYTE)(val >> 16);
	((BYTE*)reg_base)[reg_offs+3] = (BYTE)(val >> 24);
}

int epson_get_revision() {
	int rev = REV_S1D13506_UNKNOWN;
	int r = epson_read_reg(REG_REVISION_CODE);
	if ((r & 0xFC) == 0x10) {
		r &= 0x3;
		rev = (r == 1 ? REV_S1D13506_1 : r == REV_S1D13506_0 ? 0 : REV_S1D13506_UNKNOWN);
	}
	return rev;
}

int epson_get_bpl() {
	return (epson_read_wreg(REG_LCD_MEM_ADDR_OFFSET0) << 1);
}

int epson_get_mem_type() {
	return (epson_read_reg(REG_MEM_CFG) & 0x3);
}


/* See 8.3.6 for panel type bits */
int epson_get_panel_type() {
	return epson_read_reg(REG_PANEL_TYPE);
}

int epson_is_color() {
	return !!(epson_get_panel_type() & 4);
}

int epson_is_dual() {
	return !!(epson_get_panel_type() & 2);
}


int epson_get_w() {
	/* swap w/h if rotated 90 degrees */
	return epson_read_reg(REG_DISPLAY_MODE) & 0x40
	       ? epson_read_wreg(REG_LCD_VDP0) + 1
		   : (epson_read_reg(REG_LCD_HDP) + 1) << 3;
}

int epson_get_h() {
	/* swap w/h if rotated 90 degrees */
	return !(epson_read_reg(REG_DISPLAY_MODE) & 0x40)
	       ? epson_read_wreg(REG_LCD_VDP0) + 1
		   : (epson_read_reg(REG_LCD_HDP) + 1) << 3;
}

/* See 8.3.7; Table 8-20 */
int epson_get_bpp() {
	int mode = epson_read_reg(REG_LCD_DISPLAY_MODE) & 7;
	int bpp = 0;
	switch (mode) {
	case BPP_4:
		bpp = 4;
		break;
	case BPP_8:
		bpp = 8;
		break;
	case BPP_15:
		bpp = 15;
		break;
	case BPP_16:
		bpp = 16;
		break;
	default:
		break;
	}
	return bpp;
}

int epson_get_dithering() {
	/* return the inverse of the Dithering Disable bit */
	return !(epson_read_reg(REG_LCD_MISC) & 2);
}

int epson_set_dithering(int dithering) {
	int old = epson_get_dithering();
	if (dithering != old) {
		int reg = epson_read_reg(REG_LCD_MISC);
		if (dithering) {
			/* Clear Dithering Disable bit */
			epson_write_reg(REG_LCD_MISC, reg & (~2));
		} else {
			epson_write_reg(REG_LCD_MISC, reg | 2);
		}
	}
	return old;
}


int epson_get_display_addr() {
	/* address is 20-bit long */
	int lo = epson_read_wreg(REG_LCD_START_ADDR0);
	int hi = epson_read_reg(REG_LCD_START_ADDR2);
	return (lo | ((hi & 0xF) << 16));
}

int epson_set_display_addr(int addr) {
	int old = epson_get_display_addr();
	if (addr != old) {
		epson_write_dreg(REG_LCD_START_ADDR0, addr);
	}
	return old;
}

int epson_get_mem_size() {
	/* read MD[7:6], see 5.3; table 5-6 */
	int val = (epson_read_wreg(REG_MD_CFG_STATUS0) >> 6) & 3;
	return (val == 0 || val == 2 ? 512 : 2*1024);
}

void epson_vsync() {
	int i;
	for (i = EPSON_WAIT_COUNT; --i >= 0;) {
		if ((epson_read_reg(REG_LCD_VNDP) & 0x80) == 0) break;
	}
	for (i = EPSON_WAIT_COUNT; --i >= 0;) {
		if (epson_read_reg(REG_LCD_VNDP)) break;
	}
}



void epson_blt_wait_start() {
	/* Wait for BitBLT Active Status bit to become clear and blt engine is idle. */
	while (!(epson_read_reg(REG_BITBLT_CTRL0) & 0x80)) {}
}

void epson_blt_wait_end() {
	/* Spin while blt engine is busy. */
	while (epson_read_reg(REG_BITBLT_CTRL0) & 0x80) {}
}

void epson_blt_write(BltData* blt) {
	int bpl, bpp, phase, reg;
	int dest, num_words, count;
	blt_t* blt_mem;
	blt_t* src;
	if (blt == NULL || blt->mem == NULL || blt_base == NULL) return;

	bpl = epson_get_bpl();
	bpp = epson_get_bpp() >> 3; /* bytes per pixel */

	dest = blt->dest.top*bpl + blt->dest.left*bpp;

	blt_mem = (blt_t*)blt_base;
	phase = (int)blt->mem & 1;
	src = (blt_t*)((DWORD)blt->mem & 0xFFFFFFFE);
	num_words = phase + (((blt->dest.width - phase)*bpp + 1) >> 1);
	count = num_words*blt->dest.height;

	epson_blt_wait_end();

	if (blt->attrs.transparent) {
		epson_write_wreg(REG_BITBLT_BACKGND_COLOR0, blt->bg_color);
		epson_write_reg(REG_BITBLT_OPERATION, BLT_OP_MOVE_TRANSPARENT);
	} else {
		epson_write_reg(REG_BITBLT_OPERATION, BLT_OP_WRITE_ROP);
		epson_write_reg(REG_BITBLT_ROP_CODE, blt->ROP);
	}


	/*
	 * From Epson manual:
	 * If data is sourced from the CPU, then bit 0 is used for byte alignment
	 * within a 16-bit word and the other address bits are ignored.
	 */
	epson_write_reg(REG_BITBLT_SRC_START_ADDR0, phase);
	/*
	 * The BitBLT Memory Address Offset Registers form the BitBLTs 11-bit address offset
	 * from the starting word of line “n” to the starting word of line “n + 1”. 
	 */
	epson_write_wreg(REG_BITBLT_MEM_ADDR_OFFSET0, bpl >> 1); /* offset is in words */

	epson_write_reg(REG_BITBLT_DEST_START_ADDR0, dest);
	epson_write_reg(REG_BITBLT_DEST_START_ADDR1, dest >> 8);
	epson_write_reg(REG_BITBLT_DEST_START_ADDR2, dest >> 16);

	epson_write_wreg(REG_BITBLT_WIDTH0, blt->dest.width - 1);
	epson_write_wreg(REG_BITBLT_HEIGHT0, blt->dest.height - 1);

	/* start BLT engine */
	epson_write_reg(REG_BITBLT_CTRL0, 0x80);
	epson_blt_wait_start();

	while (count > 0) {
		reg = epson_read_reg(REG_BITBLT_CTRL0);
		if ((reg & 0x30) == 0x20) {
			/* BitBLT FIFO Full Status == 0 */
			/* BitBLT Half Full Status == 1 */
			/* FIFO is at least half-full - add 1 word */
			*blt_mem = *src++;
			count--;
		} else if ((reg & 0x40) == 0) {
			/* BitBLT Not Empty Status bit is 0 - FIFO is empty */
			if (count >= 16) {
				*blt_mem = *src++; /* 0 */
				*blt_mem = *src++; /* 1 */
				*blt_mem = *src++; /* 2 */
				*blt_mem = *src++; /* 3 */
				*blt_mem = *src++; /* 4 */
				*blt_mem = *src++; /* 5 */
				*blt_mem = *src++; /* 6 */
				*blt_mem = *src++; /* 7 */
				*blt_mem = *src++; /* 8 */
				*blt_mem = *src++; /* 9 */
				*blt_mem = *src++; /* A */
				*blt_mem = *src++; /* B */
				*blt_mem = *src++; /* C */
				*blt_mem = *src++; /* D */
				*blt_mem = *src++; /* E */
				*blt_mem = *src++; /* F */
				count -= 16;
			} else {
				for (;--count >= 0;) {
					*blt_mem = *src++;
				}
			}
		}
		/* FIFO is full, just spin for now */
	}
}

