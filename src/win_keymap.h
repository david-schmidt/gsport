/* this table is used to search for the Windows VK_* in col 1 or 2 */
/* flags bit 8 is or'ed into the VK, so we can distinguish keypad keys */
/* regardless of numlock */
int g_a2_key_to_wsym[][3] = {
	{ 0x35,	VK_ESCAPE,	0 },
	{ 0x7a,	VK_F1,	0 },
	{ 0x78,	VK_F2,	0 },	// OG Was 7B but F2 is defined has 0x78 in a2_key_to_ascii
	{ 0x63,	VK_F3,	0 },
	{ 0x76,	VK_F4,	0 },
	{ 0x60,	VK_F5,	0 },
	{ 0x61,	VK_F6,	0 },
	{ 0x62,	VK_F7,	0 },
	{ 0x64,	VK_F8,	0 },
	{ 0x65,	VK_F9,	0 },
	{ 0x6d,	VK_F10,	0 },
	{ 0x67,	VK_F11,	0 },
	{ 0x6f,	VK_F12,	0 },
	{ 0x69,	VK_F13,	0 },
	{ 0x6b,	VK_F14,	0 },
	{ 0x71,	VK_F15,	0 },
	{ 0x7f, VK_PAUSE, VK_CANCEL+0x100 },

	{ 0x32,	0xc0, 0 },		/* '`' */
	{ 0x12,	'1', 0 },
	{ 0x13,	'2', 0 },
	{ 0x14,	'3', 0 },
	{ 0x15,	'4', 0 },
	{ 0x17,	'5', 0 },
	{ 0x16,	'6', 0 },
	{ 0x1a,	'7', 0 },
	{ 0x1c,	'8', 0 },
	{ 0x19,	'9', 0 },
	{ 0x1d,	'0', 0 },
	{ 0x1b,	0xbd, 0 },		/* '-' */
	{ 0x18,	0xbb, 0 },		/* '=' */
	{ 0x33,	VK_BACK, 0 },		/* backspace */
	{ 0x72,	VK_INSERT+0x100, 0 },	/* Insert key */
/*	{ 0x73,	XK_Home, 0 },		alias VK_HOME to be KP_Equal! */
	{ 0x74,	VK_PRIOR+0x100, 0 },	/* pageup */
	{ 0x47,	VK_NUMLOCK, VK_NUMLOCK+0x100 },	/* clear */
	{ 0x51,	VK_HOME+0x100, 0 },		/* KP_equal is HOME key */
	{ 0x4b,	VK_DIVIDE, VK_DIVIDE+0x100 },
	{ 0x43,	VK_MULTIPLY, VK_MULTIPLY+0x100 },

	{ 0x30,	VK_TAB, 0 },
	{ 0x0c,	'Q', 0 },
	{ 0x0d,	'W', 0 },
	{ 0x0e,	'E', 0 },
	{ 0x0f,	'R', 0 },
	{ 0x11,	'T', 0 },
	{ 0x10,	'Y', 0 },
	{ 0x20,	'U', 0 },
	{ 0x22,	'I', 0 },
	{ 0x1f,	'O', 0 },
	{ 0x23,	'P', 0 },
	{ 0x21,	0xdb, 0 },	/* [ */
	{ 0x1e,	0xdd, 0 },	/* ] */
	{ 0x2a,	0xdc, 0 },	/* backslash, bar */
	{ 0x75,	VK_DELETE+0x100, 0 },
	{ 0x77,	VK_END+0x100, VK_END },
	{ 0x79,	VK_NEXT+0x100, 0 },
	{ 0x59,	VK_NUMPAD7, VK_HOME },
	{ 0x5b,	VK_NUMPAD8, VK_UP },
	{ 0x5c,	VK_NUMPAD9, VK_PRIOR },
	{ 0x4e,	VK_SUBTRACT, VK_SUBTRACT+0x100 },

	// { 0x39,	VK_CAPITAL, 0 },  // Handled specially!
	{ 0x00,	'A', 0 },
	{ 0x01,	'S', 0 },
	{ 0x02,	'D', 0 },
	{ 0x03,	'F', 0 },
	{ 0x05,	'G', 0 },
	{ 0x04,	'H', 0 },
	{ 0x26,	'J', 0 },
	{ 0x28,	'K', 0 },
	{ 0x25,	'L', 0 },
	{ 0x29,	0xba, 0 },	/* ; */
	{ 0x27,	0xde, 0 },	/* single quote */
	{ 0x24,	VK_RETURN, 0 },
	{ 0x56,	VK_NUMPAD4, VK_LEFT },
	{ 0x57,	VK_NUMPAD5, VK_CLEAR },
	{ 0x58,	VK_NUMPAD6, VK_RIGHT },
	{ 0x45,	VK_ADD, 0 },

	{ 0x38,	VK_SHIFT, 0 },
	{ 0x06,	'Z', 0 },
	{ 0x07,	'X', 0 },
	{ 0x08,	'C', 0 },
	{ 0x09,	'V', 0 },
	{ 0x0b,	'B', 0 },
	{ 0x2d,	'N', 0 },
	{ 0x2e,	'M', 0 },
	{ 0x2b,	0xbc, 0 },	/* , */
	{ 0x2f,	0xbe, 0 },	/* . */
	{ 0x2c,	0xbf, 0 },	/* / */
	{ 0x3e,	VK_UP+0x100, 0 },
	{ 0x53,	VK_NUMPAD1, VK_END },
	{ 0x54,	VK_NUMPAD2, VK_DOWN },
	{ 0x55,	VK_NUMPAD3, VK_NEXT },

	{ 0x36,	VK_CONTROL, VK_CONTROL+0x100 },
	{ 0x3a,	VK_SNAPSHOT+0x100, VK_MENU+0x100 },/* Opt=prntscrn or alt-r */

// OG ActiveGS map OA-CA to Win & AltKey
#ifndef ACTIVEGS
	{ 0x37,	VK_SCROLL, VK_MENU },		/* Command=scr_lock or alt-l */
#else
	{ 0x7f, VK_CANCEL, 0 },
	{ 0x3A, VK_LWIN+0x100, VK_LWIN },
	{ 0x37,	VK_MENU, 0 },		/* Command=alt-l */
	{ 0x37,	VK_LMENU, 0 },		/* Command=alt-l */
	{ 0x7F,	VK_SCROLL,0 },		/* RESET */
	{ 0x36,	VK_LCONTROL, 0 },	// CTRL
#endif

	{ 0x31,	' ', 0 },
	{ 0x3b,	VK_LEFT+0x100, 0 },
	{ 0x3d,	VK_DOWN+0x100, 0 },
	{ 0x3c,	VK_RIGHT+0x100, 0 },
	{ 0x52,	VK_NUMPAD0, VK_INSERT },
	{ 0x41,	VK_DECIMAL, VK_DECIMAL },
	{ 0x4c,	VK_RETURN+0x100, 0 },
	{ -1, -1, -1 }
};