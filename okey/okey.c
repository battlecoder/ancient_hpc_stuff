#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <stdlib.h>
#include <commctrl.h>
#include <commdlg.h>
#include "keydefs.h"
#include "KBhook.h"


/* === Constants And Macros =================================================== */
#define SIGNATURE_V01 "OKV1"
#define CURRENT_VERSION_STRING	"v 2.0 [2MyBeloved]"
#define OKSHORTCUT	_T("\\Windows\\StartUp\\OKey.lnk")
#define WM_LOADLAYOUT	WM_APP + 100

#define TRAY_NOTIFYICON WM_USER + 2001
#define ID_TRAY	5000

//#define DEBUG_ENABLE

/* === Function Prototypes ==================================================== */
BOOL WINAPI MainDlgProc( HWND, UINT, WPARAM, LPARAM );
int GetLastBSlash(WCHAR *st);
BOOL AddCombination(UINT mCode, BYTE vk, UINT uni);
BOOL DelCombination(int n);
UINT ModCode (BOOL Alt, BOOL Win, BOOL Ctrl, BOOL Shift);
void type_a_number(UINT val);
void TrayIconModify(HWND hwnd, UINT uID, HICON hIcon, PTSTR pszTip);
WCHAR *GetLayoutName(WCHAR *dest, int len, WCHAR *f);
void UpdateHeader();
void SetPowerKey(UINT k);
void UnHook();
void HookAgain();
void EnableIcon();
void DisableIcon();

/* === Globals ================================================================ */
typedef struct{
	UINT vMod;
	BYTE vkCode;
	UINT unicode;
} VK_COMBINATION;


BYTE Map[256];
VK_COMBINATION Combinations[256];
LOGFONT SysFont[256];
HFONT FontHandle;

HINSTANCE  g_hHookApiDLL	= NULL;	/* handle to coredll.dll */
HHOOK g_hInstalledLLKBDhook = NULL;	/* handle to the installed KB hook */

/***** Specific Dialog Information *******/
HWND ThisDlg;		/* MainDialog Handle */
HWND KeyMapDlg;
HWND AboutDlg;
HWND AdvancedDlg;
HWND MapHdlA;
HWND MapHdlB;
HWND MapHdlK;

HMENU PopUp;

BOOL WaitingKeyA = FALSE;
BOOL WaitingKeyB = FALSE;
BOOL WaitingKeyC = FALSE;
BOOL WaitingPowerKey = FALSE;
BOOL Trapped = FALSE;
BOOL StartingApp = TRUE;
BOOL NoParseESC = TRUE;
BOOL UseNumpad = FALSE;


UINT MappingKey = 0;
BYTE CKey = 0;

/***** OPTIONS ***************************/
BOOL LoadLast = TRUE;
BOOL StartMinimized = TRUE;
WCHAR LastFile[256]  = _T("\0");
WCHAR CurrentFile[256]  = _T("\0");
WCHAR LastDir[256];

typedef struct {
	BOOL unpress;
	BYTE code;
}_KEYCODE_EVENT;
_KEYCODE_EVENT KEYCODE_EVENT[64];
int KEYCODEinQueue = 0;

UINT POWER_KEY = 223;

/*** Global Status and Flags *************/
HINSTANCE hInst;	/* App Instance  *****/
BOOL WeAreHidden;
BOOL Hooked = FALSE;
BOOL _ALT_ = FALSE;
BOOL _CTRL_ = FALSE;
BOOL _WIN_  = FALSE;
BOOL _SHIFT_= FALSE;
BOOL _CAPS_= FALSE;
BOOL DisableProc = FALSE;

int TotalFonts = 0;
int TotalCombinations = 0;
BOOL AddNullKeyCode = FALSE;
WCHAR LayoutName[21];
WCHAR OKeyEXE[256];

struct QA_ENTRY{
	WCHAR file[256];
	WCHAR name[21];
}QuickAccess[10];
int TotalQA = 0;

/* Optional Debugging Functions *************************************************/
#ifdef DEBUG_ENABLE 
FILE *debugFh;
/* Init Debug **************/
void InitDebug(char *f){
	debugFh = fopen(f, "w");
}

/* End Debug ***************/
void EndDebug(){
	if (debugFh) fclose(debugFh);
}
#endif


/* INI FILE Functions ***********************************************************/
/********************************************************************************/
/** UnRegister4OKFiles: Unregisters the OK File Extension.                     **/
/********************************************************************************/
void UnRegister4OKFiles(){
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T(".ok"));
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T("oklayout\\Shell\\Open\\Command"));
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T("oklayout\\Shell\\Open"));
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T("oklayout\\Shell"));
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T("oklayout\\DefIcon"));
	RegDeleteKey(HKEY_CLASSES_ROOT ,_T("oklayout"));
}

/********************************************************************************/
/** GetRegString: Retrieve a string from the registry.                         **/
/********************************************************************************/
void GetRegString(WCHAR *buff,UINT len, HKEY hKRoot, WCHAR *skey, WCHAR *ValName){
	HKEY RegKey;
	DWORD dispo;
	DWORD type;
	DWORD sSize = len*sizeof(WCHAR);

	memset(buff,0,len*sizeof(WCHAR));
	RegCreateKeyEx(hKRoot, skey,0,_T("REG_SZ"),0,0,NULL,&RegKey, &dispo);
	RegQueryValueEx (RegKey,ValName,NULL,&type,(BYTE*)buff,&sSize);
	RegCloseKey(RegKey);
}

/********************************************************************************/
/** GetRegDword: Retrieve a dword from the registry.                           **/
/********************************************************************************/
void GetRegDword(DWORD *buff, HKEY hKRoot, WCHAR *skey, WCHAR *ValName){
	HKEY RegKey;
	DWORD dispo;
	DWORD type;
	DWORD sSize = sizeof(DWORD);

	RegCreateKeyEx(hKRoot, skey,0,_T("REG_DWORD"),0,0,NULL,&RegKey, &dispo);
	RegQueryValueEx (RegKey,ValName,NULL,&type,(BYTE*)buff,&sSize);
	RegCloseKey(RegKey);
}

/********************************************************************************/
/** SetRegString: Sets a string from the registry.                             **/
/********************************************************************************/
void SetRegString(HKEY hKRoot, WCHAR *skey, WCHAR *ValName, WCHAR *nval){
	HKEY RegKey;
	DWORD dispo;

	RegCreateKeyEx(hKRoot, skey,0,_T("REG_SZ"),0,0,NULL,&RegKey, &dispo);
	RegSetValueEx (RegKey,ValName,0,REG_SZ, (BYTE*)nval,sizeof(WCHAR)*wcslen(nval));
	RegCloseKey(RegKey);
}

/********************************************************************************/
/** SetRegDword: Sets a dword from the registry.                               **/
/********************************************************************************/
void SetRegDword(HKEY hKRoot, WCHAR *skey, WCHAR *ValName, DWORD nval){
	HKEY RegKey;
	DWORD dispo;

	RegCreateKeyEx(hKRoot, skey,0,_T("REG_DWORD"),0,0,NULL,&RegKey, &dispo);
	RegSetValueEx (RegKey,ValName,0,REG_DWORD, (BYTE*)&nval,sizeof(DWORD));
	RegCloseKey(RegKey);
}

/********************************************************************************/
/** AreOKFilesRegistered: Check that OK files are registered to us.            **/
/********************************************************************************/
BOOL AreOKFilesRegistered(){
	WCHAR buff[256];

	GetRegString(buff,256, HKEY_CLASSES_ROOT,_T(".ok"),NULL);
	return (wcscmp(buff, _T("oklayout")) == 0? TRUE: FALSE);
}

/********************************************************************************/
/** Register4OKFiles: Registers the OK File Extension.                         **/
/********************************************************************************/
void Register4OKFiles(){
	WCHAR iconstr[280];
	WCHAR shellstr[262];

	wcscpy(shellstr, OKeyEXE);
	wcscat(shellstr, _T(" %1"));
	wsprintf(iconstr, _T("%ls,-%d"),OKeyEXE, IDI_FILEICON);

	SetRegString ( HKEY_CLASSES_ROOT, _T(".ok"), NULL, _T("oklayout"));
	SetRegString ( HKEY_CLASSES_ROOT, _T("oklayout\\DefaultIcon"), NULL,iconstr);
	SetRegString ( HKEY_CLASSES_ROOT, _T("oklayout\\Shell\\Open\\Command"),NULL,
					shellstr);
}

/********************************************************************************/
/** AddQuickAccess: Adds a layout to the QA list.                              **/
/********************************************************************************/
void AddQuickAccess(WCHAR *fname, WCHAR *layname){
	wcscpy(QuickAccess[TotalQA].file, fname);
	wcscpy(QuickAccess[TotalQA].name, layname);
	TotalQA ++;
}

/********************************************************************************/
/** DelQuickAccess: Deletes a Quick Access entry.                              **/
/********************************************************************************/
BOOL DelQuickAccess(int ndx){
	int i;
	if (ndx >= TotalQA || ndx < 0) return FALSE;

	for(i=ndx;i<TotalQA-1;i++){
		memcpy(&QuickAccess[i],&QuickAccess[i+1],sizeof(struct QA_ENTRY));
	}
	TotalQA--;
	return TRUE;
}

/********************************************************************************/
/** ReloadQAList: Fills the QAList.                                            **/
/********************************************************************************/
void ReloadQAList(){
	int i;

	for (i=0;i<TotalQA;i++){
		SendDlgItemMessage(AdvancedDlg,IDC_QALIST, LB_ADDSTRING,0,
						(LPARAM)QuickAccess[i].file);
	}
}

/********************************************************************************/
/** LoadOptions: Load Options from Registry.                                   **/
/********************************************************************************/
void LoadOptions(){
	DWORD LoadLastB = 0, StMinimized = 0, NoESC = 0, Nump = 0;
	WCHAR namebuff[30], filebuff[256];
	int i;

	GetRegString (LastFile, 256, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\LastFile"),NULL);
	GetRegDword (&LoadLastB, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\LoadLast"), NULL);
	GetRegDword (&StMinimized, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\StartMinimized"), NULL);
	GetRegDword (&NoESC, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\NeverMapESC"), NULL);
	GetRegDword (&Nump, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\UseNumpad"), NULL);
	GetRegDword (&POWER_KEY, HKEY_CURRENT_USER,
				_T("\\Software\\OKey\\PowerKey"), NULL);

	LoadLast = (LoadLastB!=0);
	StartMinimized = (StMinimized!=0);
	NoParseESC = (NoESC!=0);
	UseNumpad = (Nump!=0);

	for (i=0;i<10;i++){
		wsprintf(namebuff, _T("#%d"), i+1);
		GetRegString(filebuff,256,HKEY_CURRENT_USER,
					_T("\\Software\\OKey\\QuickAccess"),namebuff);
		GetLayoutName(namebuff, 20, filebuff);
		if (namebuff[0]!='\0'){
			AddQuickAccess(filebuff, namebuff);
		}
	}
}

/********************************************************************************/
/** SaveOptions: Saves some options in registry.                               **/
/********************************************************************************/
void SaveOptions(){
	WCHAR namebuff[10];
	int i;
	DWORD LoadLastB = (DWORD)LoadLast;
	DWORD StMin = (DWORD)StartMinimized;
	DWORD NoESC = (DWORD)NoParseESC;
	DWORD Nump = (DWORD)UseNumpad;

	SetRegString(HKEY_CURRENT_USER, _T("\\Software\\OKey\\LastFile"),NULL, LastFile);
	SetRegDword(HKEY_CURRENT_USER,_T("\\Software\\OKey\\LoadLast"), NULL, LoadLastB);
	SetRegDword(HKEY_CURRENT_USER,_T("\\Software\\OKey\\StartMinimized"),NULL, StMin);
	SetRegDword(HKEY_CURRENT_USER,_T("\\Software\\OKey\\NeverMapESC"), NULL, NoESC);
	SetRegDword(HKEY_CURRENT_USER,_T("\\Software\\OKey\\UseNumpad"), NULL, Nump);
	SetRegDword(HKEY_CURRENT_USER,_T("\\Software\\OKey\\PowerKey"), NULL, POWER_KEY);

	for (i=0;i<10;i++){
		wsprintf(namebuff, _T("#%d"), i+1);
		if (i<TotalQA){
			SetRegString(HKEY_CURRENT_USER,_T("\\Software\\OKey\\QuickAccess"),
						namebuff,QuickAccess[i].file);
		}else{
			SetRegString(HKEY_CURRENT_USER,_T("\\Software\\OKey\\QuickAccess"),
						namebuff,_T("\0"));
		}
	}
}

/* KBHOOK Functions *************************************************************/
/********************************************************************************/
/** add_keycode_event: Adds a keycode to the "waiting for" queue.              **/
/********************************************************************************/
void add_keycode_event(BOOL upress, BYTE cde){
	KEYCODE_EVENT[KEYCODEinQueue].unpress = upress;
	KEYCODE_EVENT[KEYCODEinQueue].code = cde;
	KEYCODEinQueue++;
}

/********************************************************************************/
/** pop_keycode_event: Removes a keycode even from queue.                      **/
/********************************************************************************/
void pop_keycode_event(){
	int i;
	for (i=1;i<KEYCODEinQueue;i++){
		KEYCODE_EVENT[i-1].unpress = KEYCODE_EVENT[i].unpress;
		KEYCODE_EVENT[i-1].code = KEYCODE_EVENT[i].code;
	}
	KEYCODEinQueue--;
}

/********************************************************************************/
/** key_press: Simulates a key press and sets the KEYCODE_EVENT queue for this **/
/**            event.                                                          **/
/********************************************************************************/
void key_press(BYTE bVk, BYTE bScan, DWORD dwFlags){
	BOOL unpress;

	unpress = ((dwFlags & KEYEVENTF_KEYUP)? TRUE : FALSE);
	add_keycode_event(unpress, (BYTE)(AddNullKeyCode? 0 : bVk));

	#ifdef DEBUG_ENABLE
	fprintf(debugFh, "KP[%d]\tADD_KEY\tA%dW%dC%dS%dP%d %d\n",bVk, _ALT_, _WIN_,
			_CTRL_, _SHIFT_, _CAPS_, !unpress);
	#endif

	keybd_event(bVk, bScan, dwFlags | KEYEVENTF_SILENT, 0);
}

/********************************************************************************/
/** search_kcombo: Verifies if is there any combination associated with the    **/
/**                current pressed keys.                                       **/
/********************************************************************************/
int search_kcombo(BYTE vk, BOOL alt,BOOL win, BOOL ctrl, BOOL shift){
	UINT mod = ModCode(alt,win,ctrl,shift);
	int i;

	for (i=0;i<TotalCombinations;i++){
		if (Combinations[i].vkCode == vk &&
			Combinations[i].vMod == mod) return i;
	}
	return -1;
}

/********************************************************************************/
/** check_release_modifiers: update Special keys status.                       **/
/********************************************************************************/
void check_release_modifiers(UINT vk, WORD flag){
	switch (vk){
		case VK_MENU:
			_ALT_ = !(flag!=0);
		break;
		case VK_LWIN:
		case VK_RWIN:
			_WIN_ = !(flag!=0);
		break;
		case VK_CONTROL:
			_CTRL_ = !(flag!=0);
		break;
		case VK_SHIFT:
			_SHIFT_ = !(flag!=0);
	}
}

/********************************************************************************/
/** press_held_keys: Press or released the keys that generated a given UNICODE **/
/**                  combination.                                              **/
/********************************************************************************/
void press_held_keys(UINT mod_code, BOOL pressed){
	DWORD dflag = (pressed? 0: KEYEVENTF_KEYUP);
	if (mod_code & MOD_WIN) key_press(VK_LWIN, 0, dflag);
	if (mod_code & MOD_CONTROL) key_press(VK_CONTROL, 0, dflag);
	if (mod_code & MOD_ALT) key_press(VK_MENU, 0, dflag);
	if (mod_code & MOD_SHIFT){
		if (!_CAPS_) key_press(VK_SHIFT, 0, dflag);
	}else{
		if (_CAPS_) key_press(VK_SHIFT, 0, dflag);
	}
}

/********************************************************************************/
/** LLKeyHookCallback: Keyboard Hook Callback.                                 **/
/**                    [Based on code by Prathamesh Kulkarni]                  **/
/********************************************************************************/
LRESULT CALLBACK LLKeyHookCallback(int nCode, WPARAM wParam, LPARAM lParam){
	KBDLLHOOKSTRUCT* vk_info = (KBDLLHOOKSTRUCT*)lParam;
	BYTE vkCode;
	WORD flag;
	UINT unicode;
	BOOL unpress;
	int combination;
	

	vkCode = (BYTE)vk_info->vkCode;
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		flag = 0;
	}else {
		flag =  KEYEVENTF_KEYUP;
	}

	/* SPECIAL EVENTS ***********************************/
	if (vkCode == VK_ESCAPE && NoParseESC) 
		return CallNextHookEx(g_hInstalledLLKBDhook, nCode, wParam, lParam);

	if (flag != 0){
		if (WaitingPowerKey){
			EnableIcon();
			DisableProc = FALSE;
			SetPowerKey(vkCode);
		}else if (DisableProc && vkCode == POWER_KEY){
			DisableProc = FALSE;
			EnableIcon();
		}
	}

	if ((vkCode == POWER_KEY && flag==0) || DisableProc){
		DisableIcon();
		DisableProc = TRUE;
		KEYCODEinQueue = 0;
		return CallNextHookEx(g_hInstalledLLKBDhook, nCode, wParam, lParam);
	}

	/* Are we waiting for any event?*/
	if (KEYCODEinQueue > 0) {
		/* Yes? See if we were waiting for THIS event */
		unpress = ((flag == KEYEVENTF_KEYUP)? TRUE : FALSE);
		if ((KEYCODE_EVENT[0].code == vkCode || KEYCODE_EVENT[0].code==0) && 
			KEYCODE_EVENT[0].unpress == unpress){
			pop_keycode_event();

			#ifdef DEBUG_ENABLE
			fprintf(debugFh, "CB[%d] %d\tQUEUE_MATCH\tRETURNING_TO_OS\n",vkCode,!unpress);
			#endif

			/* Let the event pass as it should be */
			return CallNextHookEx(g_hInstalledLLKBDhook, nCode, wParam, lParam);
		}
	}

	/* Hardware Key Codes ****************************************/
	if (vkCode > 0xc0 && vkCode < 0xd0 && _WIN_){
		/* Trow windows key */
		key_press(0,0,0);
		key_press(VK_LWIN,0,KEYEVENTF_KEYUP);
		key_press(0,0,KEYEVENTF_KEYUP);
	}

	#ifdef DEBUG_ENABLE
	fprintf(debugFh, "CB[%d]\tNEW_EVENT\tA%dW%dC%dS%dP%d\n",vkCode, _ALT_, _WIN_,
			_CTRL_,_SHIFT_, _CAPS_);
	#endif
	/* No? then map the event */
	check_release_modifiers(vkCode, flag);
	/* Caps Lock */
	if (vkCode == 20 && flag!=0) _CAPS_ = !_CAPS_;

	combination = search_kcombo(vkCode, _ALT_,_WIN_,_CTRL_,(_SHIFT_ ^ _CAPS_));
	if (combination != -1 && flag == 0){
		unicode = Combinations[combination].unicode;
		/* Release Held Keys ***************************/
		press_held_keys(Combinations[combination].vMod, FALSE);
		/* Release the trigger key */
		key_press(vkCode,0,KEYEVENTF_KEYUP);
		key_press(VK_MENU,0,0);
		AddNullKeyCode = TRUE;
		type_a_number(unicode);
		AddNullKeyCode = FALSE;
		key_press(VK_MENU, 0,KEYEVENTF_KEYUP);
		/* Press them again ****************************/
		press_held_keys(Combinations[combination].vMod, TRUE);
	}else{
		key_press(Map[vkCode], 0, flag);
	}
	return -1;
}


/********************************************************************************/
/** ActivateKBHook: Load required libraries and attempt to set a hook for      **/
/**                 Keyboard Input. [Code by Prathamesh Kulkarni]              **/
/********************************************************************************/
BOOL ActivateKBHook(HINSTANCE hInstance, HOOKPROC LLKeyHookCallback){
	SetWindowsHookEx	= NULL;
	CallNextHookEx	= NULL;
	UnhookWindowsHookEx	= NULL;

	/* now load the coredll.dll */
	g_hHookApiDLL = LoadLibrary(_T("coredll.dll"));
	if(g_hHookApiDLL == NULL) {
		/* something is awfully wrong
		   the dll has to be present */
		return FALSE;
	}else{
		/* load the SetWindowsHookEx API call
		  the SetWindowsHookEx function installs an application-defined
		  hook procedure into a hook chain. You would install a hook procedure
		  to monitor the system for certain types of events.here we use use
		  the hook to monitor kyeboard events */
		SetWindowsHookEx = (_SetWindowsHookExW)
							GetProcAddress(g_hHookApiDLL, _T("SetWindowsHookExW"));
		if(SetWindowsHookEx == NULL){
			/* this means that MS has really stopped supporting this API in WinCE */
			return FALSE;
		}else{
			/* install the KB hook
			   the hande needs to be saved for default processing of the events and
			   to uninstall the hook, once we are done with it */
			HookAgain();
			if(g_hInstalledLLKBDhook == NULL) return FALSE;
		}

		/* load CallNextHookEx() API call
		  the CallNextHookEx function passes the hook information to the
		  next hook procedure in the current hook chain. we use this call
		  for default processing of events. */
		CallNextHookEx = (_CallNextHookEx)
						GetProcAddress(g_hHookApiDLL, _T("CallNextHookEx"));
		if(CallNextHookEx == NULL) return FALSE;


		/* load UnhookWindowsHookEx() API
		   the UnhookWindowsHookEx function removes a hook procedure
		   installed in a hook chain by the SetWindowsHookEx function.
		   we use this call to unistall the hook. */
		UnhookWindowsHookEx = (_UnhookWindowsHookEx)
							GetProcAddress(g_hHookApiDLL, _T("UnhookWindowsHookEx"));
		if(UnhookWindowsHookEx == NULL)	return FALSE;
	}

	/* all the APIs are loaded and the application is hooked */
	return TRUE;
}

/********************************************************************************/
/** DeactivateKBHook: Removes Keyboard Hook. [Code by Prathamesh Kulkarni]     **/
/********************************************************************************/
BOOL DeactivateKBHook(){
	/* unload the hook */
	UnHook();
	/* unload the coredll.dll */
	if(g_hHookApiDLL != NULL) {
		FreeLibrary(g_hHookApiDLL);
		g_hHookApiDLL = NULL;
	}
	
	/* we have terminated gracefully */
	return TRUE;
}

/********************************************************************************/
/** UnHook: Temporarily Disables Hook and HotKeys.                             **/
/********************************************************************************/
void UnHook(){

	DisableIcon();
	DisableProc = TRUE;
	if (!Hooked) return;
	if (g_hInstalledLLKBDhook!=NULL){
		UnhookWindowsHookEx(g_hInstalledLLKBDhook);
		g_hInstalledLLKBDhook = NULL;
	}
	EnableWindow(GetDlgItem(ThisDlg,IDDISABLE),FALSE);
	EnableWindow(GetDlgItem(ThisDlg,IDAPPLY),TRUE);	

	Hooked = FALSE;
}

/********************************************************************************/
/** HookAgain: Restores after a call to UnHook().                              **/
/********************************************************************************/
void HookAgain(){

	EnableIcon();
	DisableProc = FALSE;
	if (Hooked) return;
	if (g_hInstalledLLKBDhook == NULL){
		g_hInstalledLLKBDhook = SetWindowsHookEx(WH_KEYBOARD_LL,
								LLKeyHookCallback, hInst, 0);
	}
	/* CAPS Key */
	_CAPS_ = GetKeyState(VK_CAPITAL) & 0x01;

	EnableWindow(GetDlgItem(ThisDlg,IDDISABLE),TRUE);
	EnableWindow(GetDlgItem(ThisDlg,IDAPPLY),FALSE);
	Hooked = TRUE;
}

/********************************************************************************/
/** SetPowerKey: Setups the On/Off Key.                                        **/
/********************************************************************************/
void SetPowerKey(UINT k){
	if (AdvancedDlg){
		SetDlgItemText(AdvancedDlg, IDC_NEWONOFF, _T("Setup New"));
		SetDlgItemText(AdvancedDlg, IDC_OFFBOX, KEY_NAME[k]);
	}
	WaitingPowerKey = FALSE;
	POWER_KEY = k;

	SaveOptions();
}
/* FILE FUNCTIONS ***************************************************************/
/********************************************************************************/
/** GetLayoutName: Opens a layout file and retrieves its name                  **/
/********************************************************************************/
WCHAR *GetLayoutName(WCHAR *dest, int len, WCHAR *f){
	WCHAR buff[21] = _T("\0");
	char sign[5];

	FILE *fh;
	fh = _wfopen(f, _T("rb"));

	memset(dest,0,len*sizeof(WCHAR));

	if (!fh) return dest;
	fgets(sign,5, fh);

	/* First File Format Version *****************/
	if (!strcmp(sign, SIGNATURE_V01)){;
		/* First come 20 WORDS for Layout name */
		fread(buff, sizeof(WCHAR), 20, fh);
	}
	fclose(fh);
	wcsncpy(dest, buff, (len<20? len : 20));
	return  dest;
}
/********************************************************************************/
/** LoadLayoutFile: Loads a OK-Keyboard Layout File.                           **/
/********************************************************************************/
BOOL LoadLayoutFile(WCHAR *f){
	int i;
	char sign[5];
	BYTE Unicodes;
	VK_COMBINATION UniCmb;

	FILE *fh;
	fh = _wfopen(f, _T("rb"));

	if (!fh) return FALSE;
	fgets(sign,5, fh);

	/* First File Format Version *****************/
	if (!strcmp(sign, SIGNATURE_V01)){
		TotalCombinations = 0;
		/* First come 20 WORDS for Layout name */
		fread(LayoutName, sizeof(WCHAR), 20, fh);

		/* Then comes VK mapping. Thanks God VK codes are single BYTES */
		for (i=0;i<256;i++) Map[i] = fgetc(fh);
		/* Now retrieve the next byte that says how many Unicode assignments
		are on the file */
		Unicodes = fgetc(fh);
		/* Finally start retrieving them */
		for (i=0;i<Unicodes;i++){
			fread(&UniCmb, sizeof(VK_COMBINATION), 1, fh);
			AddCombination(UniCmb.vMod,UniCmb.vkCode, UniCmb.unicode);
		}
	}
	fclose(fh);
	wcscpy(CurrentFile, f);
	if (PopUp) UpdateHeader();
	return TRUE;
}
/********************************************************************************/
/** SaveLayoutFile: Saves a OK-Keyboard Layout File.                           **/
/********************************************************************************/
BOOL SaveLayoutFile(WCHAR *f){
	int i;
	BYTE Unicodes = TotalCombinations;

	FILE *fh;
	fh = _wfopen(f, _T("wb"));

	if (!fh) return FALSE;

	/* First File Format Version *****************/
	fputs(SIGNATURE_V01, fh);
	fwrite(LayoutName, sizeof(WCHAR),20, fh);
	for (i=0;i<256;i++) fputc(Map[i], fh);
	fputc(Unicodes, fh);
	for (i=0;i<Unicodes;i++){
		fwrite(&Combinations[i], sizeof(VK_COMBINATION), 1, fh);
	}
	fclose(fh);
	return TRUE;
}
/* MAPPING FUNCTIONS ************************************************************/
/********************************************************************************/
/** type_a_number: Simulate key pressed for typing a given number.             **/
/********************************************************************************/
void type_a_number(UINT val){
	UINT n=val;
	int div;
	BYTE z, k;

	if (val > 10000) div = 10000;
	else if (val > 1000) div = 1000;
	else if (val > 100) div = 100;
	else if (val > 10) div = 10;
	else div = 1;

	while (div){
		z = n/div;
		if (UseNumpad) k = VK_NUMPAD0 + z;
		else k = '0' + z;
		key_press(k,0,0);
		key_press(k,0,KEYEVENTF_KEYUP);
		n -= z*div;
		div /= 10;
	}
}

/********************************************************************************/
/** GetFileName: Show the STD Open or Save File dialog and return the selection**/
/********************************************************************************/
WCHAR *GetFileName(WCHAR *fdest, WCHAR *desc, WCHAR *ext, HWND hW, BOOL Save){
	OPENFILENAME of;
	WCHAR buff[256];
	WCHAR filter[256];

	wcscpy(buff, LastFile);
	fdest[0] = 0;
	memset(&of, 0, sizeof(of));
	wsprintf(filter, _T("%s%c%s%c"), desc, 0, ext, 0);
	of.lStructSize = sizeof(OPENFILENAME);
	of.hInstance = hInst;
	of.hwndOwner = hW;
	of.lpstrInitialDir = LastDir;
	of.lpstrFilter = filter;
	of.lpstrFile = buff;
	of.nMaxFile = 256;

	if (Save){
		of.lpstrTitle = _T("OKey Layout Save ::");
		GetSaveFileName(&of);
	}else{
		of.lpstrTitle = _T("OKey Layout Load ::");
		of.Flags = OFN_PATHMUSTEXIST;
		GetOpenFileName(&of);
	}

	wcscpy(fdest, buff);
	wcsncpy (LastDir, buff,GetLastBSlash(buff));

	return fdest;
}
/* MISC FUNCTIONS ***************************************************************/
/********************************************************************************/
/** AskExit: Does the user really want to quit?                                **/
/********************************************************************************/
BOOL AskExit(HWND hD){
	int r = MessageBox(hD, _T("Do you really want to Exit OKey?"),
					_T("OKey Quit Confirmation"), MB_YESNO | MB_ICONQUESTION
					| MB_TOPMOST | MB_SETFOREGROUND);
	return (r == IDYES);
}

/********************************************************************************/
/** GetLastBSlash: Gets Last Backslash in a string.                            **/
/********************************************************************************/
int GetLastBSlash(WCHAR *st){
	int i, n=0;
	int len = wcslen(st);

	for (i=0;i<len;i++)
		if (st[i] == '\\') n = i;

	return n;
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
/** GetCheckBoxBool: Returns TRUE if a CheckBox is checked.                    **/
/********************************************************************************/
BOOL GetCheckBoxBool(HWND hdlg, int idc){
	return (SendDlgItemMessage(hdlg,idc,BM_GETCHECK,0,0) == BST_CHECKED);
}

/********************************************************************************/
/** CheckBoxIfBool: Sets a checkbox according a boolean value.                 **/
/********************************************************************************/
void CheckBoxIfBool(HWND hdlg, int idc, BOOL b){
	SendDlgItemMessage(hdlg,idc,BM_SETCHECK,(b? BST_CHECKED : BST_UNCHECKED),0);
}

/********************************************************************************/
/** TrapKeys: Create Hotkeys for trapping  Keypresses.                         **/
/********************************************************************************/
void TrapKeys(HWND hwnd){
	int i;

	if (Trapped) return;
	for (i=0;i<0xc1;i++)	RegisterHotKey(hwnd,i+1,0, i);
	for (i=0xc1;i<0xcf;i++) RegisterHotKey(hwnd,i+1, MOD_WIN, i);
	for (i=0xd0;i<0xfe;i++)	RegisterHotKey(hwnd,i+1,0, i);
	Trapped = TRUE;
}
/********************************************************************************/
/** ReleaseKeys: Release Hotkeys.                                              **/
/********************************************************************************/
void ReleaseKeys(HWND hwnd){
	int i;

	if (!Trapped) return;
	for (i=0;i<0xfe;i++) UnregisterHotKey(hwnd, i+1);
	Trapped = FALSE;
}

/********************************************************************************/
/** GetChecked: Gets the state of a Check Box.                                 **/
/********************************************************************************/
BOOL GetChecked(HWND Dlg, int CBoxID){
	return (SendDlgItemMessage(Dlg, CBoxID,BM_GETCHECK,0,0) == BST_CHECKED);
}
/* UNICODE COMBINATIONS HANDLING ************************************************/
/********************************************************************************/
/** ModCode: Returns the needed modifier for a hotkey.                         **/
/********************************************************************************/
UINT ModCode ( BOOL Alt, BOOL Win, BOOL Ctrl, BOOL Shift){
	UINT mCode = 0;
	if (Alt)  mCode = MOD_ALT;
	if (Win)  mCode |= MOD_WIN;
	if (Ctrl) mCode |= MOD_CONTROL;
	if (Shift)mCode |= MOD_SHIFT;
	return mCode;
}

/********************************************************************************/
/** UniKey2String: Gives the written form for a UNICODE Key Setup.             **/
/********************************************************************************/
WCHAR *Key2String(WCHAR *dest, UINT mod_code, int k, UINT uni){
	
	wsprintf(dest,_T("[%lc] "), uni);
	if (mod_code & MOD_WIN) wcscat(dest, _T("Win+"));
	if (mod_code & MOD_CONTROL) wcscat(dest, _T("Ctrl+"));
	if (mod_code & MOD_ALT) wcscat(dest, _T("Alt+"));
	if (mod_code & MOD_SHIFT) wcscat(dest, _T("Shift+"));
	wcscat(dest, KEY_NAME[k]);

	return dest;
}
/********************************************************************************/
/** AddCombination: Adds a UNICODE key sequence.                               **/
/********************************************************************************/
BOOL AddCombination(UINT mCode, BYTE vk, UINT uni){
	if (TotalCombinations >= 255) return FALSE;
	
	Combinations[TotalCombinations].vMod = mCode;
	Combinations[TotalCombinations].vkCode = vk;
	Combinations[TotalCombinations].unicode = uni;
	TotalCombinations++;
	return TRUE;
}

/********************************************************************************/
/** RefreshCombinations: Redraw the whole combination list.                    **/
/********************************************************************************/
void RefreshCombinations(){
	WCHAR kst[36];
	int i;

	SendDlgItemMessage(KeyMapDlg,IDC_COMBLIST, LB_RESETCONTENT,0,0);
	for (i=0;i<TotalCombinations;i++){
		Key2String(kst,Combinations[i].vMod, Combinations[i].vkCode,
				Combinations[i].unicode);
		SendDlgItemMessage(KeyMapDlg,IDC_COMBLIST, LB_ADDSTRING,0,(LPARAM)kst);
	}
}

/********************************************************************************/
/** DelCombination: Deletes a UNICODE registered combination.                  **/
/********************************************************************************/
BOOL DelCombination(int n){
	int i;

	if (TotalCombinations <= 0) return FALSE;


	for (i=n;i<TotalCombinations-1;i++){
		Combinations[n].vMod =Combinations[n+1].vMod;
		Combinations[n].vkCode = Combinations[n+1].vkCode;
		Combinations[n].unicode = Combinations[n+1].unicode;
	}
	TotalCombinations--;
	return TRUE;
}


/* FONT INTERFACING *************************************************************/
/********************************************************************************/
/** SetPreviewFont: Changes current font for UNICODE preview.                  **/
/********************************************************************************/
void SetPreviewFont(int n){

	if (FontHandle) DeleteObject(FontHandle);
	FontHandle = CreateFontIndirect(&SysFont[n]);
	if (FontHandle){
		SendDlgItemMessage( KeyMapDlg, IDC_UNIPREV, WM_SETFONT,
							(WPARAM)FontHandle, MAKELPARAM(TRUE,0));

		SendDlgItemMessage( KeyMapDlg, IDC_COMBLIST, WM_SETFONT,
							(WPARAM)FontHandle, MAKELPARAM(TRUE,0));
	}
}

/********************************************************************************/
/** EnumFontProc: Callback for Enumerating System Fonts.                       **/
/********************************************************************************/
int CALLBACK EnumFontProc(LOGFONT* lplf, TEXTMETRIC* lptm, DWORD dwType, LPARAM lpData ){
	WCHAR FaceName[36];
 	wsprintf(FaceName, _T("%s"), lplf->lfFaceName);

	lplf->lfHeight = 14;
	lplf->lfWidth  = 0;
	memcpy(&SysFont[TotalFonts], lplf, sizeof(LOGFONT));
	TotalFonts++;
	SendDlgItemMessage (KeyMapDlg,IDC_PREVIEWFONT,
						LB_ADDSTRING, 0, (LPARAM)FaceName);
	if (TotalFonts > 255) return 0;
	else return 1;
}

/* DIALOG INTERFACING ***********************************************************/
/********************************************************************************/
/** SelectKeyFromList: Selects in A and B boxes the same key.                  **/
/********************************************************************************/
void SelectKeyFromList(BOOL FromA){
	HWND hdlFrom = (FromA? MapHdlA : MapHdlB);
	HWND hdlTo  = (FromA? MapHdlB : MapHdlA);
	int sel = SendMessage(hdlFrom,LB_GETCURSEL,0,0);

	SendMessage(hdlTo, LB_SETCURSEL, sel, 0);
}

/********************************************************************************/
/** GetUnicodeFromEdit: Returns the numerical value in the Unicode edit box.   **/
/********************************************************************************/
UINT GetUnicodeFromEdit(){
	return SendDlgItemMessage(KeyMapDlg, IDC_EDITSPIN, UDM_GETPOS,0,0);
}
/********************************************************************************/
/** UpdateUnicode: Updates the Unicode Preview Box.                            **/
/********************************************************************************/
void UpdateUnicode(){
	UINT UniCode;
	WCHAR nbuff[2];

	UniCode = GetUnicodeFromEdit();
	nbuff[0] = UniCode;
	nbuff[1] = 0;
	SetDlgItemText(KeyMapDlg, IDC_UNIPREV, nbuff);
}

/********************************************************************************/
/** SetUniEditText: Sets the Unicode Edit box text.                            **/
/********************************************************************************/
void SetUniEditText(UINT n){
	WCHAR nbuff[5];
	wsprintf(nbuff,_T("%lu"), n);
	SetDlgItemText(KeyMapDlg, IDC_UNIEDIT, nbuff);
}
/********************************************************************************/
/** SelectKey: Selects a Key for Edition.                                      **/
/********************************************************************************/
void SelectKey(int k){
	int sel = SendMessage(MapHdlA,LB_GETCURSEL,0,0);
	
	SendMessage(MapHdlA,LB_SETCURSEL, k, 0);
	SelectKeyFromList(TRUE); 
	MappingKey = k;
}
/********************************************************************************/
/** HideDialog: Hide the Main Dialog, Also set the WeAreHidden flag.           **/
/********************************************************************************/
void HideDialog(){
	if (KeyMapDlg) ShowWindow(KeyMapDlg, SW_HIDE);
	if (AdvancedDlg) ShowWindow(AdvancedDlg, SW_HIDE);
	if (AboutDlg) ShowWindow(AboutDlg, SW_HIDE);
	ShowWindow(ThisDlg, SW_HIDE);
	WeAreHidden = TRUE;
}

/********************************************************************************/
/** ShowDialog: Shows the Main Dialog. Also clears the WeAreHidden flag.       **/
/********************************************************************************/
void ShowDialog(){
	ShowWindow(ThisDlg, SW_SHOW);
	SetForegroundWindow(ThisDlg);
	if (KeyMapDlg) {
		ShowWindow(KeyMapDlg, SW_SHOW);
		SetForegroundWindow(KeyMapDlg);
	}
	if (AdvancedDlg){
		ShowWindow(AdvancedDlg, SW_SHOW);
		SetForegroundWindow(AdvancedDlg);
	}
	if (AboutDlg) {
		ShowWindow(AboutDlg, SW_SHOW);
		SetForegroundWindow(AboutDlg);
	}
	WeAreHidden = FALSE;
}

/********************************************************************************/
/** FillMapAB: Fills Key lists A and B.                                        **/
/********************************************************************************/
void FillMapAB(){
	int i;

	SendMessage(MapHdlA, LB_INITSTORAGE, 256, 15);
	SendMessage(MapHdlB, LB_INITSTORAGE, 256, 15);

	for (i=0;i<256;i++){
		SendMessage(MapHdlA,LB_ADDSTRING, 0, (LPARAM)KEY_NAME[i]);
		SendMessage(MapHdlB,LB_ADDSTRING, 0, (LPARAM)KEY_NAME[Map[i]]);
		SendMessage(MapHdlK,LB_ADDSTRING, 0, (LPARAM)KEY_NAME[i]);
	}
}

/********************************************************************************/
/** UpdateMap: Updates Key mapping box.                                        **/
/********************************************************************************/
void UpdateMap(k){

	SendMessage(MapHdlB, LB_DELETESTRING, k, 0);
	SendMessage(MapHdlB,LB_INSERTSTRING, k, (LPARAM)KEY_NAME[Map[k]]);
}

/* TRAY INTERFACING *************************************************************/
#define PMENU_EXIT		255
#define PMENU_SEP2		253
#define PMENU_ENABLED	252
#define PMENU_SHOWDLG	251
#define PMENU_SEP0		250
#define PMENU_CURRENT	249
#define PMENU_LAYOUTS	1

/********************************************************************************/
/** MenuInsertItem: Inserts a string item in our popup Menu.                   **/
/********************************************************************************/
void MenuInsertItem(WCHAR *item, UINT id, DWORD moreFlags){
	InsertMenu(PopUp, 0,MF_BYPOSITION | MF_STRING | moreFlags,id, item);
	//AppendMenu(PopUp, MF_STRING,id, item);
}
/********************************************************************************/
/** MenuInsertSeparator: Inserts a separator in our popup menu.                **/
/********************************************************************************/
void MenuInsertSeparator(){
	InsertMenu(PopUp, 0,MF_BYPOSITION | MF_SEPARATOR ,0, NULL);
	//AppendMenu(PopUp, MF_SEPARATOR ,0, NULL);
}

/********************************************************************************/
/** ResetMenu: Resets Menu Content.                                            **/
/********************************************************************************/
void ResetMenu(){
	int i;
	for (i=0;i<TotalQA;i++){
		DeleteMenu(PopUp,PMENU_LAYOUTS+i,MF_BYCOMMAND);
	}
	/* Delete Header */
	DeleteMenu(PopUp,PMENU_CURRENT,MF_BYCOMMAND);
	DeleteMenu(PopUp,0,MF_BYPOSITION);
}


/********************************************************************************/
/** AddHeader: Adds the Header to PopUpMenu.                                   **/
/********************************************************************************/
void AddHeader(){
	MenuInsertSeparator();
	if (LayoutName[0])
		MenuInsertItem(LayoutName, PMENU_CURRENT,MF_CHECKED);
	else MenuInsertItem(_T("<None>"), PMENU_CURRENT,MF_CHECKED);
}

/********************************************************************************/
/** UpdateHeader: Update Popup Menu Header                                     **/
/********************************************************************************/
void UpdateHeader(){
	DeleteMenu(PopUp, PMENU_CURRENT, MF_BYCOMMAND); /* It should be 0 anyway */

	if (LayoutName[0])
		MenuInsertItem(LayoutName, PMENU_CURRENT,MF_CHECKED);
	else MenuInsertItem(_T("<None>"), PMENU_CURRENT,MF_CHECKED);
}

/********************************************************************************/
/** AddQAMenu: Adds Quick Access entries to the Popup menu                     **/
/********************************************************************************/
void AddQAMenu(){
	int i;

	for (i=0;i<TotalQA;i++){
		MenuInsertItem(QuickAccess[i].name, PMENU_LAYOUTS+i,0);
	}
}

/********************************************************************************/
/** CreatePopUp: Creates the systray menu with the basic options.              **/
/********************************************************************************/
void CreatePopUp(){
	PopUp = CreatePopupMenu();
	if (PopUp){
		MenuInsertItem(_T("Exit"), PMENU_EXIT,0);
		MenuInsertSeparator();
		MenuInsertItem(_T("Enabled"), PMENU_ENABLED, MF_CHECKED);
		MenuInsertItem(_T("Show OKey"), PMENU_SHOWDLG,0);
		MenuInsertSeparator();
	}
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
	mID = TrackPopupMenu(PopUp,TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RETURNCMD|
						TPM_NONOTIFY,rct.right, rct.top, 0, ThisDlg, NULL);
	return mID;
}

/********************************************************************************/
/** DisableIcon: Put the disabled icon in the system tray.                     **/
/********************************************************************************/
void DisableIcon(){
	TrayIconModify(ThisDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON2)), NULL);
	if (PopUp)
		CheckMenuItem(PopUp, PMENU_ENABLED ,MF_BYCOMMAND | MF_UNCHECKED);
}


/********************************************************************************/
/** EnableIcon: Put the enabled icon in the system tray.                       **/
/********************************************************************************/
void EnableIcon(){
	TrayIconModify(ThisDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON)), NULL);
	if (PopUp)
		CheckMenuItem(PopUp, PMENU_ENABLED ,MF_BYCOMMAND | MF_CHECKED);
}

/********************************************************************************/
/** TrayMessage: Sends a Message to System Tray.                               **/
/********************************************************************************/
BOOL TrayMessage(HWND hwnd, DWORD dwMessage, UINT uID, HICON hIcon, PTSTR pszTip){
	BOOL res = FALSE;
	NOTIFYICONDATA tnd;
  
	tnd.cbSize		= sizeof(NOTIFYICONDATA);
	tnd.hWnd		= hwnd;
	tnd.uID			= uID;
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
/** TrayIconModify: Modifies a Tray Icon.                                      **/
/********************************************************************************/
void TrayIconModify(HWND hwnd, UINT uID, HICON hIcon, PTSTR pszTip){
	TrayMessage(hwnd, NIM_MODIFY, uID, hIcon, NULL);
}

/********************************************************************************/
/** TrayIconAdd: Adds a icon to System Traybar.                                **/
/********************************************************************************/
void TrayIconAdd(HWND hwnd, UINT uID, HICON hIcon, PTSTR pszTip){
	TrayMessage(hwnd, NIM_ADD, uID,  hIcon, NULL);
}

/* ENTRY POINT ******************************************************************/
/********************************************************************************/
/** WinMain: The Default Entry Point                                           **/
/********************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR lpCmd, int nShow){
	int retcode, i;
	HWND pr;


	hInst = hInstance;
	InitCommonControls();

	
	pr = FindWindow(_T("Dialog"),_T("OKey Keyboard Layout Utility"));
	if (pr){
		if (lpCmd[0]){
			SendMessage(pr,WM_LOADLAYOUT, 0,(WPARAM)lpCmd);
		}
		exit(EXIT_SUCCESS);
	}

	#ifdef DEBUG_ENABLE
	InitDebug("\\oklog.txt");
	#endif

	if (lpCmd[0]) LoadLayoutFile(lpCmd);
	GetModuleFileName(hInstance, OKeyEXE, 256);
	/* Initialize Map */
	for (i=0;i<256;i++) Map[i] = i;
	retcode = DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG),
						NULL, (DLGPROC)MainDlgProc);

	#ifdef DEBUG_ENABLE
	EndDebug();
	#endif

	return FALSE;
}


/* DIALOGS CALLBACK *************************************************************/
/********************************************************************************/
/** AdvancedDlgProc: Advanced Setup Dialog Message Handler.                    **/
/********************************************************************************/
BOOL WINAPI AdvancedDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId;
	int sel;
	WCHAR fbuff[256], nbuff[24];


	switch(msg){
		case WM_INITDIALOG:
			AdvancedDlg = hDlg;
			SetPowerKey(POWER_KEY);
			CheckBoxIfBool(hDlg, IDC_NEVERESC, NoParseESC);
			CheckBoxIfBool(hDlg, IDC_USENUMPAD, UseNumpad);
			ReloadQAList();
			ResetMenu();
			break;

		case WM_CLOSE:
			AddQAMenu();
			AddHeader();
			WaitingPowerKey = FALSE;
			if (DisableProc) {
				DisableProc = FALSE;
				EnableIcon();
			}
			AdvancedDlg = NULL;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case WM_PAINT:
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			switch (wmId){
				case IDC_OKADD:
					if (TotalQA > 9){
						MessageBox(hDlg, _T("Can't add more than 10 Entries!!"),
										_T("OKey Msg"),MB_OK | MB_ICONEXCLAMATION);
						break;
					}
					GetFileName(fbuff, _T("OK Layout Files (*.ok)"), _T("*.ok"), hDlg, TRUE);
					if (fbuff[0]){
						GetLayoutName(nbuff, 20, fbuff);
						if (nbuff[0]!='\0'){
							AddQuickAccess(fbuff, nbuff);
							SendDlgItemMessage(hDlg, IDC_QALIST, LB_ADDSTRING, 0,
								(LPARAM)fbuff);
							SaveOptions();
						}
					}
				break;
				case IDC_NEVERESC:
					NoParseESC = GetCheckBoxBool(hDlg, IDC_NEVERESC);
					SaveOptions();
				break;
				case IDC_USENUMPAD:
					UseNumpad = GetCheckBoxBool(hDlg, IDC_USENUMPAD);
					SaveOptions();
				break;
				case IDC_OKDEL:
					sel = SendDlgItemMessage(hDlg,IDC_QALIST,LB_GETCURSEL,0,0);					
					if (DelQuickAccess(sel)){
						SendDlgItemMessage(hDlg,IDC_QALIST,
											LB_RESETCONTENT,0,0);
						ReloadQAList();
						SaveOptions();
					}
					
				break;
				case IDCANCEL:
				case IDC_OK:
					SendMessage(hDlg,WM_CLOSE,0,0);
				break;
				case IDC_NEWONOFF:
					SetDlgItemText(hDlg, IDC_NEWONOFF, _T("Turn Off your unit"));
					WaitingPowerKey=TRUE;
					DisableProc = TRUE;
					DisableIcon();
				break;

				default:
					DefWindowProc(hDlg, msg, wParam, lParam);
			}
	}
	return FALSE;
}

/********************************************************************************/
/** AboutDlgProc: About Dialog Message Handler.                                **/
/********************************************************************************/
BOOL WINAPI AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId;

	switch(msg){
		case WM_INITDIALOG:
			AboutDlg = hDlg;
			SetDlgItemText(hDlg,IDC_VERSION, _T(CURRENT_VERSION_STRING));
			break;

		case WM_CLOSE:
			AboutDlg = NULL;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case WM_PAINT:
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			switch (wmId){
				case IDCANCEL:
					SendMessage(hDlg,WM_CLOSE,0,0);
				break;
				default:
					DefWindowProc(hDlg, msg, wParam, lParam);
			}
	}
	return FALSE;
}
/********************************************************************************/
/** MapDlgProc: Key Mapping Dialog Message Handler.                            **/
/********************************************************************************/
BOOL WINAPI MapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	int sel;
	int nKey;
	BYTE ksel;
	WCHAR buff[256];
	WCHAR ext[4];
	UINT UniCode;
	HDC DeskhDC, hDc;
	HWND desk;
	UINT mods;
	WCHAR kst[36];
	HWND Spin;

	switch(msg){
		case WM_HOTKEY:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (msg != WM_HOTKEY){
				nKey = (int)wParam;
				mods = (HIBYTE(HIWORD(lParam)) & 0x20 ? MOD_ALT : 0);
			}else{
				nKey = (UINT) HIWORD(lParam);
				mods = (UINT) LOWORD(lParam);
			}

			if (WaitingKeyA){
				SetDlgItemText(hDlg, IDC_MAPNEWBTTN, _T("New"));
				SelectKey(nKey);
				SendMessage(hDlg, WM_COMMAND, IDC_MAP2BTTN, 0);
				SetFocus(hDlg);
			}else if (WaitingKeyB){
				Map[MappingKey] = nKey;
				WaitingKeyB = FALSE;
				UpdateMap(MappingKey);
				SetDlgItemText(hDlg, IDC_MAP2BTTN, _T("=>"));
				SendMessage(MapHdlB, LB_SETCURSEL, MappingKey, 0);
				MappingKey = 0;
				ReleaseKeys(hDlg);
			}else if (WaitingKeyC){
				if (mods & MOD_ALT || nKey == VK_MENU) {
					SendDlgItemMessage( hDlg, IDC_ALTCHECK,
										BM_SETCHECK,BST_CHECKED,0);
				}else if (mods & MOD_SHIFT || nKey == VK_SHIFT){
					SendDlgItemMessage( hDlg, IDC_SHIFTCHECK,
										BM_SETCHECK,BST_CHECKED,0);
				}else if
					(mods & MOD_CONTROL || nKey == VK_CONTROL){
					SendDlgItemMessage( hDlg, IDC_CTRLCHECK,
										BM_SETCHECK,BST_CHECKED,0);
				}else if
					(mods & MOD_WIN || nKey == VK_LWIN || nKey == VK_RWIN){
					SendDlgItemMessage( hDlg, IDC_WINCHECK,
										BM_SETCHECK,BST_CHECKED,0);
				}
				CKey = nKey;
				SetDlgItemText(hDlg,IDC_KEYBOX, KEY_NAME[CKey]);
				WaitingKeyC = FALSE;
				SetDlgItemText(hDlg, IDC_PRESSKEY, _T("Press a Key"));

				ReleaseKeys(hDlg);
			}
			break;
		case WM_PAINT:
			break; /* Let DefDlgProc Handle painting */
			
		case WM_CLOSE:
			ReleaseKeys(hDlg);
			GetDlgItemText(hDlg,IDC_LAYOUTNAME,LayoutName,20);
			KeyMapDlg = NULL;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case WM_INITDIALOG:
			WaitingKeyA = FALSE;
			WaitingKeyB = FALSE;
			WaitingKeyC = FALSE;

			KeyMapDlg = hDlg;
			MapHdlA = GetDlgItem(hDlg, IDC_KEYLISTA);
			MapHdlB = GetDlgItem(hDlg, IDC_KEYLISTB);
			MapHdlK = GetDlgItem(hDlg, IDC_SIMKEY);
			/* Set Dialog Controls */
			FillMapAB();
			SendDlgItemMessage(hDlg, IDC_LAYOUTNAME, EM_SETLIMITTEXT,19,0);
			SetDlgItemText(hDlg,IDC_LAYOUTNAME, LayoutName);
			SelectKey(0);

			SendDlgItemMessage(hDlg, IDC_UNIEDIT, EM_SETLIMITTEXT,6,0);
			SetUniEditText(65);

			/* Attach the SpinBox to the UNICODE Edit Box */
			Spin = GetDlgItem(hDlg, IDC_EDITSPIN);
			SendMessage(Spin, UDM_SETBUDDY,
						(WPARAM)GetDlgItem(hDlg,IDC_UNIEDIT), 0);
			SendMessage(Spin,UDM_SETBASE,10,0);
			SendMessage(Spin,UDM_SETRANGE32,0,65535);
			SendMessage(Spin,UDM_SETPOS,0,(LPARAM)MAKELONG(65,0));

			/* Enumerate Fonts */
			desk = GetWindow(ThisDlg, GW_OWNER);
			DeskhDC = GetDC(desk);
			hDc = CreateCompatibleDC(DeskhDC);
			ReleaseDC (desk, DeskhDC);
			EnumFontFamilies (hDc, NULL, (FONTENUMPROC)EnumFontProc, 0);
			DeleteDC(hDc);

			SendDlgItemMessage(hDlg,IDC_PREVIEWFONT, LB_SETCURSEL, TotalFonts-1,0);
			SetPreviewFont(TotalFonts-1);
			RefreshCombinations();
			return TRUE;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam);

			switch (wmId){
				case IDC_SIMULATEKEY:
					ksel = (BYTE)SendMessage(MapHdlK,LB_GETCURSEL,0,0);
					SetFocus(hDlg);
					keybd_event(ksel,0,0,0);
					keybd_event(ksel,0,KEYEVENTF_KEYUP,0);
				return TRUE;
				case IDC_KEYLISTA:
					SelectKeyFromList(TRUE);
				break;

				case IDC_KEYLISTB:
					SelectKeyFromList(FALSE);
				break;

				case IDC_MAPNEWBTTN:
					SetDlgItemText(hDlg, IDC_MAPNEWBTTN, _T("..."));
					TrapKeys(hDlg);
					WaitingKeyA = TRUE;
					WaitingKeyB = FALSE;
					SetFocus(hDlg);
					break;
				case IDSAVE:
					GetFileName(buff, _T("OK Layout Files (*.ok)"), _T("*.ok"), hDlg, TRUE);
					if (buff[0]){
						GetDlgItemText(hDlg,IDC_LAYOUTNAME,LayoutName,20);
						wcscpy(ext,(WCHAR *)&buff[wcslen(buff)-3]);
						_wcslwr(ext);
						if (wcscmp(ext, _T(".ok"))) wcscat(buff, _T(".ok"));
						if (SaveLayoutFile(buff)){
							wcscpy(LastFile, buff);
						}
					}
				break;
				case IDCANCEL:
					SendMessage(hDlg,WM_CLOSE,0,0);
					break;

				case IDC_MAP2BTTN:
					sel = SendMessage(MapHdlA, LB_GETCURSEL, 0, 0);
					if (sel!= LB_ERR){
						WaitingKeyA = FALSE;
						SetDlgItemText(hDlg, IDC_MAP2BTTN, _T("..."));
						SelectKey(sel);
						TrapKeys(hDlg);
						WaitingKeyB = TRUE;
						SetFocus(hDlg);
					}
					break; 

				case IDC_UNIADD:
					mods = ModCode(
							GetChecked(hDlg, IDC_ALTCHECK),
							GetChecked(hDlg, IDC_WINCHECK),
							GetChecked(hDlg, IDC_CTRLCHECK),
							GetChecked(hDlg, IDC_SHIFTCHECK));

					UniCode = GetUnicodeFromEdit();
					Key2String(kst,mods, CKey,UniCode);
					if (AddCombination(mods,CKey, UniCode)){
						SendDlgItemMessage(hDlg,IDC_COMBLIST,
											LB_ADDSTRING,0,(LPARAM)kst);
					}
					
				break;
				case IDC_UNIDEL:
					sel = SendDlgItemMessage(hDlg,IDC_COMBLIST,LB_GETCURSEL,0,0);					
					if (DelCombination(sel)){
						SendDlgItemMessage(hDlg,IDC_COMBLIST,
											LB_RESETCONTENT,0,0);
						RefreshCombinations();
					}
					
				break;
				case IDC_PRESSKEY:
					SetDlgItemText(hDlg, IDC_PRESSKEY, _T("..."));
					WaitingKeyC = TRUE;
					TrapKeys(hDlg);
					SetFocus(hDlg);
				break;
				case IDC_UNIEDIT:
					UpdateUnicode();
				break;
				case IDC_PREVIEWFONT:
					SetPreviewFont( SendDlgItemMessage(hDlg,IDC_PREVIEWFONT,
									LB_GETCURSEL,0,0));
				break;

				default:
				   return DefWindowProc(hDlg, msg, wParam, lParam);
			}
			return TRUE;
			break;
	}
	return FALSE;
}
/********************************************************************************/
/** MainDlgProc: Main Dialog Message Processing.                               **/
/********************************************************************************/
BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	WCHAR buff[256];
	BOOL OldState;
	int mId;

	switch(msg){
		case WM_PAINT:
			if (StartingApp){
				if (LoadLast){
					if (!CurrentFile[0]) LoadLayoutFile(LastFile);
						SendDlgItemMessage (hDlg,IDC_LOADLASTCHECK, 
											BM_SETCHECK, BST_CHECKED,0);
				}
				/* Did we load a keyboard layout? */
				if (CurrentFile[0]) {
					wcscpy(LastFile, CurrentFile);
					SetDlgItemText(hDlg,IDC_FILEBOX, LayoutName);
					SaveOptions();
				}
				if (StartMinimized) HideDialog();
				StartingApp = FALSE;
			}
		break; /* Let DefDlgProc Handle painting */
		
		case WM_LOADLAYOUT:
			wcscpy(buff, (WCHAR*)lParam);
			if (LoadLayoutFile(buff)){
				wcscpy(LastFile, CurrentFile);
				SetDlgItemText(hDlg,IDC_FILEBOX, LayoutName);
				SaveOptions();
			}
		return TRUE;
		case WM_CLOSE:
			if (AskExit(hDlg)){
				DeactivateKBHook();
				if (KeyMapDlg) EndDialog(KeyMapDlg, TRUE);
				if (AboutDlg) EndDialog(AboutDlg, TRUE);
				TrayIconDelete(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON)), NULL);
				EndDialog(hDlg, TRUE);
			}
			return TRUE;

		case WM_INITDIALOG:
			ThisDlg = hDlg;
			CreatePopUp();
			LoadOptions();
			if (TotalQA) AddQAMenu();
			AddHeader();

			if(!ActivateKBHook(hInst, LLKeyHookCallback)){
				MessageBox(hDlg, 
				_T("Couldn't Install Keyboard Hook!"), 
				_T("Critical Error"), MB_OK);
				exit(1);
			}
			TrayIconAdd(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON)), NULL);
			
			/* Are OK files registered? */
			if (AreOKFilesRegistered())
				SendDlgItemMessage(hDlg,IDC_ASSOCIATEFILES,BM_SETCHECK,BST_CHECKED,0);
			/* Does the startup shortcut exist? */
			if (FileExists(OKSHORTCUT)) CheckBoxIfBool(hDlg, IDC_RUNSTARTUP, TRUE);
			//ShowDialog();
			if (StartMinimized) CheckBoxIfBool(hDlg, IDC_STARTMINIMIZED, TRUE);
			return TRUE;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam);

			switch (wmId){
				case IDC_RUNSTARTUP:
					if (GetCheckBoxBool(hDlg,IDC_RUNSTARTUP)){
						wsprintf(buff, _T("\"%ls\""), OKeyEXE);
						SHCreateShortcut(OKSHORTCUT, buff);
					}else DeleteFile(OKSHORTCUT);
					break;
				case IDC_ADVANCED:
					EnableWindow(hDlg, FALSE);
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ADVANCED),
						NULL, (DLGPROC)AdvancedDlgProc);
					EnableWindow(hDlg, TRUE);
					SetForegroundWindow(hDlg);
				break;
				case IDC_ASSOCIATEFILES:
					if (GetChecked(hDlg,IDC_ASSOCIATEFILES)){
						Register4OKFiles();
					}else{
						UnRegister4OKFiles();
					}
				break;

				case IDABOUT:
					EnableWindow(hDlg, FALSE);
					DialogBox ( hInst, MAKEINTRESOURCE(IDD_ABOUT),
								NULL, (DLGPROC)AboutDlgProc);
					EnableWindow(hDlg, TRUE);
					SetForegroundWindow(hDlg);
				break;
				case IDC_LOADLASTCHECK:
					LoadLast = GetChecked(hDlg, IDC_LOADLASTCHECK);
					SaveOptions();
				break;
				case IDC_STARTMINIMIZED:
					StartMinimized = GetChecked(hDlg, IDC_STARTMINIMIZED);
					SaveOptions();
				break;
				case IDAPPLY:
					HookAgain();
				break;

				case IDDISABLE:
					UnHook();
				break;

				case IDLOAD:
					GetFileName(buff, _T("OK Layout Files (*.ok)"), _T("*.ok"), hDlg, FALSE);
					if (buff[0]){
						if (LoadLayoutFile(buff)) {
							wcscpy(LastFile, buff);
							SetDlgItemText(hDlg,IDC_FILEBOX, LayoutName);
							SaveOptions();
						}
					}
					
					break;
				case IDMINIMIZE:
					HideDialog();
				break;
				case IDEDIT:
					OldState = Hooked;
					UnHook();
					EnableWindow(hDlg, FALSE);
					DialogBox(hInst, MAKEINTRESOURCE(IDD_KEYLAYDLG), NULL, (DLGPROC)MapDlgProc);
					EnableWindow(hDlg, TRUE);
					SetForegroundWindow(hDlg);
					if (OldState) HookAgain();
				break;
				default:
				   return DefWindowProc(hDlg, msg, wParam, lParam);
			}
			return TRUE;
			break;

		case TRAY_NOTIFYICON:
			switch (lParam){
				case WM_LBUTTONDBLCLK:
					if (wParam == ID_TRAY){
						if (WeAreHidden) ShowDialog();
						else HideDialog();
						return (TRUE);
					}
				break;
				case WM_LBUTTONDOWN:
					if (wParam == ID_TRAY && WeAreHidden){
						mId = DisplayMenu();
						switch (mId){
							case PMENU_EXIT:
								SendMessage(hDlg, WM_CLOSE,0,0);
							break;
							case PMENU_SHOWDLG:
								ShowDialog();
							break;
							case PMENU_ENABLED:
								if (DisableProc && AdvancedDlg == NULL &&
									KeyMapDlg ==NULL) HookAgain();
								else UnHook();
							break;
							default:
								mId -= PMENU_LAYOUTS;
								if (mId >= 0 && mId < TotalQA){
									if (LoadLayoutFile(
										QuickAccess[mId].file)) {
										wcscpy(LastFile, CurrentFile);
										SetDlgItemText(hDlg,IDC_FILEBOX, LayoutName);
										SaveOptions();
									}
								}
						}
						return (TRUE);
					}
			}
			break;
	}
	return FALSE;
}
