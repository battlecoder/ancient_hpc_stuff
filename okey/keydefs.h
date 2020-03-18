WCHAR KEY_NAME[256][15] = {
_T("-INVALID-"), _T("LBUTTON"),	_T("RBUTTON"), 	_T("BREAK"), 
_T("MBUTTON"), 	_T("0x05"), 	_T("0x06"), 	_T("0x07"), 
_T("BACKSPC"), 	_T("TAB"), 		_T("0x0A"), 	_T("0x0B"), 
_T("CLEAR"), 	_T("ENTER"), 	_T("0x0E"), 	_T("0x0F"), 
_T("SHIFT"), 	_T("CTRL"), 	_T("ALT"), 		_T("PAUSE"), 
_T("CAPSLOCK"), _T("KANJI_0"), 	_T("KANJI_1"), 	_T("KANJI_2"), 
_T("KANJI_3"), 	_T("KANJI_4"), 	_T("0x1A"), 	_T("ESC"), 
_T("KANJI_5"), 	_T("KANJI_6"), 	_T("KANJI_7"), 	_T("KANJI_8"), 
_T("SPACE"), 	_T("PGUP"), 	_T("PGDN"), 	_T("END"), 
_T("HOME"), 	_T("LEFT"), 	_T("UP"), 		_T("RIGHT"), 
_T("DOWN"), 	_T("SELECT"), 	_T("OEM_0"), 	_T("EXECUTE"), 
_T("PRNTSCRN"), _T("0x2D"), 	_T("0x2E"), 	_T("HELP"), 
_T("0"), 		_T("1"), 		_T("2"), 		_T("3"), 
_T("4"), 		_T("5"), 		_T("6"), 		_T("7"), 
_T("8"), 		_T("9"), 		_T("0x3A"), 	_T("0x3B"), 
_T("0x3C"), 	_T("0x3D"), 	_T("0x3E"), 	_T("0x3F"), 
_T("0x40"), 	_T("A"), 		_T("B"), 		_T("C"), 
_T("D"), 		_T("E"), 		_T("F"), 		_T("G"), 
_T("H"), 		_T("I"), 		_T("J"), 		_T("K"), 
_T("L"), 		_T("M"), 		_T("N"), 		_T("O"), 
_T("P"), 		_T("Q"), 		_T("R"), 		_T("S"), 
_T("T"), 		_T("U"), 		_T("V"), 		_T("W"), 
_T("X"), 		_T("Y"), 		_T("Z"), 		_T("LWIN"), 
_T("RWIN"), 	_T("APPS"), 	_T("0x5E"), 	_T("0x5F"), 
_T("NUMPAD0"), 	_T("NUMPAD1"), 	_T("NUMPAD2"), 	_T("NUMPAD3"), 
_T("NUMPAD4"), 	_T("NUMPAD5"), 	_T("NUMPAD6"), 	_T("NUMPAD7"), 
_T("NUMPAD8"), 	_T("NUMPAD9"), 	_T("KEY*"), 	_T("KEY+"), 
_T("SEP."), 	_T("KEY-"), 	_T("KEY."), 	_T("KEY/"), 
_T("F1"),		_T("F2"),		_T("F3"),		_T("F4"), 
_T("F5"),		_T("F6"), 		_T("F7"), 		_T("F8"), 
_T("F9"),	 	_T("F10"),	 	_T("F11"),		_T("F12"), 
_T("F13"),		_T("F14"),		_T("F15"),		_T("F16"), 
_T("F17"),		_T("F18"),		_T("F19"),		_T("F20"), 
_T("F21"),		_T("F22"),		_T("F23"),		_T("F24"), 
_T("0x88"), 	_T("0x89"), 	_T("0x8A"), 	_T("0x8B"), 
_T("0x8C"), 	_T("0x8D"), 	_T("0x8E"), 	_T("0x8F"), 
_T("NUMLOCK"), 	_T("SCRLLOCK"), _T("0x92"), 	_T("0x93"), 
_T("0x94"), 	_T("0x95"), 	_T("0x96"), 	_T("0x97"), 
_T("0x98"), 	_T("0x99"), 	_T("0x9A"), 	_T("0x9B"), 
_T("0x9C"), 	_T("0x9D"), 	_T("0x9E"), 	_T("0x9F"), 
_T("LSHIFT"), 	_T("RSHIFT"), 	_T("LCTRL"), 	_T("RCTRL"), 
_T("LALT"), 	_T("RALT"), 	_T("0xA6"), 	_T("0xA7"), 
_T("0xA8"), 	_T("0xA9"), 	_T("0xAA"), 	_T("0xAB"), 
_T("0xAC"), 	_T("0xAD"), 	_T("0xAE"), 	_T("0xAF"), 
_T("0xB0"), 	_T("0xB1"), 	_T("0xB2"), 	_T("0xB3"), 
_T("0xB4"), 	_T("0xB5"), 	_T("0xB6"), 	_T("0xB7"), 
_T("0xB8"), 	_T("0xB9"), 	_T("OEM_1 ;:"),	_T("+"), 
_T(","),		_T("-"),		_T("."),		_T("OEM_2 /?"), 
_T("OEM_3 `~"),	_T("HWKEY0"), 	_T("HWKEY1"), 	_T("HWKEY2"), 
_T("HWKEY3"), 	_T("HWKEY4"), 	_T("HWKEY5"), 	_T("HWKEY6"), 
_T("HWKEY7"), 	_T("HWKEY8"), 	_T("HWKEY9"), 	_T("HWKEYA"), 
_T("HWKEYB"), 	_T("HWKEYC"), 	_T("HWKEYD"), 	_T("HWKEYE"), 
_T("0xD0"), 	_T("0xD1"), 	_T("0xD2"), 	_T("0xD3"), 
_T("0xD4"), 	_T("0xD5"), 	_T("0xD6"), 	_T("0xD7"), 
_T("0xD8"), 	_T("0xD9"), 	_T("0xDA"), 	_T("OEM_4 [{"), 
_T("OEM_5 \\|"),_T("OEM_6 ]}"),	_T("OEM_7 '\""),_T("OEM_8"), 
_T("OEM_9"), 	_T("OEM_A"), 	_T("OEM_B"), 	_T("OEM_C"), 
_T("OEM_D"), 	_T("0xE5"), 	_T("OEM_E"), 	_T("0xE7"), 
_T("0xE8"), 	_T("OEM_F"),	_T("OEM_10"), 	_T("OEM_11"), 
_T("OEM_12"), 	_T("OEM_13"), 	_T("OEM_14"), 	_T("OEM_15"), 
_T("OEM_16"), 	_T("OEM_17"), 	_T("OEM_18"), 	_T("OEM_19"), 
_T("OEM_1A"), 	_T("OEM_1B"), 	_T("ATTN"), 	_T("CRSEL"), 
_T("EXSEL"), 	_T("EREOF"), 	_T("PLAY"), 	_T("ZOOM"), 
_T("NONAME"), 	_T("PA1"), 		_T("EMCLEAR"), 	_T("-INVALID-")
};