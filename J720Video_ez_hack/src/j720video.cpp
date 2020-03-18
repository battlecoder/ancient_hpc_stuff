#include <windows.h>

#include <gx.h>
#include <s1d13506.h>


void main(int argc, char* argv[]) {
	int res = epson_init();
	if (res) {
		int rev = epson_get_revision();
		int bpl = epson_get_bpl();
		int memt = epson_get_mem_type();
		int pt = epson_get_panel_type();
		int h = epson_get_h();
		void* mem = epson_get_membase();
		int col = epson_is_color();
		int dual = epson_is_dual();
		int bpp = epson_get_bpp();
		int dith = epson_get_dithering();
		int daddr = epson_get_display_addr();
		int msize = epson_get_mem_size();
	}
	epson_reset();
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow) {
	int argc = 2;
	char* argv[2];
	int len = _tcslen(lpCmdLine);

	argv[1] = (char*)malloc(len+1);
	WideCharToMultiByte(CP_ACP,0, lpCmdLine, len, argv[1], len, NULL, NULL);
	argv[1][len] = 0;

	main(argc, argv);
	return 0;
}
