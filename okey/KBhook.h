#ifndef _WINCE_KB_HOOK_H
#define _WINCE_KB_HOOK_H

//used for passing to SetWindowsHookEx funtion to set a Low level (LL) keyboard hook
#define WH_KEYBOARD_LL   		20

// Define the function types used by hooks
typedef LRESULT	(CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef HHOOK 		(WINAPI *_SetWindowsHookExW)(int, HOOKPROC, HINSTANCE, DWORD);
typedef LRESULT	(WINAPI *_CallNextHookEx)(HHOOK, int, WPARAM, LPARAM);
typedef LRESULT	(WINAPI *_UnhookWindowsHookEx)(HHOOK);


// For the low level keyboard hook, your keyboards procedures is passed a pointer to KBDLLHOOKSTRUCT instance
typedef struct {
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
    DWORD time;
    unsigned long *dwExtraInfo;
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;


// Win32 Hook APIs 
static _SetWindowsHookExW 		SetWindowsHookEx;
static _UnhookWindowsHookEx	UnhookWindowsHookEx;
static _CallNextHookEx  		CallNextHookEx;	

/** 
*	Function Name:ActivateKBHook
*	
*	Function Desc:Initializes the proc. adress of various hook related APIs.
*				  Loads the keyboard hook.
*
*	Parameters:
*				 HINSTANCE hInstance : handle to the application to be hooked
*				 HOOKPROC LLKeyboardHookCallbackFunction : procedure where the control will come to after any KB event.
*	Returns:	
*				 true if we get all the proc addresses of hook related APIs and load the hook succesfully
*				 false if any of the above fails
*
*	Author:		 Prathamesh Kulkarni
**/
BOOL ActivateKBHook(HINSTANCE hInstance, HOOKPROC LLKeyboardHookCallbackFunction);

/** 
*	Function Name:DeactivateKBHook
*	
*	Function Desc:Uninstall the KB hook
*
*	Parameters:
*				 none
*	Returns:	
*				 true if we exit gracefully
*
*	Author:		 Prathamesh Kulkarni
**/
BOOL DeactivateKBHook();



#endif

