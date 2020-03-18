#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <stdlib.h>
#include <commctrl.h>
#include <types.h>	// For UINT8 etc.
#include "pegdser.h"  // IOCTL_SERIAL stuff.
#define PENSIZE		4
#define TRAY_NOTIFYICON WM_USER + 2001
#define ID_TRAY	5000
#define CORRECT_INCR(t, dt, tmax) dt = (t+dt < 0? -t : (t+dt > tmax? tmax-t : dt))
#define MB_CUSTOM_STYLE		MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND

HINSTANCE	hInst;
HWND ThisDlg;		/* Dialog Handle */

HPEN hPen;
POINT mousep;
RECT CursorRC;

const UINT8 gcB0BitMask = 0x20;
const UINT8 gcB1BitMask = 0x10;


HANDLE g_hReadThread;
HANDLE g_hDrawThread;
HANDLE g_hSerial;
HANDLE g_hThreadStopped;
BOOL   g_fStopThread;
BOOL	SwapButtons = FALSE;
BOOL	DoMirrorX = FALSE;
BOOL	DoMirrorY = FALSE;
BOOL	DoRotate = FALSE;

ULONG CALLBACK ReadThread(PVOID pv);
ULONG CALLBACK DrawThread(PVOID pv);
BOOL SendData(UINT8 rawData[]);
BOOL InitSerialPort();
COLORREF bgpixel;
TCHAR COM_PORT[16];


BOOL WeAreHidden = FALSE;
BOOL DriverLoaded = FALSE;
BOOL IniHide = FALSE;
BOOL Detected = FALSE;
HWND ComPortBox;

DWORD           oldBaudRate;
BYTE            oldParity;
BYTE            oldByteSize;
BYTE            oldStopBits;

int SCREENW;
int SCREENH;

/* Function Prototypes **********************************************************/
BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* DIALOG INTERFACING ***********************************************************/
/********************************************************************************/
/** HideDialog: Hide the Main Dialog, Also set the WeAreHidden flag.           **/
/********************************************************************************/
void HideDialog(){
	ShowWindow(ThisDlg, SW_HIDE);
	WeAreHidden = TRUE;
}

/********************************************************************************/
/** ShowDialog: Shows the Main Dialog. Also clears the WeAreHidden flag.       **/
/********************************************************************************/
void ShowDialog(){
	ShowWindow(ThisDlg, SW_SHOW);
	SetForegroundWindow(ThisDlg);
	WeAreHidden = FALSE;
}

void CloseAndRestoreSerial(){
	DCB dcb;

	dcb.BaudRate = oldBaudRate;
	dcb.Parity   = oldParity;
    dcb.ByteSize = oldByteSize;
	dcb.StopBits = oldStopBits; 

	SetCommState(g_hSerial, (LPDCB)&dcb);
	CloseHandle(g_hSerial);
	g_hSerial = NULL;
}

/* MOUSE INTERFACING ************************************************************/
BOOL DetectSerialMouse(TCHAR * szPort, BOOL CleanBefore){
    const DWORD     cBaudRate = 1200;
    const BYTE      cParity   = NOPARITY;   // No parity
    const BYTE      cByteSize = 7; 
    const BYTE      cStopBits = ONESTOPBIT; // 1 stop bit

    ULONG           windowsError;
    DWORD           dwTemp;
    DCB             dcb;
    BOOL            bSuccess;
    BYTE            serialByte = 0x00;

    g_hSerial = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (g_hSerial == INVALID_HANDLE_VALUE) {
        g_hSerial = NULL;
        return FALSE;
    }

    bSuccess = GetCommState(g_hSerial, (LPDCB)&dcb);
    if (!bSuccess) {
        CloseHandle(g_hSerial);
        g_hSerial = NULL;
        return FALSE;
    }


    oldBaudRate = dcb.BaudRate;		dcb.BaudRate = cBaudRate;
    oldParity = dcb.Parity;			dcb.Parity = cParity;
    oldByteSize = dcb.ByteSize;		dcb.ByteSize = cByteSize;
    oldStopBits = dcb.StopBits;		dcb.StopBits = cStopBits;

    if (!SetCommState(g_hSerial, (LPDCB)&dcb)) {
        CloseHandle(g_hSerial);
        g_hSerial = NULL;
        return FALSE;
    }
	InitSerialPort();

	// Reset RTS and DTR and wait. Just for weird cases.
	DeviceIoControl(g_hSerial, IOCTL_SERIAL_CLR_DTR, NULL, 0, NULL, 0, &dwTemp, NULL);
	DeviceIoControl(g_hSerial, IOCTL_SERIAL_CLR_RTS, NULL, 0, NULL, 0, &dwTemp, NULL);
	Sleep(150);

    DeviceIoControl(g_hSerial, IOCTL_SERIAL_CLR_RTS, NULL, 0, NULL, 0, &dwTemp, NULL);
    DeviceIoControl(g_hSerial, IOCTL_SERIAL_SET_DTR, NULL, 0, NULL, 0, &dwTemp, NULL);
    Sleep(150);

	if (CleanBefore){
		dwTemp = 1;
		while ( dwTemp != 0 ) {
	        // Delete the serial port input queue.
		    ReadFile(g_hSerial, &serialByte, 1, &dwTemp, NULL);
		}
	}

    DeviceIoControl(g_hSerial, IOCTL_SERIAL_SET_RTS, NULL, 0, NULL, 0, &dwTemp, NULL);
	Sleep(10);

    dwTemp = 1;
    while (dwTemp != 0) {
		bSuccess = ReadFile(g_hSerial, &serialByte, 1, &dwTemp, NULL);
        // 7-bit data only.
        serialByte &= 0x7F;

        if (!bSuccess) {
            windowsError = GetLastError();
            CloseAndRestoreSerial();
            return FALSE;
        }
        if ( serialByte == 'M' || serialByte == 'B') {
            //
            // We've successfully found that a mouse is attached. We can now
            // return true and be done with it.  We'll leave the phSerial 
            // connected with the baudrate etc set.  Deinit will close the port.
            //
          
            // Setup the serial port parameters (timeouts, etc).
            //
			return TRUE;
        }
    }

    // Didn't find a mouse, so put the serial port back into its old state and
    // disconnect.
    CloseAndRestoreSerial();
    return FALSE;
}


void DrawCursorShape(HDC hdc, int x, int y){
	POINT pt[2];

	pt[0].x = x;
	pt[0].y = y;
	pt[1].x = pt[0].x+PENSIZE;
	pt[1].y = pt[0].y;

	SetROP2(hdc,R2_NOT);
	Polyline(hdc, pt,2);
}

void DrawCursor(int dx, int dy){
	HDC hdc;

	hdc = GetDC(NULL);
	SelectObject(hdc, hPen);
	if (GetPixel(hdc, mousep.x-dx, mousep.y-dy)==bgpixel){
		DrawCursorShape(hdc,mousep.x-dx,mousep.y-dy);
	}
	DrawCursorShape(hdc,mousep.x,mousep.y);
	bgpixel = GetPixel(hdc, mousep.x,mousep.y);
	
	ReleaseDC(NULL, hdc);
}


BOOL InitSerialPort(){
    // DetectSerialMouse connected our g_hSerial handle to the
    // serial driver and set the baudrate etc correctly.  Now we need to
    // set the comm timeouts to something acceptable for the mouse.
    COMMTIMEOUTS ctmo;

    if (!GetCommTimeouts(g_hSerial, &ctmo)) return (FALSE);

    ctmo.ReadIntervalTimeout = MAXDWORD;
    ctmo.ReadTotalTimeoutMultiplier = 0;
    ctmo.ReadTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(g_hSerial, &ctmo)) return (FALSE);
    if (!SetCommMask (g_hSerial, EV_RXCHAR)) return (FALSE);
	
    return (TRUE);
}

BOOL Driver_Init(){
	g_hReadThread = NULL;
	g_hDrawThread = NULL;
    g_hSerial = (HANDLE) NULL;
    g_hThreadStopped = CreateEvent (0, FALSE, FALSE, 0);
    g_fStopThread = FALSE;

	// An old mouse I tested didn't work with the code that cleans
	// the Input Queue. However, another (more modern) mouse didn't work
	// without it. So, try both methods if the first fail.
	Detected = DetectSerialMouse(COM_PORT, FALSE);
	if (!Detected) Detected = DetectSerialMouse(COM_PORT, TRUE);

	if (Detected) {
		if (MessageBox(NULL, TEXT("A Serial Mouse Has been detected. Do you want to use it?"),
				TEXT("Serial Mouse Detected"),MB_YESNO | MB_CUSTOM_STYLE)== IDYES){
			mousep.x = SCREENW/2;
			mousep.y = SCREENH/2;
			hPen = CreatePen(PS_SOLID, PENSIZE, RGB(0,0,0));
			g_hReadThread = CreateThread(NULL, 0, ReadThread, 0, 0, NULL);
			g_hDrawThread = CreateThread(NULL, 0, DrawThread, 0, 0, NULL);
			DriverLoaded = TRUE;
			return TRUE;
		} else return FALSE;
	}else{
		MessageBox(NULL, TEXT("NO Serial Mouse detected!"), TEXT("Alert"), MB_OK | MB_CUSTOM_STYLE);
		return TRUE;
	}
}

void Driver_DeInit(){
	//
    // Tell the ReadThread to shut down as soon as possible.
    //
    g_fStopThread = TRUE;

    if (g_hSerial) {
        // 
        // WakeUp the WaitCommEvent if necessary
        //
        SetCommMask (g_hSerial, 0);
    }

    WaitForSingleObject(g_hThreadStopped, 2000);  // Don't wait forever. 
    CloseHandle(g_hThreadStopped);

    if (g_hSerial != NULL) CloseAndRestoreSerial();

    if ( g_hReadThread != NULL ) {
        CloseHandle(g_hReadThread);
        g_hReadThread = NULL;
    }

	DeleteObject(hPen);
}

ULONG CALLBACK  DrawThread(PVOID pv){
	int lx=mousep.x, ly=mousep.y;

	while (!g_fStopThread) {	
		DrawCursor(mousep.x-lx,mousep.y-ly);
		lx = mousep.x;
		ly = mousep.y;
		Sleep(25);
	}
	return 1;
}

ULONG CALLBACK  ReadThread(PVOID pv){
    const UINT8 cSyncBitMask = 0x40;
    UINT8   serialBytes[3];
    UINT8   rawData[3];
    UINT8   curByte = 0;
    BOOL    bSuccess;
    DWORD   dwBytesRead;
    DWORD   count;
    DWORD   fdwCommMask;

    //
    // The thread can be exitted by setting g_fStopThread.
    //
    while (!g_fStopThread) {
        //
        // We spend most of our time waiting for a serial event here.
        //
        WaitCommEvent (g_hSerial, &fdwCommMask, 0);
        if (fdwCommMask & EV_RXCHAR) {
            //
            // We've woken up due to a real character arriving.
            //
            do {
                //
                // Continue to read the serial port while data is available.
                //
                bSuccess = ReadFile(g_hSerial, &serialBytes, 3, &dwBytesRead, NULL);
                //
                // Only get 7-bits of data back from ReadFile here.
                //
                serialBytes[0] &= 0x7F;
                serialBytes[1] &= 0x7F;
                serialBytes[2] &= 0x7F;

                if (!bSuccess) {
                } else if (dwBytesRead == 0) {
                    //
                    // Timed out with no data read.  Do nothing.
                    //
                } else {
                    //
                    // Successfully received commands from the mouse.
                    //
                    for ( count = 0; count < dwBytesRead; count++ ) {
                        // Pick the byte up
                        rawData[curByte] = serialBytes[count];
                        switch ( curByte ) {
                        // First byte has the sync bit set, next two don't.
                        case 0:
                            if ( (rawData[0] & cSyncBitMask) != 0 ) {
                                curByte++;
                            }
                            break;
                        case 1:
                            if ( (rawData[1] & cSyncBitMask) == 0 ) {
                                curByte++;                          
                            } else {
                                curByte = 0;
                            }
                            break;
                        case 2:
                            if ( (rawData[2] & cSyncBitMask) == 0 ) {
                                SendData(rawData);
                            }
                            curByte = 0;
                            break;
                        }
                    }
                }

            } while ( (!g_fStopThread) && dwBytesRead && bSuccess );
        }
    }

    //
    // Signal to Deinit that we are exiting now, so it's safe to cleanup.
    //
    SetEvent (g_hThreadStopped);
    return (1);
}   



void swap_bool(BOOL *b1, BOOL *b2){
	BOOL t = *b1;
	*b1 = *b2;
	*b2 = t;
}

// -----------------------------------------------------------------------------
//	UINT8 rawData[]	assumed to be 3 long.
// -----------------------------------------------------------------------------
BOOL SendData(UINT8 rawData[]){
    unsigned int evfMouse = 0;
    static BOOL bLastB0Down = FALSE;
    static BOOL bLastB1Down = FALSE;
	DWORD nx, ny;
    BOOL    bButton0Down;
    BOOL    bButton1Down;
    INT16   xOffset;
    INT16   yOffset;
	INT16	t;

    bButton0Down = ((rawData[0] & gcB0BitMask) != 0);
    bButton1Down = ((rawData[0] & gcB1BitMask) != 0);

	if (SwapButtons) swap_bool(&bButton0Down, &bButton1Down);

    xOffset = (INT8)(((rawData[0] & 0x03) << 6) | rawData[1]);
    yOffset = (INT8)(((rawData[0] & 0x0C) << 4) | rawData[2]);

    if (xOffset || yOffset) evfMouse |= MOUSEEVENTF_MOVE;
	if (DoMirrorX) xOffset = -xOffset;
	if (DoMirrorY) yOffset = -yOffset;

	if (DoRotate){
		t = yOffset;
		yOffset = xOffset;
		xOffset = t;
	}

    if (bLastB0Down != bButton0Down || bButton0Down) {
        if (bButton0Down) {
            evfMouse |= MOUSEEVENTF_LEFTDOWN;
        } else {
            evfMouse |= MOUSEEVENTF_LEFTUP;
        }

        bLastB0Down = bButton0Down;
    }

    if (bLastB1Down != bButton1Down || bButton1Down) {
        if (bButton1Down) {
            evfMouse |= MOUSEEVENTF_RIGHTDOWN;
        } else {
            evfMouse |= MOUSEEVENTF_RIGHTUP;
        }

        bLastB1Down = bButton1Down;
	}

	CORRECT_INCR(mousep.x, xOffset, SCREENW);
	CORRECT_INCR(mousep.y, yOffset, SCREENH);
	mousep.x += xOffset;
	mousep.y += yOffset;

	nx = mousep.x*(65535/SCREENW);
	ny = mousep.y*(65535/SCREENH);
	mouse_event(MOUSEEVENTF_ABSOLUTE|evfMouse,nx,ny, 0,0);
	//DrawCursor(xOffset,yOffset);
    return (TRUE);
}




/* TRAY INTERFACING *************************************************************/
/********************************************************************************/
/** TrayMessage: Sends a Message to System Tray.                               **/
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

/* REGISTRY INTERFACING *********************************************************/
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
/** LoadSettings: Load Settings from Registry.                                 **/
/********************************************************************************/
void LoadSettings(){
	DWORD dw;
	GetRegString(COM_PORT, 16,HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("ComPort"));
	GetRegDword(&dw,HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("SwapButtons"));
	SwapButtons = (dw != 0);
	GetRegDword(&dw,HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("MirrorX"));
	DoMirrorX = (dw != 0);
	GetRegDword(&dw,HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("MirrorY"));
	DoMirrorY = (dw != 0);
	GetRegDword(&dw,HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("Rotate"));
	DoRotate = (dw != 0);
	if (COM_PORT[0]==0) wcscpy(COM_PORT, _T("COM1:"));
}

/********************************************************************************/
/** SaveSettings: Save Settings into registry.                                 **/
/********************************************************************************/
void SaveSettings(){
	DWORD swap = (SwapButtons? 1 : 0);
	DWORD mirx = (DoMirrorX? 1: 0);
	DWORD miry = (DoMirrorY? 1: 0);
	DWORD rot =  (DoRotate? 1: 0);
	SetRegString(HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("ComPort"),COM_PORT);
	SetRegDword (HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("SwapButtons"),swap);
	SetRegDword (HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("MirrorX"),mirx);
	SetRegDword (HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("MirrorY"),miry);
	SetRegDword (HKEY_CURRENT_USER,_T("\\Software\\MouSer"),_T("Rotate"),rot);
}

/* GUI INTERFACING **************************************************************/
/********************************************************************************/
/** CheckBoxIfBool: Sets a checkbox according a boolean value.                 **/
/********************************************************************************/
void CheckBoxIfBool(HWND hdlg, int idc, BOOL b){
	SendDlgItemMessage(hdlg,idc,BM_SETCHECK,(b? BST_CHECKED : BST_UNCHECKED),0);
}
/********************************************************************************/
/** GetCheckBoxBool: Returns TRUE if a CheckBox is checked.                    **/
/********************************************************************************/
BOOL GetCheckBoxBool(HWND hdlg, int idc){
	return (SendDlgItemMessage(hdlg,idc,BM_GETCHECK,0,0) == BST_CHECKED);
}


/********************************************************************************/
/** GetGUISettings: Get Settings from GUI.                                     **/
/********************************************************************************/
void GetGUISettings(){
	SwapButtons = GetCheckBoxBool(ThisDlg, IDC_SWAPCHECK);
	DoMirrorX = GetCheckBoxBool(ThisDlg, IDC_MIRRORXCHECK);
	DoMirrorY = GetCheckBoxBool(ThisDlg, IDC_MIRRORYCHECK);
	DoRotate = GetCheckBoxBool(ThisDlg, IDC_ROTATECHECK);
	GetWindowText(ComPortBox, COM_PORT, 16);
}

/********************************************************************************/
/** SetGUISettings: Set elements in GUI to match current Settings.             **/
/********************************************************************************/
void SetGUISettings(){
	CheckBoxIfBool(ThisDlg, IDC_SWAPCHECK, SwapButtons);
	CheckBoxIfBool(ThisDlg, IDC_MIRRORXCHECK, DoMirrorX);
	CheckBoxIfBool(ThisDlg, IDC_MIRRORYCHECK, DoMirrorY);
	CheckBoxIfBool(ThisDlg, IDC_ROTATECHECK, DoRotate);
	SetWindowText(ComPortBox, COM_PORT);
}


/* CALLBACKS ********************************************************************/
/********************************************************************************/
/** MainDlgProc: Main Dialog Procedure.                                        **/
/********************************************************************************/
BOOL WINAPI MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	int wmId;
	BOOL IniWentRight;

	switch(msg){
		case WM_INITDIALOG:
			ThisDlg = hDlg;
			TrayIconAdd(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON)), NULL);
			LoadSettings();
			IniWentRight = Driver_Init();
			
			if (!IniWentRight) {
				SendMessage(hDlg, WM_CLOSE, 0,0);
			} else {
				ComPortBox = GetDlgItem(ThisDlg, IDC_COMPORT);
				SendDlgItemMessage(ThisDlg, IDC_COMPORT, EM_SETLIMITTEXT,16,0);
				IniHide = TRUE;
				SetForegroundWindow(ThisDlg);
				SetGUISettings();
				return TRUE;
			} 
			break;

		case WM_CLOSE:
			TrayIconDelete(hDlg, ID_TRAY, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAYICON)), NULL);
			EndDialog(hDlg, TRUE);
			if (DriverLoaded){
				GetGUISettings();
				SaveSettings();
				Driver_DeInit();
			}
			return TRUE;

		case WM_PAINT:
			if (IniHide) {
				HideDialog();
				IniHide = FALSE;
				return TRUE;
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
			}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			switch (wmId){
				case IDOK:
					GetGUISettings();
					SaveSettings();
					HideDialog();
					return TRUE;
				case IDCANCEL:
					SendMessage(hDlg,WM_CLOSE,0,0);
				break;
				case IDC_SWAPCHECK:
				case IDC_MIRRORXCHECK:
				case IDC_MIRRORYCHECK:
				case IDC_ROTATECHECK:
					GetGUISettings();
					SaveSettings();
				default:
					DefWindowProc(hDlg, msg, wParam, lParam);
			}
	}
	return FALSE;
}

/* ENTRY POINT ******************************************************************/
/********************************************************************************/
/** WinMain: The Default Entry Point                                           **/
/********************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR lpCmd, int nShow){
	int retcode;

	hInst = hInstance;
	SCREENW = GetSystemMetrics(SM_CXSCREEN);
	SCREENH = GetSystemMetrics(SM_CYSCREEN);

	InitCommonControls();
	retcode = DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDLG), NULL, (DLGPROC)MainDlgProc);
	return FALSE;
}
