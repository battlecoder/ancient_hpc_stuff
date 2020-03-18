#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <stdlib.h>
#include <commctrl.h>

#define TRAY_NOTIFYICON WM_USER + 2001
#define ID_TRAY	5000


/* === Constants And Macros =================================================== */
#define WWSHORTCUT			_T("\\Windows\\Startup\\Winwatch.lnk")
#define FIXWINDOW_HK		1
#define SCROLLUP_HK			7
#define SCROLLDWN_HK		9
#define SCROLLRGT_HK		11
#define SCROLLLFT_HK		13
#define BORDER_HK			15
#define HIDETB_HK			17
#define FIT2CONTENT_HK		21
#define CENTER_HK			23
#define RESIZEUP_HK			25
#define RESIZEDWN_HK		27
#define RESIZELFT_HK		29
#define RESIZERGT_HK		31
#define LANDSCAPE_HK		33

/* Toggle-based Hotkeys */
#define SCROLL_TOGGLE_HK	41
#define RESIZE_TOGGLE_HK	42
#define GENERAL_TOGGLE_HK	43

#define ESC_HOTKEY		5
#define WIN_HOTKEY		7
#define UP_HOTKEY		9
#define DOWN_HOTKEY		11
#define RIGHT_HOTKEY	13
#define LEFT_HOTKEY		15
#define FREEHK_KEYAREA	17


#define PMENU_SCROLL1		4
#define PMENU_SCROLL2		5
#define PMENU_SCROLL3		6
#define PMENU_SCROLL5		7
#define PMENU_SCROLL10		8
#define PMENU_SCROLL20		9
#define PMENU_SCROLL40		10
#define PMENU_SCROLL60		11
#define PMENU_TABPROCESS	20
#define PMENU_NOTONLYDLGS	21
#define PMENU_PPCSIZEFIRST	22
#define PMENU_ONLYACTIVE	31
#define PMENU_FIXPOSITION	32
#define PMENU_FIXSIZE		33
#define PMENU_TOGGLE_STATUS	100
#define PMENU_SHOWWINWATCH	101
#define PMENU_EXIT			102

#define INIX1			1
#define INIY1			2
#define INIX2			3
#define INIY2			4
#define INISTARTMIN		5
#define INIONLY2FOCUS	6
#define INIONLYDIALOGS	7
#define INIFIXSIZE		8
#define INIFIXPOS		9
#define INITASKBARUP	10
#define INITABCTLPOST	21
#define INITRYPPCFIRST	22
#define INIUNKNOWN		23
#define INISCROLLHOLD	24
#define INIBORDERKEY	35
#define INIRESIZEHOLD	36
#define INIFITCKEY		28
#define INIFITAKEY		30
#define INICENTERKEY	32
#define INIHIDETBKEY	34
#define INIHOLDKEY		13

/* === Function Prototypes ==================================================== */
BOOL WINAPI MainDlgProc( HWND, UINT, WPARAM, LPARAM );
void GetValuesFromGUI();

/* === Global Variables ======================================================= */

HINSTANCE hInst;	/* App Instance  */
HWND ThisDlg;		/* Dialog Handle */
HWND CurrentParent;	/* Current Parent Window when enumerating childs */
HWND HotkeyDlg;
HMENU PopUp;
HMENU ScrollPopUp;
HMENU FixPopUp;
HMENU FitPopUp;
BOOL WeAreHidden = FALSE;
/* Default options */
BOOL FixXY = TRUE;
BOOL FixWH = TRUE;
BOOL Only2Focus = FALSE;
BOOL StartMin = FALSE;
BOOL MinimizeMe = FALSE;
BOOL TaskBarUp = FALSE;
BOOL TabCtlPostProcess = FALSE;
BOOL TryPPCSize1st = TRUE;
BOOL MoreThanDlgs = TRUE;
WCHAR WinwatchEXE[256];
WCHAR WinwatchINI[256];

int ScrollStep = 20;
int  Now_Editting = -99;	/* Are we currently editting a hotkey? */
UINT ToggledPressed = 0;

long AreaX1=2, AreaY1=2, AreaX2=640,AreaY2=240;
typedef struct{
	long maxx;
	long maxy;
}MAX_VALS;

typedef struct{
	UINT scroll;
	UINT border;
	UINT resize;
	UINT fitC;
	UINT fitA;
	UINT center;
	UINT hidetb;
	UINT landscape;
	UINT hold;
}__KEY_DEFS;
__KEY_DEFS KEY_DEFS;

/* MISC FUNCTIONS ***************************************************************/
RECT old_work_area;
RECT new_work_area;

/********************************************************************************/
/** swap_rect:Swaps two RECT structures.                                       **/
/********************************************************************************/
void swap_rect(RECT *a, RECT *b){
	RECT c = *a;

	*a = *b;
	*b = c;
}
/********************************************************************************/
/** FileExists: Returns TRUE if a given file exists.                           **/
/********************************************************************************/
BOOL FileExists(WCHAR *file){
	FILE *fh;
	fh = _wfopen(file, _T("r"));
	if (!fh) return FALSE;
	fclose(fh);
	return TRUE;
}
/********************************************************************************/
/** AskExit: Ask the user to confirm before quitting.                          **/
/********************************************************************************/
BOOL AskExit(HWND hD){
	int r = MessageBox(hD, _T("Do you really want to Exit WinWatch?"),
					_T("Winwatch Quit Confirmation"), MB_YESNO | MB_ICONQUESTION
						| MB_TOPMOST | MB_SETFOREGROUND);
	return (r == IDYES);
}
/********************************************************************************/
/** DefaultKeyDefs: Restore Default HotKey assignments                         **/
/********************************************************************************/
void DefaultKeyDefs(){
	KEY_DEFS.hold = VK_LWIN;

	KEY_DEFS.border		= 'S';
	KEY_DEFS.resize		= VK_SHIFT;
	KEY_DEFS.scroll		= 0;
	KEY_DEFS.fitC		= 'X';
	KEY_DEFS.fitA		= 'F';
	KEY_DEFS.center		= 'T';
	KEY_DEFS.hidetb		= 'Z';
	KEY_DEFS.landscape	= 'H';
}

/********************************************************************************/
/** IsHoldKey: Returns TRUE if a given Key is one of the 'Holdable' Keys       **/
/********************************************************************************/
BOOL IsHoldKey(UINT vk){
	return (vk==VK_LWIN || vk==VK_RWIN || vk==VK_CONTROL ||
			vk==VK_MENU || vk==VK_SHIFT || vk==0);
}

/********************************************************************************/
/** getHKmod: Returns the Needed modifier for a HotKey based on the 'hold' key **/
/********************************************************************************/
UINT getHKmod(UINT k){
	switch (k){
		case VK_LWIN:
		case VK_RWIN:
			return MOD_WIN;
		case VK_CONTROL:
			return MOD_CONTROL;
		case VK_MENU:
			return MOD_ALT;
		case VK_SHIFT:
			return MOD_SHIFT;
		default:
			/* Hardware Keys (0xc1 - 0xcf) are actually
			WIN + [0xc1 - 0xcf] combinations */
			if (k >= 0xc1 && k <= 0xcf)
				return MOD_WIN;
			else return 0;
	}
}

/********************************************************************************/
/** key2string: Attempts to identify a key and write the description to a str  **/
/********************************************************************************/
WCHAR *key2string(DWORD vk_code, WCHAR *dest){
	switch (vk_code){
		case 0:
			wcscpy(dest, _T("None"));
		break;
		case VK_SHIFT:
			wcscpy(dest, _T("SHIFT"));
		break;
		case VK_CONTROL:
			wcscpy(dest, _T("CTRL"));
		break;
		case VK_MENU:
			wcscpy(dest, _T("ALT"));
		break;
		case VK_LWIN:
		case VK_RWIN:
			wcscpy(dest, _T("WIN"));
			break;
		default:
			if (vk_code >= 'A' && vk_code <= 'Z'){
				dest[0]= (WORD)(0xff & vk_code);
				dest[1] = 0;
			}else if (vk_code >= '0' && vk_code <= '9'){
				dest[0]= (WORD)(0xff & vk_code) + '0';
				dest[1] = 0;
			}else wsprintf(dest,_T("0x%03x"),vk_code);
	}
	return dest;
}
/* TRAY INTERFACING *************************************************************/
/********************************************************************************/
/** TrayMessage: Sends a Message to Syste Tray.                                **/
/********************************************************************************/
BOOL TrayMessage(HWND hwnd, DWORD dwMessage, UINT uID, HICON hIcon, PTSTR pszTip){
	BOOL res = FALSE;
	NOTIFYICONDATA tnd;
  
	tnd.cbSize		= sizeof(NOTIFYICONDATA);
	tnd.hWnd		= hwnd;
	tnd.uID		= uID;
	tnd.uFlags		= NIF_MESSAGE|NIF_ICON;
	tnd.uCallbackMessage	= TRAY_NOTIFYICON;
	tnd.hIcon		= hIcon;
	tnd.szTip[0]		= '\0';

	res = Shell_NotifyIcon(dwMessage, &tnd);
	return res;
}

/********************************************************************************/
/** TrayIconDelete: Deletes an Icon from System Tray.                          **/
/********************************************************************************/
void TrayIconDelete(HWND hwnd, UINT uID, HICON hIcon, PTSTR pszTip){
	TrayMessage(hwnd, NIM_DELETE, uID, hIcon, NULL);
}

/********************************************************************************/
/** TrayIconAdd: Adds a icon to System Traybar.                                **/
/********************************************************************************/
void TrayIconAdd(HWND hwnd, UINT uID, HICON hIcon, PTSTR pszTip){
	TrayMessage(hwnd, NIM_ADD, uID,  hIcon, NULL);
}



/********************************************************************************/
/** DisplayPopUp: Custom way of displaying a popup menu                        **/
/********************************************************************************/
DWORD DisplayPopUp(HMENU hM, int x, int y){
	return TrackPopupMenu(hM,TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RETURNCMD|
						TPM_NONOTIFY,x, y, 0, ThisDlg, NULL);
}


/********************************************************************************/
/** DisplayMenu: Displays the systray menu.                                    **/
/********************************************************************************/
DWORD DisplayMenu(){
	RECT rct;
	DWORD mID;
	HWND TaskBar;

	TaskBar = FindWindow(TEXT("HHTaskBar"),NULL);
	GetWindowRect(TaskBar,&rct);
	mID = DisplayPopUp(PopUp, rct.right, rct.top);
	return mID;
}

/********************************************************************************/
/** MakePopups: Create Popups menus.                                           **/
/********************************************************************************/
void MakePopups(){
	ScrollPopUp = CreatePopupMenu();
	FixPopUp = CreatePopupMenu();
	FitPopUp = CreatePopupMenu();

	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL1, _T("1 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL2, _T("2 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL3, _T("3 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL5, _T("5 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL10, _T("10 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL20, _T("20 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL40, _T("40 px"));
	AppendMenu(ScrollPopUp, MF_STRING, PMENU_SCROLL60, _T("60 px"));

	AppendMenu(FitPopUp, MF_STRING, PMENU_ONLYACTIVE, _T("Only Active"));
	AppendMenu(FitPopUp, MF_STRING, PMENU_FIXPOSITION, _T("Fix Position"));
	AppendMenu(FitPopUp, MF_STRING, PMENU_FIXSIZE, _T("Fix Size"));

	AppendMenu(FixPopUp, MF_STRING, PMENU_NOTONLYDLGS, _T("Not Only Dialogs"));
	AppendMenu(FixPopUp, MF_STRING, PMENU_TABPROCESS, _T("TabCtl Post Process"));
	AppendMenu(FixPopUp, MF_STRING, PMENU_PPCSIZEFIRST, _T("Try PPC Size First"));


	PopUp = CreatePopupMenu();
	AppendMenu(PopUp,MF_POPUP, (UINT)ScrollPopUp,_T("Scroll/Resize"));
	AppendMenu(PopUp,MF_POPUP, (UINT)FitPopUp,_T("Fit To Area"));
	AppendMenu(PopUp,MF_POPUP, (UINT)FixPopUp,_T("Fit To Content (Fix)"));
	AppendMenu(PopUp,MF_SEPARATOR, 0,NULL);

	AppendMenu(PopUp,MF_STRING,PMENU_SHOWWINWATCH,_T("Show Winwatch"));
	AppendMenu(PopUp,MF_SEPARATOR, 0,NULL);

	AppendMenu(PopUp,MF_STRING,PMENU_EXIT,_T("Exit"));
}

/********************************************************************************/
/** UpdatePopups: Update popup menus.                                          **/
/********************************************************************************/
void UpdatePopups(){
	GetValuesFromGUI();
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL1, (ScrollStep==1? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL2, (ScrollStep==2? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL3, (ScrollStep==3? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL5, (ScrollStep==5? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL10, (ScrollStep==10? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL20, (ScrollStep==20? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL40, (ScrollStep==40? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(ScrollPopUp, PMENU_SCROLL60, (ScrollStep==60? MF_CHECKED : MF_UNCHECKED));

	
	CheckMenuItem(FitPopUp, PMENU_FIXPOSITION, (FixXY? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(FitPopUp, PMENU_FIXSIZE, (FixWH? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(FitPopUp, PMENU_ONLYACTIVE, (Only2Focus? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem(FixPopUp, PMENU_TABPROCESS, (TabCtlPostProcess? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(FixPopUp, PMENU_NOTONLYDLGS, (MoreThanDlgs? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(FixPopUp, PMENU_PPCSIZEFIRST, (TryPPCSize1st? MF_CHECKED : MF_UNCHECKED));
	
}
/* DIALOG INTERFACING ***********************************************************/
/********************************************************************************/
/** HideDialog: Hide the Main Dialog, Also set the WeAreHidden flag.           **/
/********************************************************************************/
void HideDialog(){
	ShowWindow(ThisDlg, SW_HIDE);
	if (HotkeyDlg) ShowWindow(HotkeyDlg, SW_HIDE);
	WeAreHidden = TRUE;
}

/********************************************************************************/
/** ShowDialog: Shows the Main Dialog. Also clears the WeAreHidden flag.       **/
/********************************************************************************/
void ShowDialog(){
	ShowWindow(ThisDlg, SW_SHOW);
	if (HotkeyDlg){
		ShowWindow(HotkeyDlg, SW_SHOW);
		SetForegroundWindow(HotkeyDlg);
	}else SetForegroundWindow(ThisDlg);
	WeAreHidden = FALSE;
}

/********************************************************************************/
/** MoveNoResize: Moves a window without resizing it.                          **/
/********************************************************************************/
void MoveNoResize(HWND hwnd, long x, long y){
	SetWindowPos(hwnd,NULL,x,y,0,0,SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOZORDER);
}


/********************************************************************************/
/** MakeSizeable: Force a window to have a sizing border.                      **/
/********************************************************************************/
void MakeSizeable(HWND hwnd){
	LONG wlng;
	wlng = GetWindowLong(hwnd, GWL_STYLE);
	wlng |=  WS_BORDER | WS_SIZEBOX;
	SetWindowLong(hwnd, GWL_STYLE, wlng);
}

/********************************************************************************/
/** CenterWindow. Centers Active Dialog.                                       **/
/********************************************************************************/
void CenterWindow(HWND hwnd){
	RECT rct;
	RECT desk;
	long x, y, w, h;


	GetWindowRect(GetDesktopWindow(), &desk);
	GetWindowRect(hwnd,&rct);
	w = rct.right - rct.left;
	h = rct.bottom - rct.top;
	x = (desk.right - w)/2;
	y = (desk.bottom - h)/2;
	MoveNoResize(hwnd, x, y);
}
/********************************************************************************/
/** ResizeNoMove: Resizes a window without moving it.                          **/
/********************************************************************************/
void ResizeNoMove(HWND hwnd, long dx, long dy){
	RECT rct;
	GetWindowRect(hwnd,&rct);
	SetWindowPos(hwnd,NULL,0,0,rct.right-rct.left+dx,rct.bottom-rct.top+dy,SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
}

/********************************************************************************/
/** FixTabCtrls: Attemps to fix tabbed controls.                               **/
/********************************************************************************/
void FixTabCtrls(HWND hwnd){
	HWND child;
	WCHAR buff[128];
	RECT pr, tabr;

	child = GetWindow(hwnd,GW_CHILD);
	while (child){
		FixTabCtrls(child);
		GetWindowRect(hwnd,&pr);
		pr.right-=6;
		pr.bottom-=6;
		GetClassName(child,(LPTSTR)buff,128);
		if (!wcscmp(buff,_T("SysTabControl32"))) {
			GetWindowRect(child,&tabr);
			TabCtrl_AdjustRect(child, TRUE, &pr);
			ResizeNoMove(child, pr.right-tabr.left,pr.bottom-tabr.top);
		}
		child = GetWindow(child,GW_HWNDNEXT);
	}
}
/********************************************************************************/
/** ScrollNoResize: Scrolls a windows without resizing it.                     **/
/********************************************************************************/
void ScrollNoResize(HWND hwnd, long dx, long dy){
	RECT rct;
	GetWindowRect(hwnd,&rct);
	MoveWindow(hwnd,rct.left+dx,rct.top+dy,rct.right-rct.left,rct.bottom-rct.top,TRUE);
}

/********************************************************************************/
/** MaximizeTopLevel: Callback for maximizing top level Windows.               **/
/********************************************************************************/
BOOL CALLBACK MaximizeTopLevel(HWND hwnd, LPARAM lParam){
	RECT wr;
	int sx = new_work_area.left;
	int sw = new_work_area.right-sx;
	int sy = new_work_area.top;
	int sh = new_work_area.bottom-sy;

	GetWindowRect(hwnd, &wr);
	if (IsWindowVisible(hwnd) && memcmp(&wr, &old_work_area, sizeof(RECT))==0){
		SetWindowPos(hwnd,0,sx,sy,sw,sh,SWP_NOZORDER | SWP_NOACTIVATE);
		
	}
	return TRUE;

}

/********************************************************************************/
/** GetCurrentWorkArea: Sets the old_work_area RECT with current Work Area.    **/
/********************************************************************************/
void GetCurrentWorkArea(){
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&old_work_area, 0);
}

/********************************************************************************/
/** SetWorkArea: Sets the new Work Area Size.                                  **/
/********************************************************************************/
void SetWorkArea(int x, int y, int cx, int cy){
	UINT flg = SWP_NOACTIVATE | SWP_NOREPOSITION | SWP_NOZORDER;

	new_work_area.left = x;
	new_work_area.right = x+cx;
	new_work_area.top = y;
	new_work_area.bottom = y+cy;
	SetWindowPos(GetDesktopWindow(),0,x,y,cx,cy, flg);
	SystemParametersInfo(SPI_SETWORKAREA, 0, (PVOID)&new_work_area,SPIF_SENDCHANGE);
}

#define ADJUST_FULLSCREEN_DIALOGS EnumWindows((WNDENUMPROC)MaximizeTopLevel,0)

/********************************************************************************/
/** HideTaskBar: Shows/Hide the task bar.                                      **/
/********************************************************************************/
void HideTaskBar(){
	HWND tbh, dH;
	HDC dDC;
	int sw, sh;

	tbh = FindWindow(TEXT("HHTaskBar"),NULL);
	dH = GetDesktopWindow();
	dDC = GetDC(dH);
	sw = GetDeviceCaps(dDC, HORZRES);
	sh = GetDeviceCaps(dDC, VERTRES);

	if (IsWindowVisible(tbh)){
		ShowWindow(tbh, SW_HIDE);
		/* Get work area */
		GetCurrentWorkArea();
		/* Set system area and desktop as desired */
		SetWorkArea(0,0,sw,sh);
	}else{
		ShowWindow(tbh, SW_SHOW);
		/* Restore work area and desktop */
		swap_rect(&old_work_area, &new_work_area);
		sw = new_work_area.right - new_work_area.left;
		sh = new_work_area.bottom - new_work_area.top;
		SetWorkArea(new_work_area.left,new_work_area.top,sw,sh);
	}
	ADJUST_FULLSCREEN_DIALOGS;
}


/********************************************************************************/
/** taskbarup: Put the TaskBar in the upper part of the screen.                **/
/********************************************************************************/
void taskbarup(){
	RECT rct;
	HWND hnd;
	long h, w;
	int sx = GetDeviceCaps(GetDC(NULL),HORZRES);
	int sy = GetDeviceCaps(GetDC(NULL),VERTRES);


	hnd = FindWindow(TEXT("HHTaskBar"),NULL);
	if (!IsWindowVisible(hnd)) HideTaskBar();

	GetCurrentWorkArea();
	GetWindowRect(hnd,&rct);
	w = rct.right-rct.left;
	h = rct.bottom-rct.top;
	MoveWindow(hnd,0,0,w,h,TRUE);
	SetWorkArea(0,h,sx,sy-h);
	ADJUST_FULLSCREEN_DIALOGS;
}

/********************************************************************************/
/** taskbardown: Put the TaskBar in the default position.                      **/
/********************************************************************************/
void taskbardown(){
	RECT rct;
	HWND hnd;
	long h, w;
	int sx = GetDeviceCaps(GetDC(NULL),HORZRES);
	int sy = GetDeviceCaps(GetDC(NULL),VERTRES);

	hnd = FindWindow(TEXT("HHTaskBar"),NULL);
	if (!IsWindowVisible(hnd)) HideTaskBar();

	GetCurrentWorkArea();
	GetWindowRect(hnd,&rct);
	w = rct.right-rct.left;
	h = rct.bottom-rct.top;
	MoveWindow(hnd,0,sy-h,w,h,TRUE);
	SetWorkArea(0,0,sx,sy-h);
	ADJUST_FULLSCREEN_DIALOGS;
}

/* WINDOW SCANNING AND FIXING ***************************************************/

/********************************************************************************/
/** RegisterHotKeys: Register ALL Hotkeys.                                     **/
/********************************************************************************/
void RegisterHotkeys(){
	UINT hkmod;
	UINT rhkmod;

	if (IsHoldKey(KEY_DEFS.hold)){
		hkmod = getHKmod(KEY_DEFS.hold);
		RegisterHotKey(ThisDlg, FIXWINDOW_HK, hkmod, KEY_DEFS.fitA);
		RegisterHotKey(ThisDlg, FIT2CONTENT_HK, hkmod, KEY_DEFS.fitC);
		RegisterHotKey(ThisDlg, BORDER_HK, hkmod, KEY_DEFS.border);
		RegisterHotKey(ThisDlg, CENTER_HK, hkmod, KEY_DEFS.center);
		RegisterHotKey(ThisDlg, HIDETB_HK, hkmod, KEY_DEFS.hidetb);
		RegisterHotKey(ThisDlg, LANDSCAPE_HK, hkmod, KEY_DEFS.landscape);
		/* Resize */
		if (IsHoldKey(KEY_DEFS.resize)){
			rhkmod = hkmod | getHKmod(KEY_DEFS.resize);
			RegisterHotKey(ThisDlg, RESIZEUP_HK, rhkmod, VK_UP);
			RegisterHotKey(ThisDlg, RESIZEDWN_HK, rhkmod, VK_DOWN);
			RegisterHotKey(ThisDlg, RESIZELFT_HK, rhkmod, VK_LEFT);
			RegisterHotKey(ThisDlg, RESIZERGT_HK, rhkmod, VK_RIGHT);
		}else{
			RegisterHotKey(ThisDlg, RESIZE_TOGGLE_HK, hkmod, KEY_DEFS.resize);
		}
		/* Scroll */
		if (IsHoldKey(KEY_DEFS.scroll)){
			rhkmod = hkmod | getHKmod(KEY_DEFS.scroll);
			RegisterHotKey(ThisDlg, SCROLLUP_HK, rhkmod, VK_UP);
			RegisterHotKey(ThisDlg, SCROLLDWN_HK, rhkmod, VK_DOWN);
			RegisterHotKey(ThisDlg, SCROLLLFT_HK, rhkmod, VK_LEFT);
			RegisterHotKey(ThisDlg, SCROLLRGT_HK, rhkmod, VK_RIGHT);
		}else{
			RegisterHotKey(ThisDlg, SCROLL_TOGGLE_HK, hkmod, KEY_DEFS.scroll);
		}

	}else{
		RegisterHotKey(ThisDlg, GENERAL_TOGGLE_HK, 0, KEY_DEFS.hold);
	}
}

/********************************************************************************/
/** UnregisterHotkeys: Unregister All Hotkeys.                                 **/
/********************************************************************************/
void UnregisterHotkeys(){
	UnregisterHotKey(ThisDlg,FIXWINDOW_HK);
	UnregisterHotKey(ThisDlg,SCROLLUP_HK);
	UnregisterHotKey(ThisDlg,SCROLLDWN_HK);
	UnregisterHotKey(ThisDlg,SCROLLRGT_HK);
	UnregisterHotKey(ThisDlg,SCROLLLFT_HK);
	UnregisterHotKey(ThisDlg,SCROLL_TOGGLE_HK);
	UnregisterHotKey(ThisDlg,RESIZEUP_HK);
	UnregisterHotKey(ThisDlg,RESIZEDWN_HK);
	UnregisterHotKey(ThisDlg,RESIZERGT_HK);
	UnregisterHotKey(ThisDlg,RESIZELFT_HK);
	UnregisterHotKey(ThisDlg,RESIZE_TOGGLE_HK);
	UnregisterHotKey(ThisDlg,FIT2CONTENT_HK);
	UnregisterHotKey(ThisDlg,CENTER_HK);
	UnregisterHotKey(ThisDlg,BORDER_HK);
	UnregisterHotKey(ThisDlg,HIDETB_HK);
	UnregisterHotKey(ThisDlg,LANDSCAPE_HK);
	UnregisterHotKey(ThisDlg,GENERAL_TOGGLE_HK);
}


/********************************************************************************/
/** UnRegisterToggleActionKeys: Unregister every key used for given actions in **/
/**                             toggle-based hotkeys.                          **/
/********************************************************************************/
void UnRegisterToggleActionKeys(){
	if (!IsHoldKey(KEY_DEFS.hold)){
		UnregisterHotKey(ThisDlg,BORDER_HK);
		UnregisterHotKey(ThisDlg,FIXWINDOW_HK);
		UnregisterHotKey(ThisDlg,FIT2CONTENT_HK);
		UnregisterHotKey(ThisDlg,CENTER_HK);
		UnregisterHotKey(ThisDlg,HIDETB_HK);
		UnregisterHotKey(ThisDlg,LANDSCAPE_HK);

		if (!IsHoldKey(KEY_DEFS.resize)){
			UnregisterHotKey(ThisDlg,RESIZE_TOGGLE_HK);
		}
		UnregisterHotKey(ThisDlg,RESIZEUP_HK);
		UnregisterHotKey(ThisDlg,RESIZEDWN_HK);
		UnregisterHotKey(ThisDlg,RESIZERGT_HK);
		UnregisterHotKey(ThisDlg,RESIZELFT_HK);
		
		if (!IsHoldKey(KEY_DEFS.scroll)){
			UnregisterHotKey(ThisDlg,SCROLL_TOGGLE_HK);
		}
		UnregisterHotKey(ThisDlg,SCROLLUP_HK);
		UnregisterHotKey(ThisDlg,SCROLLDWN_HK);
		UnregisterHotKey(ThisDlg,SCROLLRGT_HK);
		UnregisterHotKey(ThisDlg,SCROLLLFT_HK);
	}else{
		if (!IsHoldKey(KEY_DEFS.resize)){
			UnregisterHotKey(ThisDlg, RESIZEUP_HK);
			UnregisterHotKey(ThisDlg, RESIZEDWN_HK);
			UnregisterHotKey(ThisDlg, RESIZELFT_HK);
			UnregisterHotKey(ThisDlg, RESIZERGT_HK);
		}
		if (!IsHoldKey(KEY_DEFS.scroll)){
			UnregisterHotKey(ThisDlg, SCROLLUP_HK);
			UnregisterHotKey(ThisDlg, SCROLLDWN_HK);
			UnregisterHotKey(ThisDlg, SCROLLLFT_HK);
			UnregisterHotKey(ThisDlg, SCROLLRGT_HK);
		}
	}
}

/********************************************************************************/
/** make_horizontal: Attempts to put objects beyond end of screen at the side. **/
/********************************************************************************/
void make_horizontal(HWND hwnd){
	HWND child;
	RECT rct;
	int px, py, w, h;
	int wx2 = AreaX1 + (AreaX2-AreaX1)/2;

	MoveWindow(hwnd,AreaX1,AreaY1,AreaX2-AreaX1,AreaY2-AreaY1,TRUE);

	child = GetWindow(hwnd,GW_CHILD);
	if (child){
		while(child){
			GetWindowRect(child,&rct);
			px = rct.left;
			py = rct.top;
			w = rct.right - px;
			h = rct.bottom - py;
			if (py >= AreaY2){
				MoveWindow(child,px+wx2, py-AreaY2+AreaY1,w,h,TRUE);
			}
			child = GetWindow(child,GW_HWNDNEXT);
		}
	}
}

/********************************************************************************/
/** Fix2PPCSize: Resizes a windows as in a PPC.                                **/
/********************************************************************************/
void Fix2PPCSize(HWND hwnd){
	SetWindowPos(hwnd,NULL,0,0,240,320,SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
}

/********************************************************************************/
/** FixWindow: Correct Window position/size to fit the 'viewable' area.        **/
/********************************************************************************/
void FixWindow(HWND hwnd){
	RECT rct;
	long x,y,w,h;
	WCHAR wstr[128];


	/* Only Dialog */
	GetClassName(hwnd,wstr,128);
	if (wcscmp(_T("Dialog"),wstr)) return;

	GetWindowRect(hwnd,&rct);
	x = rct.left;
	y = rct.top;
	w = rct.right - rct.left;
	h = rct.bottom - rct.top;


	/* Position Correction */
	if (x<AreaX1 && FixXY) x=AreaX1;
	if (y<AreaY1 && FixXY) y=AreaY1;
	if (w+x > AreaX2 && x>AreaX1 && FixXY) x = AreaX2 - w;
	if (h+y > AreaY2 && y>AreaY1 && FixXY) y = AreaY2 - h;

	/* Size Correction */
	if (FixWH && w+x > AreaX2) w = AreaX2 - x;
	if (FixWH && h+y > AreaY2) h = AreaY2 - y;

	/*  Apply changes if any */
	if	(x!=rct.left || y!=rct.top ||
		(rct.bottom-rct.top)!=h || (rct.right-rct.left)!=w){
		SetWindowPos(hwnd,NULL,x,y,w,h, SWP_FRAMECHANGED | SWP_NOZORDER);
		UpdateWindow(CurrentParent);
	}
}

/********************************************************************************/
/** ResizeToFitAll: Attempts to recursively resize windows to fit content.     **/
/********************************************************************************/
void ResizeToFitAll(HWND hwnd, int stw, int sth){
	RECT rct;
	HWND child;
	long px, py, maxh,maxw, sw, sh;
	WCHAR buff[128];
	

	GetClassName(hwnd,buff,128);
	if (wcscmp(_T("Dialog"),buff) && !MoreThanDlgs) return;
	/* Combo boxes SHOULDN'T be touched */
	if (!wcscmp(_T("combobox"),buff)) return;
	/* Don't process invisible elements */
	if (!IsWindowVisible(hwnd)) return;

	child = GetWindow(hwnd,GW_CHILD);
	if (child){
		GetWindowRect(hwnd,&rct);
		px = rct.left;
		py = rct.top;
		maxw = stw;
		maxh = sth;
		while(child){
			/* Find out the 'base' size for this child */
			GetWindowRect(child,&rct);
			sw = rct.right-rct.left;
			sh = rct.bottom-rct.top;
			ResizeToFitAll(child,((sw>stw)? stw : sw),((sh>sth)? sth : sh));
			GetWindowRect(child,&rct);
			rct.right -= px;
			rct.bottom-= py;
			if (rct.right > maxw)  maxw = rct.right;
			if (rct.bottom > maxh) maxh = rct.bottom;
			child = GetWindow(child,GW_HWNDNEXT);
		}
		SetWindowPos(hwnd,NULL,0,0,maxw,maxh,SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
	}
}



/********************************************************************************/
/** EnumParents: Callback for enumerating Parent level Windows.                **/
/********************************************************************************/
BOOL CALLBACK EnumParents(HWND hwnd, LPARAM lParam ){
	HWND child;

	CurrentParent = hwnd;
	FixWindow(hwnd);
	child=GetWindow(hwnd,GW_CHILD);
	while (child){
		FixWindow(child);
		child=GetWindow(child,GW_HWNDNEXT);
	}
	return TRUE;

}


/* FILE ROUTINES ****************************************************************/
/********************************************************************************/
/** ini_param: Return a code according valid INI parameter strings.            **/
/********************************************************************************/
int ini_param(char *str){
    int code = INIUNKNOWN;

    if (!strcmp(str,"areax1")){				code = INIX1;}
    else if (!strcmp(str,"areay1")){		code = INIY1;}
    else if(!strcmp(str,"areax2")){			code = INIX2;}
    else if(!strcmp(str,"areay2")){			code = INIY2;}
    else if(!strcmp(str,"startminimized")){	code = INISTARTMIN;}
    else if(!strcmp(str,"only2focus")){		code = INIONLY2FOCUS;}
	else if(!strcmp(str,"onlydialogs")){	code = INIONLYDIALOGS;}
    else if(!strcmp(str,"fixsize")){		code = INIFIXSIZE;}
    else if(!strcmp(str,"fixpos")){			code = INIFIXPOS;}
	else if(!strcmp(str,"taskbarup")){		code = INITASKBARUP;}
	else if(!strcmp(str,"tabctlprocess")){	code = INITABCTLPOST;}
	else if(!strcmp(str,"tryppcsizefirst")){code = INITRYPPCFIRST;}
	else if(!strcmp(str,"scrollheldkey")){	code = INISCROLLHOLD;}
	else if(!strcmp(str,"resizeheldkey")){	code = INIRESIZEHOLD;}
	else if(!strcmp(str,"borderkey")){		code = INIBORDERKEY;}
	else if(!strcmp(str,"hidetbkey")){		code = INIHIDETBKEY;}
	else if(!strcmp(str,"fitcontentkey")){	code = INIFITCKEY;}
	else if(!strcmp(str,"fitareakey")){		code = INIFITAKEY;}
	else if(!strcmp(str,"centerkey")){		code = INICENTERKEY;}
	else if(!strcmp(str,"holdkey")){		code = INIHOLDKEY;}
    return code;
}

/********************************************************************************/
/** read_ini: Reads Ini File.                                                  **/
/********************************************************************************/
void read_ini(){
	FILE *fh;
	char buff[256]="";
	char *key,*val;
	long nval;

	fh = _wfopen(WinwatchINI, _T("rt"));
	if (fh){
		while (!feof(fh)){
			fgets(buff, 255, fh);
			if (buff[strlen(buff)-1]==10) buff[strlen(buff)-1]=0;
			key = buff;
			val = strchr(buff,'=');
			if (val){
				val[0] = 0;
				val++;
				nval = atol(val);
				switch (ini_param(key)){
					case INIX1:	AreaX1 = nval;	break;
					case INIY1:	AreaY1 = nval;	break;
					case INIX2:	AreaX2 = nval;	break;
					case INIY2:	AreaY2 = nval;	break;
					case INISTARTMIN:
						StartMin = (nval!=0);
					break;
					case INIONLY2FOCUS:
						Only2Focus = (nval!=0);
					break;
					case INIONLYDIALOGS:
						MoreThanDlgs = (nval==0);
					break;
					case INIFIXSIZE:
						FixWH = (nval!=0);
					break;
					case INIFIXPOS:
						FixXY = (nval!=0);
					break;
					case INITASKBARUP:
						TaskBarUp = (nval!=0);
					break;
					case INITABCTLPOST:
						TabCtlPostProcess = (nval!=0);
					break;
					case INITRYPPCFIRST:
						TryPPCSize1st = (nval!=0);
					break;
					case INISCROLLHOLD:
						KEY_DEFS.scroll = nval;
					break;
					case INIHOLDKEY:
						KEY_DEFS.hold = nval;
					break;
					case INIRESIZEHOLD:
						KEY_DEFS.resize = nval;
					break;
					case INIBORDERKEY:
						KEY_DEFS.border = nval;
					break;
					case INIFITCKEY:
						KEY_DEFS.fitC = nval;
					break;
					case INIFITAKEY:
						KEY_DEFS.fitA = nval;
					break;
					case  INICENTERKEY:
						KEY_DEFS.center = nval;
					break;
					case INIHIDETBKEY:
						KEY_DEFS.hidetb = nval;	
				}
			}
		}
		fclose(fh);
	}
	MinimizeMe = StartMin;
}

/********************************************************************************/
/** save_ini: Saves Ini File.                                                  **/
/********************************************************************************/
void save_ini(){
	FILE *fh;

	fh = _wfopen(WinwatchINI, _T("wt"));
	if (fh){
		fprintf(fh,"areax1=%d\n",AreaX1);
		fprintf(fh,"areay1=%d\n",AreaY1);
		fprintf(fh,"areax2=%d\n",AreaX2);
		fprintf(fh,"areay2=%d\n",AreaY2);
		fprintf(fh,"startminimized=%d\n",(StartMin ? 1 : 0));
		fprintf(fh,"only2focus=%d\n",(Only2Focus ? 1 : 0));
		fprintf(fh,"onlydialogs=%d\n",(MoreThanDlgs? 0 : 1));
		fprintf(fh,"fixpos=%d\n",(FixXY ? 1 : 0));
		fprintf(fh,"fixsize=%d\n",(FixWH ? 1 : 0));
		fprintf(fh,"taskbarup=%d\n",(TaskBarUp ? 1 : 0));
		fprintf(fh,"tabctlprocess=%d\n",(TabCtlPostProcess ? 1 : 0));
		fprintf(fh,"tryppcsizefirst=%d\n",(TryPPCSize1st ? 1 : 0));
		fprintf(fh,"holdkey=%d\n", KEY_DEFS.hold);
		fprintf(fh,"scrollheldkey=%d\n", KEY_DEFS.scroll);
		fprintf(fh,"resizeheldkey=%d\n", KEY_DEFS.resize);
		fprintf(fh,"borderkey=%d\n", KEY_DEFS.border);
		fprintf(fh,"hidetbkey=%d\n", KEY_DEFS.hidetb);
		fprintf(fh,"fitcontentkey=%d\n", KEY_DEFS.fitC);
		fprintf(fh,"fitareakey=%d\n", KEY_DEFS.fitA);
		fprintf(fh,"centerkey=%d",KEY_DEFS.center);
		fclose(fh);
	}
}


/* GUI INTERFACING **************************************************************/
/********************************************************************************/
/** SetKeyField: Sets a Control Text with the 'name' of a given keycode.       **/
/********************************************************************************/
void SetKeyField(HWND KeyDlg, int DlgItem, UINT k){
	WCHAR buff[10];

	key2string(k,buff);
	SetDlgItemText(KeyDlg, DlgItem,(LPCTSTR)buff);
}

/********************************************************************************/
/** UpdateHotKeyEditor: Label each button on the Key Editing panel to current  **/
/**                     values.                                                **/
/********************************************************************************/
void UpdateHotKeyEditor(HWND KeyDlg){
	SetKeyField(KeyDlg, IDC_HOLDKEY, KEY_DEFS.hold);
	SetKeyField(KeyDlg, IDC_SCROLLKEYBTTN, KEY_DEFS.scroll);
	SetKeyField(KeyDlg, IDC_RESIZEKEYBTTN, KEY_DEFS.resize);
	SetKeyField(KeyDlg, IDC_RESKEYBTTN, KEY_DEFS.border);
	SetKeyField(KeyDlg, IDC_FITCNTKEYBTTN, KEY_DEFS.fitC);
	SetKeyField(KeyDlg, IDC_FITAREAKEYBTN, KEY_DEFS.fitA);
	SetKeyField(KeyDlg, IDC_CNTRKEYBTTN, KEY_DEFS.center);
	SetKeyField(KeyDlg, IDC_TBKEYBTTN, KEY_DEFS.hidetb);
	SetKeyField(KeyDlg, IDC_LANDKEYBTTN, KEY_DEFS.landscape);
}

/********************************************************************************/
/** GetNumericTxt: Reads a textbox as a number and returns the value.          **/
/********************************************************************************/
long GetNumericTxt(int DlgItem){
	WCHAR buff[20];
	WCHAR *stopped;
	GetDlgItemText(ThisDlg,DlgItem,(LPTSTR)buff,20);
	return wcstol(buff,&stopped, 10);

}

/********************************************************************************/
/** SetNumericTxt: Sets a textbox to a given numeric value.                    **/
/********************************************************************************/
void SetNumericTxt(int DlgItem, long n){
	WCHAR buff[20];

	_ltow(n,buff,10);
	SetDlgItemText(ThisDlg,DlgItem,(LPCTSTR)buff);
}

/********************************************************************************/
/** CheckBoxIfBool: Sets a checkbox according a boolean value.                 **/
/********************************************************************************/
void CheckBoxIfBool(HWND hdlg, int idc, BOOL b){
	SendDlgItemMessage(hdlg,idc,BM_SETCHECK,(b? BST_CHECKED : BST_UNCHECKED),0);
}

/********************************************************************************/
/** Update2Settings: Updates Main dialog with current settings.                **/
/********************************************************************************/
void Update2Settings(){
	SetNumericTxt(IDC_X1, AreaX1);
	SetNumericTxt(IDC_Y1, AreaY1);
	SetNumericTxt(IDC_X2, AreaX2);
	SetNumericTxt(IDC_Y2, AreaY2);

	CheckBoxIfBool(ThisDlg, IDC_CHECKWH, FixWH);
	CheckBoxIfBool(ThisDlg, IDC_CHECKXY, FixXY);
	CheckBoxIfBool(ThisDlg, IDC_FOCUSONLY, Only2Focus);
	CheckBoxIfBool(ThisDlg, IDC_NOTONLYDLG, MoreThanDlgs);
	CheckBoxIfBool(ThisDlg, IDC_STARTMIN, StartMin);
	CheckBoxIfBool(ThisDlg, IDC_TASKBARUP, TaskBarUp);
	CheckBoxIfBool(ThisDlg, IDC_TABCTLPROC, TabCtlPostProcess);
	CheckBoxIfBool(ThisDlg, IDC_TRYPPCFIRST, TryPPCSize1st);
}
/********************************************************************************/
/** GetCheckBoxBool: Returns TRUE if a CheckBox is checked.                    **/
/********************************************************************************/
BOOL GetCheckBoxBool(HWND hdlg, int idc){
	return (SendDlgItemMessage(hdlg,idc,BM_GETCHECK,0,0) == BST_CHECKED);
}

/********************************************************************************/
/** GetValuesFromGUI: Retrieves options from Main Dialog.                      **/
/********************************************************************************/
void GetValuesFromGUI(){
	FixXY				= GetCheckBoxBool(ThisDlg,IDC_CHECKXY);
	FixWH				= GetCheckBoxBool(ThisDlg,IDC_CHECKWH);
	StartMin			= GetCheckBoxBool(ThisDlg,IDC_STARTMIN);
	Only2Focus			= GetCheckBoxBool(ThisDlg,IDC_FOCUSONLY);
	MoreThanDlgs		= GetCheckBoxBool(ThisDlg,IDC_NOTONLYDLG);
	TaskBarUp			= GetCheckBoxBool(ThisDlg,IDC_TASKBARUP);
	TabCtlPostProcess	= GetCheckBoxBool(ThisDlg,IDC_TABCTLPROC);
	TryPPCSize1st		= GetCheckBoxBool(ThisDlg,IDC_TRYPPCFIRST);

	AreaX1 = GetNumericTxt(IDC_X1);
	AreaY1 = GetNumericTxt(IDC_Y1);
	AreaX2 = GetNumericTxt(IDC_X2);
	AreaY2 = GetNumericTxt(IDC_Y2);
}

/* ENTRY POINT ******************************************************************/
/********************************************************************************/
/** WinMain: The Default Entry Point                                           **/
/********************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR lpCmd, int nShow){
	int retcode;
	unsigned int i;
	WCHAR tmp[256];

	hInst = hInstance;
	InitCommonControls();

	memset(tmp, 0, 256*sizeof(WCHAR));
	GetModuleFileName(hInstance, tmp, 256);
	wsprintf(WinwatchEXE, _T("\"%ls\""), tmp);
	for (i=wcslen(tmp);i>0;i--){
		if (tmp[i]==(WCHAR)'\\'){
			tmp[i] = 0;
			break;
		}
	}
	wsprintf(WinwatchINI,_T("%ls\\winwatch.ini"),tmp);

	DefaultKeyDefs();
	read_ini();
	retcode = DialogBox(hInst, MAKEINTRESOURCE(IDD_WINWATCH), NULL, (DLGPROC)MainDlgProc);
	return FALSE;
}

/* DIALOGS CALLBACK *************************************************************/
/********************************************************************************/
/** KeyDlgProc: HotKey Editing dialog message processing.                      **/
/********************************************************************************/
BOOL WINAPI KeyDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent, nKey;
	int i;

	switch (msg){
		case WM_INITDIALOG:
			HotkeyDlg = hDlg;
			UnregisterHotkeys();
			/* Hotkeys for difficult keys */
			RegisterHotKey(hDlg,WIN_HOTKEY,MOD_WIN, VK_LWIN);
			RegisterHotKey(hDlg,ESC_HOTKEY,0, VK_ESCAPE);
			RegisterHotKey(hDlg,UP_HOTKEY,0, VK_UP);
			RegisterHotKey(hDlg,DOWN_HOTKEY,0, VK_DOWN);
			RegisterHotKey(hDlg,LEFT_HOTKEY,0, VK_LEFT);
			RegisterHotKey(hDlg,RIGHT_HOTKEY,0, VK_RIGHT);
			for (i=0xc1;i<0xcf;i++) RegisterHotKey(hDlg, FREEHK_KEYAREA+i, MOD_WIN, i);
			Now_Editting = -99;
		break;
		case WM_PAINT:
			UpdateHotKeyEditor(hDlg);
		break;
		case WM_CLOSE:
			/* Unregister our Key traps if not done before */
			UnregisterHotKey(hDlg, WIN_HOTKEY);
			UnregisterHotKey(hDlg, ESC_HOTKEY);
			UnregisterHotKey(hDlg, UP_HOTKEY);
			UnregisterHotKey(hDlg, DOWN_HOTKEY);
			UnregisterHotKey(hDlg, LEFT_HOTKEY);
			UnregisterHotKey(hDlg, RIGHT_HOTKEY);
			for (i=0xc1;i<0xcf;i++) UnregisterHotKey(hDlg, FREEHK_KEYAREA+i);
			/* Enable HotKeys again */
			RegisterHotkeys();
			EndDialog(hDlg, TRUE);
			HotkeyDlg = NULL;
			return TRUE;

		case WM_HOTKEY:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (Now_Editting != -99) {
				if (msg != WM_HOTKEY) nKey = (int)wParam;
				else{
					nKey = (UINT)HIWORD(lParam);
				}
				if (nKey == VK_ESCAPE) nKey = 0;
				switch (Now_Editting){
					case IDC_HOLDKEY:
						KEY_DEFS.hold = nKey;
					break;
					case IDC_SCROLLKEYBTTN:
						KEY_DEFS.scroll = nKey;
					break;
					case IDC_RESIZEKEYBTTN:
						KEY_DEFS.resize = nKey;
					break;
					case IDC_RESKEYBTTN:
						KEY_DEFS.border = nKey;
					break;
					case IDC_FITCNTKEYBTTN:
						KEY_DEFS.fitC = nKey;
					break;
					case IDC_FITAREAKEYBTN:
						KEY_DEFS.fitA = nKey;;
					break;
					case IDC_CNTRKEYBTTN:
						KEY_DEFS.center = nKey;
					break;
					case IDC_TBKEYBTTN:
						KEY_DEFS.hidetb = nKey;
					break;
					case IDC_LANDKEYBTTN:
						KEY_DEFS.landscape = nKey;
				}
				
				UpdateHotKeyEditor(hDlg);
			}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam);
			switch (wmId){
				case IDCLEAR:
					SendMessage(hDlg, WM_KEYDOWN, VK_ESCAPE, 0);
				break;
				case IDOK:
					SendMessage(hDlg,WM_CLOSE,0,0);
				break;
				case IDDEFAULT:
					DefaultKeyDefs();
					UpdateHotKeyEditor(hDlg);
				break;
				default:
					if (Now_Editting != -99) UpdateHotKeyEditor(hDlg);
					Now_Editting = wmId;
					SetDlgItemText(hDlg,wmId,_T("<Press>"));
					SetFocus(hDlg);
			}
			SetFocus(hDlg);
			return TRUE;

		default:
				return FALSE;
	}
	SetFocus(hDlg);
	return FALSE;
}

/********************************************************************************/
/** MainDlgProc: Main Dialog Message Processing.                               **/
/********************************************************************************/
BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	HWND handle;
	DWORD mid;

	switch(msg){
		case WM_PAINT:
			if (MinimizeMe) {
				HideDialog();
				MinimizeMe = FALSE;
				return TRUE;
			}else return FALSE; /* Let DefDlgProc Handle painting */
			
		case WM_CLOSE:
				UnregisterHotkeys();
				TrayIconDelete(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(APP_ICON)), NULL);
				EndDialog(hDlg, TRUE);
			return TRUE;

		case WM_INITDIALOG:
			if (FileExists(WWSHORTCUT)) CheckBoxIfBool(hDlg, IDC_RUNSTARTUP, TRUE);
			ThisDlg = hDlg;
			RegisterHotkeys();
			MakePopups();
			TrayIconAdd(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(APP_ICON)), NULL);
			SendDlgItemMessage(hDlg, IDC_X1, EM_SETLIMITTEXT,4,0);
			SendDlgItemMessage(hDlg, IDC_Y1, EM_SETLIMITTEXT,4,0);
			SendDlgItemMessage(hDlg, IDC_X2, EM_SETLIMITTEXT,4,0);
			SendDlgItemMessage(hDlg, IDC_Y2, EM_SETLIMITTEXT,4,0);
			Update2Settings();
			SetForegroundWindow(hDlg);
			/* Apply Misc Settings */
			if (TaskBarUp) taskbarup();
			UpdatePopups();
			return TRUE;
		case WM_HOTKEY:
			switch(wParam){
				/* TOGGLE CASES ***********************************************/
				case GENERAL_TOGGLE_HK:
					UnRegisterToggleActionKeys();
					if (ToggledPressed == GENERAL_TOGGLE_HK) {
						ToggledPressed = 0;
						return TRUE;
					}
					ToggledPressed = GENERAL_TOGGLE_HK;
					RegisterHotKey(ThisDlg, FIXWINDOW_HK, 0, KEY_DEFS.fitA);
					RegisterHotKey(ThisDlg, FIT2CONTENT_HK, 0, KEY_DEFS.fitC);
					RegisterHotKey(ThisDlg, BORDER_HK, 0, KEY_DEFS.border);
					RegisterHotKey(ThisDlg, CENTER_HK, 0, KEY_DEFS.center);
					RegisterHotKey(ThisDlg, HIDETB_HK, 0, KEY_DEFS.hidetb);
					RegisterHotKey(ThisDlg, LANDSCAPE_HK, 0, KEY_DEFS.landscape);
					if (IsHoldKey(KEY_DEFS.resize)){
						RegisterHotKey(ThisDlg, RESIZEUP_HK, getHKmod(KEY_DEFS.resize), VK_UP);
						RegisterHotKey(ThisDlg, RESIZEDWN_HK, getHKmod(KEY_DEFS.resize), VK_DOWN);
						RegisterHotKey(ThisDlg, RESIZELFT_HK, getHKmod(KEY_DEFS.resize), VK_LEFT);
						RegisterHotKey(ThisDlg, RESIZERGT_HK, getHKmod(KEY_DEFS.resize), VK_RIGHT);
					}else{
						RegisterHotKey(ThisDlg, RESIZE_TOGGLE_HK, 0, KEY_DEFS.resize);
					}

					if (IsHoldKey(KEY_DEFS.scroll)){
						RegisterHotKey(ThisDlg, SCROLLUP_HK, getHKmod(KEY_DEFS.scroll), VK_UP);
						RegisterHotKey(ThisDlg, SCROLLDWN_HK, getHKmod(KEY_DEFS.scroll), VK_DOWN);
						RegisterHotKey(ThisDlg, SCROLLLFT_HK, getHKmod(KEY_DEFS.scroll), VK_LEFT);
						RegisterHotKey(ThisDlg, SCROLLRGT_HK, getHKmod(KEY_DEFS.scroll), VK_RIGHT);
					}else{
						RegisterHotKey(ThisDlg, SCROLL_TOGGLE_HK, 0, KEY_DEFS.scroll);
					}
					return TRUE;

				case RESIZE_TOGGLE_HK:
					UnRegisterToggleActionKeys();
					if (ToggledPressed == RESIZE_TOGGLE_HK){
						ToggledPressed  = 0;
						return TRUE;
					}
					RegisterHotKey(ThisDlg,RESIZEUP_HK,0,VK_UP);
					RegisterHotKey(ThisDlg,RESIZEDWN_HK,0,VK_DOWN);
					RegisterHotKey(ThisDlg,RESIZERGT_HK,0,VK_RIGHT);
					RegisterHotKey(ThisDlg,RESIZELFT_HK,0,VK_LEFT);
					ToggledPressed = RESIZE_TOGGLE_HK;
					return TRUE;

				case SCROLL_TOGGLE_HK:
					UnRegisterToggleActionKeys();
					if (ToggledPressed == SCROLL_TOGGLE_HK){
						ToggledPressed  = 0;
						return TRUE;
					}
					RegisterHotKey(ThisDlg,SCROLLUP_HK,0,VK_UP);
					RegisterHotKey(ThisDlg,SCROLLDWN_HK,0,VK_DOWN);
					RegisterHotKey(ThisDlg,SCROLLRGT_HK,0,VK_RIGHT);
					RegisterHotKey(ThisDlg,SCROLLLFT_HK,0,VK_LEFT);
					ToggledPressed = SCROLL_TOGGLE_HK;
					return TRUE;

				/* FUNCTIONAL CASES *******************************************/
				case FIT2CONTENT_HK:
					ToggledPressed = 0;
					GetValuesFromGUI();
					handle = GetForegroundWindow();
					if (TryPPCSize1st){
						Fix2PPCSize(handle);
						ResizeToFitAll(handle,240,320);
					}else{
						ResizeToFitAll(handle,16,16);
					}
					if (TabCtlPostProcess) FixTabCtrls(handle);
					return TRUE;
				case RESIZEUP_HK:
					ResizeNoMove(GetForegroundWindow(),0,-ScrollStep);
					return TRUE;
				case RESIZEDWN_HK:
					ResizeNoMove(GetForegroundWindow(),0, ScrollStep);
					return TRUE;
				case RESIZELFT_HK:
					ResizeNoMove(GetForegroundWindow(),-ScrollStep,0);
					return TRUE;
				case RESIZERGT_HK:
					ResizeNoMove(GetForegroundWindow(),ScrollStep,0);
					return TRUE;
				case SCROLLUP_HK:
					ScrollNoResize(GetForegroundWindow(),0,-ScrollStep);
					return TRUE;
				case SCROLLDWN_HK:
					ScrollNoResize(GetForegroundWindow(),0, ScrollStep);
					return TRUE;
				case SCROLLRGT_HK:
					ScrollNoResize(GetForegroundWindow(),ScrollStep, 0);
					return TRUE;
				case SCROLLLFT_HK:
					ScrollNoResize(GetForegroundWindow(),-ScrollStep,0);
					return TRUE;
				case FIXWINDOW_HK:
					/* Get Current Settings*/
					GetValuesFromGUI();
					/* Get active window */
					if (Only2Focus){
						FixWindow(GetForegroundWindow());
					}else
						EnumWindows((WNDENUMPROC)EnumParents,0);
					return TRUE;
				case CENTER_HK:
					CenterWindow(GetForegroundWindow());
					return TRUE;
				case BORDER_HK:
					MakeSizeable(GetForegroundWindow());
					return TRUE;
				case HIDETB_HK:
					HideTaskBar();
					return TRUE;
				case LANDSCAPE_HK:
					make_horizontal(GetForegroundWindow());
					return TRUE;
			}
			break;      
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId){
				case IDSAVEBUTTON:
					GetValuesFromGUI();
					save_ini();
					return TRUE;
				case IDHIDEBTTN:
				case IDOK:
					HideDialog();
					UpdatePopups();
					return TRUE;
				case IDQUIT:
					if (AskExit(hDlg)){
						if (HotkeyDlg) SendMessage(HotkeyDlg,WM_CLOSE,0,0);
						SendMessage(ThisDlg,WM_CLOSE,0,0);
					}
					return TRUE;
				case IDC_TASKBARUP:
					/* Changed Option? */
					TaskBarUp = !TaskBarUp;
					(TaskBarUp ? taskbarup() : taskbardown());
					return TRUE;
				case IDC_RUNSTARTUP:
					if (GetCheckBoxBool(hDlg, IDC_RUNSTARTUP)){
						SHCreateShortcut(WWSHORTCUT,WinwatchEXE);
					}else{
						DeleteFile(WWSHORTCUT);
					}
					return TRUE;
				case IDHKEYSBTTN:
					EnableWindow(hDlg, FALSE);
					DialogBox(hInst, MAKEINTRESOURCE(IDD_KEYEDITOR), NULL, (DLGPROC)KeyDlgProc);
					EnableWindow(hDlg, TRUE);
					SetForegroundWindow(hDlg);
					return TRUE;
				default:
				   return DefWindowProc(hDlg, msg, wParam, lParam);
			}
			break;
      
		case TRAY_NOTIFYICON:
			switch (lParam){
				case WM_LBUTTONDBLCLK:
					if (wParam == ID_TRAY){
						if (WeAreHidden) ShowDialog();
						else HideDialog();
						return (TRUE);
					}
				case WM_LBUTTONDOWN:
					if (wParam == ID_TRAY){
						if (WeAreHidden) {
							mid = DisplayMenu();
							switch (mid){
								case PMENU_SCROLL1:	ScrollStep = 1 ; break;
								case PMENU_SCROLL2:	ScrollStep = 2 ; break;
								case PMENU_SCROLL3:	ScrollStep = 3 ; break;
								case PMENU_SCROLL5:	ScrollStep = 5 ; break;
								case PMENU_SCROLL10:ScrollStep = 10 ;break;
								case PMENU_SCROLL20:ScrollStep = 20 ;break;
								case PMENU_SCROLL40:ScrollStep = 40 ;break;
								case PMENU_SCROLL60:ScrollStep = 60; break;
								case PMENU_TABPROCESS:
									TabCtlPostProcess = !TabCtlPostProcess;
									break;
								case PMENU_FIXPOSITION:
									FixXY = !FixXY;
									break;
								case PMENU_FIXSIZE:
									FixWH = !FixWH;
									break;
								case PMENU_ONLYACTIVE:
									Only2Focus = !Only2Focus;
									break;
								case PMENU_NOTONLYDLGS:
									MoreThanDlgs = !MoreThanDlgs;
									break;
								case PMENU_PPCSIZEFIRST:
									TryPPCSize1st = !TryPPCSize1st;
									break;
								case PMENU_EXIT:
									if (AskExit(NULL)){
										SendMessage(hDlg, WM_CLOSE,0,0);
									}
									break;
								case PMENU_SHOWWINWATCH:
									ShowDialog();
									break;
							}
							Update2Settings();
							UpdatePopups();
						}
						return TRUE;
					}
			}
			break;
	}
	return FALSE;
}
