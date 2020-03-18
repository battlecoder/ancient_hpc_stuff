/*
 * GAPI implementation for HP Jornada 720
 * Copyright (C) 2002 Wild West Software
 */

#include <windows.h>

#define GXDLL_EXPORTS 1
#include <gx.h>

#include <liblcd.h>


#define GX_MIN(x,y) ((x) < (y) ? (x) : (y))


static void* video_buffer = NULL;
static BltData blt;

static const wchar_t j720_id[] = _T("HP, Jornada 7");

struct GXOptions {
	union {
		int flags;
		struct {
			int gx_wait_vsync : 1;
			int gx_qvga       : 1;
			int gx_direct_mem : 1;
		};
	};
	GXOptions() {flags = 0;}
};

static GXOptions options;

void GXGetOptions() {
	/*
	Uncommented...
					Elias Zacarias
	*/
	options.gx_wait_vsync = 1;
	options.gx_qvga = 0;
	options.gx_direct_mem = 1;
}

int GXDetectHardware() {
	int res = 0;

	wchar_t oeminfo[256];
	::memset(oeminfo, 0, sizeof(oeminfo));
	::SystemParametersInfo(SPI_GETOEMINFO, sizeof(oeminfo), &oeminfo, 0);
	res = ::wcslen(oeminfo);
	res = (res && res >= sizeof(j720_id) / sizeof(wchar_t));
	if (res) {
		wchar_t id[sizeof(j720_id) / sizeof(wchar_t)];
		::memcpy(id, oeminfo, sizeof(id));
		id[sizeof(id)/sizeof(wchar_t) - 1] = wchar_t(0);
		res = (::wcscmp(id, j720_id) == 0);
	}

	return res;
}


int GXOpenDisplay(HWND hWnd, DWORD dwFlags) {
	GXGetOptions();
	int res = GXDetectHardware();
	if (res) res = epson_init();
	if (res) {
		int w = epson_get_w();
		int h = epson_get_h();
		if (options.gx_qvga) {
			w = GX_MIN(w, 320);
			h = GX_MIN(h, 240);
		}

		if (video_buffer != NULL) delete [] video_buffer;
		video_buffer = (void*) new short[w*h];
		res = (video_buffer != NULL);

		BltRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.width = w;
		rect.height = h;

		blt.dest = rect;
		blt.attrs.all = 0;
		blt.ROP = ROP_S;
		blt.mem = video_buffer;

		if (res) {
			::memset(video_buffer, 0, w*h*sizeof(short));
			if (dwFlags == GX_FULLSCREEN) {
				if (::IsWindow(hWnd)) {
					::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rect.width, rect.height, SWP_SHOWWINDOW);
					::ShowWindow(hWnd, SW_SHOW);
					::UpdateWindow(hWnd);
				}
				HWND hdt = ::FindWindow(NULL, _T("Desktop"));
				if (hdt) {
					::ShowWindow(hdt, SW_HIDE);
				}
				epson_blt_write(&blt);
			}
		}
	}
	return res;
}

int GXCloseDisplay() {
	epson_reset();
	if (video_buffer) delete [] video_buffer;
	HWND hdt = ::FindWindow(NULL, _T("Desktop"));
	if (hdt) {
		::ShowWindow(hdt, SW_SHOW);
	}
	return 1;
}

void* GXBeginDraw() {
	

	/*
	HACK FOR FORCING THE SCREEN IN PDOSBOX (n0p's port) TO
	START IN THE LEFT EDGE OF THE SCREEN NOT IN THE CENTER 
											Elias Zacarias

	int offs = (options.gx_qvga ? 319*sizeof(short) : 0);
	*/
	int offs = (options.gx_qvga ? 319*sizeof(short) : -160*(int)sizeof(short));

	if (options.gx_direct_mem) {
		return (void*)((DWORD)epson_get_membase()+offs);
	} else {
		return (void*)((DWORD)video_buffer+offs);
	}
}

int GXEndDraw() {
	if (!options.gx_direct_mem) {
		if (options.gx_wait_vsync) epson_vsync();
		epson_blt_write(&blt);
	}
	return 1;
}

int GXOpenInput() {
	return 1;
}

int GXCloseInput() {
	return 1;
}

GXDisplayProperties GXGetDisplayProperties() {
	GXGetOptions();
	GXDisplayProperties dp;
	::memset(&dp, 0, sizeof(GXDisplayProperties));
	if (!GXDetectHardware()) return dp;

	epson_init(); /* it's safe to call init() multiple times */
	int w = epson_get_w();
	int h = epson_get_h();
	if (options.gx_qvga) {
		w = GX_MIN(h, 240);
		h = GX_MIN(epson_get_w(), 320);
	}
	dp.cxWidth = w;
	dp.cyHeight = h;
	dp.cBPP = epson_get_bpp();
	if (options.gx_qvga) {
		dp.cbxPitch = sizeof(short) * (options.gx_direct_mem ? epson_get_w() : h);
		dp.cbyPitch = -(int)sizeof(short);
	} else {
		dp.cbxPitch = sizeof(short);
		dp.cbyPitch = sizeof(short)*w;
	}
	dp.ffFormat = (kfDirect565 | kfDirect);
	if (options.gx_qvga) dp.ffFormat |= kfLandscape;
	return dp;
}

GXKeyList GXGetDefaultKeys(int iOptions) {
	GXKeyList kl;
	::memset(&kl, 0, sizeof(GXKeyList));

	kl.vkLeft  = VK_LEFT;
	kl.vkRight = VK_RIGHT;
	kl.vkUp    = VK_UP;
	kl.vkDown  = VK_DOWN;
	kl.vkStart = VK_RETURN;
	kl.vkA     = 'A';
	kl.vkB     = 'B';
	kl.vkC     = 'C';

	return kl;
}

int GXSuspend() {
	return 0;
}

int GXResume() {
	return 0;
}

int GXSetViewport(DWORD dwTop, DWORD dwHeight, DWORD dwReserved1, DWORD dwReserved2) {
	return 0;
}

BOOL GXIsDisplayDRAMBuffer() {
	return TRUE;
}


extern "C"
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID pvReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}

