/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 by GSport contributors
 
 Based on the KEGS emulator written by and Copyright (C) 2003 Kent Dickey

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or (at your 
 option) any later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

const char rcsid_macdriver_c[] = "@(#)$KmKId: macdriver.c,v 1.24 2004-11-14 10:23:29-05 kentd Exp $";

// Quartz: CreateCGContextForPort vs QDBeginCGContext

// Use CGDisplayMoveCursorToPoint(kCGDirectMainDisplay) to warp pointer
// Use CGPointMake to get a point

#include <Carbon/Carbon.h>
#include <Quicktime/Quicktime.h>

#include "defc.h"
#include "protos_macdriver.h"

#define MAX_STATUS_LINES	7
#define X_LINE_LENGTH		88
#define MAX_MAC_ARGS		128

WindowRef	g_main_window;
WindowRef	g_main_window_saved;
Rect		g_main_window_saved_rect;
Ptr		g_mac_fullscreen_state = 0;
int		g_quit_seen = 0;
EventHandlerUPP g_quit_handler_UPP;
EventHandlerUPP g_dummy_event_handler_UPP;
RgnHandle	g_event_rgnhandle = 0;
int		g_ignore_next_click = 0;
int		g_mainwin_active = 0;
GDHandle	g_gdhandle = 0;

int	g_mac_mouse_x = 0;
int	g_mac_mouse_y = 0;

FMFontFamily	g_status_font_family;

extern Kimage g_mainwin_kimage;

int	g_mac_argc = 0;
char	*g_mac_argv[MAX_MAC_ARGS];
word32	g_mac_shift_control_state = 0;
extern char g_argv0_path[];
extern char *g_fatal_log_strs[];
extern int g_fatal_log;

extern word32 g_red_mask;
extern word32 g_green_mask;
extern word32 g_blue_mask;
extern int g_red_left_shift;
extern int g_green_left_shift;
extern int g_blue_left_shift;
extern int g_red_right_shift;
extern int g_green_right_shift;
extern int g_blue_right_shift;

int	g_use_shmem = 0;

extern int Verbose;

extern int g_warp_pointer;
extern int g_screen_depth;
extern int g_force_depth;
int g_screen_mdepth = 0;

extern int g_send_sound_to_file;

extern int g_quit_sim_now;
extern int g_config_control_panel;

extern int g_video_act_width;
extern int g_video_act_height;
extern int g_video_act_margin_left;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top;
extern int g_video_act_margin_bottom;

int	g_auto_repeat_on = -1;
int	g_x_shift_control_state = 0;


extern int Max_color_size;

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

int	g_alt_left_up = 1;
int	g_alt_right_up = 1;

extern word32 g_full_refresh_needed;

extern int g_border_sides_refresh_needed;
extern int g_border_special_refresh_needed;
extern int g_status_refresh_needed;

extern int g_lores_colors[];

extern int g_a2vid_palette;

extern int g_installed_full_superhires_colormap;

extern int g_screen_redraw_skip_amt;

extern word32 g_a2_screen_buffer_changed;
extern char *g_status_ptrs[MAX_STATUS_LINES];
extern const char g_kegs_version_str[];

#if 0
extern int g_debug_file_fd;

/* HACK to debug startup issues when launched from Finder */
int
printf(const char *fmt, ...)
{
	va_list	ap;
	int	ret;

	va_start(ap, fmt);

	if(g_debug_file_fd < 0) {
		g_debug_file_fd = open("/tmp/kegs.out",
				O_CREAT | O_WRONLY | O_TRUNC, 0x1b6);
		fprintf(stdout, "g_debug_file_fd = %d, %d\n", g_debug_file_fd,
				errno);
	}
	ret = kegs_vprintf(fmt, ap);

	va_end(ap);

	return ret;
}
#endif

pascal OSStatus
quit_event_handler(EventHandlerCallRef call_ref, EventRef event, void *ignore)
{
	OSStatus	err;

	err = CallNextEventHandler(call_ref, event);
	if(err == noErr) {
		g_quit_seen = 1;
	}
	return err;
}

void
show_simple_alert(char *str1, char *str2, char *str3, int num)
{
	char		buf[256];

	g_fatal_log_strs[0] = kegs_malloc_str(str1);
	g_fatal_log_strs[1] = kegs_malloc_str(str2);
	g_fatal_log_strs[2] = kegs_malloc_str(str3);
	g_fatal_log = 3;
	if(num != 0) {
		snprintf(buf, 250, ": %d", num);
		g_fatal_log_strs[g_fatal_log++] = kegs_malloc_str(buf);
	}
	x_show_alert(0, 0);
}

void
x_dialog_create_kegs_conf(const char *str)
{
	char	*path;
	char	tmp_buf[512];
	int	ret;

	ret = x_show_alert(1, str);
	if(ret) {
		// Create empty file
		path = "~/Library/KEGS";
		snprintf(tmp_buf, 500, "mkdir -p %s", path);
		system(tmp_buf);
		snprintf(tmp_buf, 500, "touch %s/%s", path, str);
		system(tmp_buf);
	}
}

int
x_show_alert(int is_fatal, const char *str)
{
	DialogRef	alert;
	DialogItemIndex	out_item_hit;
	CFStringRef	cfstrref, cfstrref2;
	CFStringRef	okstrref;
	AlertStdCFStringAlertParamRec alert_param;
	OSStatus osstat;
	char	*bufptr, *buf2ptr;
	int	sum, len;
	int	i;

	/* The dialog eats all events--including key-up events */
	/* Call adb_all_keys_up() to prevent annoying key-repeat problems */
	/*  for instance, a key-down causes a dialog to appear--and the */
	/*  eats the key-up event...then as soon as the dialog goes, adb.c */
	/*  auto-repeat will repeat the key, and the dialog re-appears...*/
	adb_all_keys_up();

	sum = 20;
	for(i = 0; i < g_fatal_log; i++) {
		sum += strlen(g_fatal_log_strs[i]);
	}
	bufptr = malloc(sum);
	buf2ptr = bufptr;
	for(i = 0; i < g_fatal_log; i++) {
		len = strlen(g_fatal_log_strs[i]);
		len = MIN(len, sum);
		len = MAX(len, 0);
		memcpy(bufptr, g_fatal_log_strs[i], MIN(len, sum));
		bufptr += len;
		bufptr[0] = 0;
		sum = sum - len;
	}

	cfstrref = CFStringCreateWithCString(NULL, buf2ptr,
						kCFStringEncodingMacRoman);

	printf("buf2ptr: :%s:\n", buf2ptr);

	osstat = GetStandardAlertDefaultParams(&alert_param,
						kStdCFStringAlertVersionOne);

	if(str) {
		// Provide an extra option--create a file
		cfstrref2 = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				CFSTR("Create ~/Library/KEGS/%s"), str);
		alert_param.otherText = cfstrref2;
	}
	okstrref = CFSTR("Click OK to continue");
	if(is_fatal) {
		okstrref = CFSTR("Clock OK to exit KEGS");
	}
	CreateStandardAlert(kAlertStopAlert, cfstrref, okstrref,
			&alert_param, &alert);
	out_item_hit = -1;
	RunStandardAlert(alert, NULL, &out_item_hit);
	printf("out_item_hit: %d\n", out_item_hit);
	free(buf2ptr);

	clear_fatal_logs();		/* free the fatal_log string memory */
	return (out_item_hit >= 3);
}


pascal OSStatus
my_cmd_handler( EventHandlerCallRef handlerRef, EventRef event, void *userdata)
{
	OSStatus	osresult;
	HICommand	command;
	word32		command_id;

	osresult = eventNotHandledErr;

	GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL,
		sizeof(HICommand), NULL, &command);

	command_id = (word32)command.commandID;
	switch(command_id) {
	case 'Kbep':
		SysBeep(10);
		osresult = noErr;
		break;
	case 'abou':
		show_simple_alert("GSportMac v", (char *)g_kegs_version_str,
			", Copyright 2004 Kent Dickey\n"
			"Latest version at http://kegs.sourceforge.net/\n", 0);
		osresult = noErr;
		break;
	case 'KCFG':
		cfg_toggle_config_panel();
		osresult = noErr;
		break;
	case 'quit':
		break;
	case 'swin':
		/* not sure what this is, but Panther sends it */
		break;
	default:
		printf("commandID %08x unknown\n", command_id);
		SysBeep(90);
		break;
	}
	return osresult;
}

int g_upd_count = 0;

void
update_window(void)
{
	SetPortWindowPort(g_main_window);

	PenNormal();

	g_full_refresh_needed = -1;
	g_a2_screen_buffer_changed = -1;
	g_status_refresh_needed = 1;
	g_border_sides_refresh_needed = 1;
	g_border_special_refresh_needed = 1;

	g_upd_count++;
	if(g_upd_count > 250) {
		g_upd_count = 0;
	}

}

struct char_int_un {
	union {
		char c[4];
		UInt32	uint;
	} u;
	char c2[2];
};

typedef struct char_int_un Char_int;

int g_event_count = 0;

void
show_event(UInt32 event_class, UInt32 event_kind, int handled)
{
	Char_int	char_int;

	if(handled == 0 && event_class != 'cgs ') {
		char_int.c2[0] = 0;
		char_int.u.uint = event_class;
		printf("Event %d:  %08x = %s, %d\n", g_event_count,
			(int)event_class, &(char_int.u.c[0]), (int)event_kind);
	}
	g_event_count++;
}

pascal OSStatus
my_win_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata)
{
	OSStatus os_result;
	UInt32	event_kind;

	os_result = eventNotHandledErr;

	// SysBeep(1);

	event_kind = GetEventKind(event);
	// show_alert("win handler", event_kind);
	if(event_kind == kEventWindowDrawContent) {
		update_window();
	} if(event_kind == kEventWindowClose) {
		g_quit_sim_now = 1;
		g_quit_seen = 1;
		my_exit(0);
	} else {
		show_event(GetEventClass(event), event_kind, 0);
		update_window();
	}

	return os_result;
}


pascal OSStatus
dummy_event_handler(EventHandlerCallRef call_ref, EventRef in_event,
								void *ignore)
{
	OSStatus	err;
	EventHandlerRef	installed_handler;
	EventTypeSpec	event_spec = { kEventClassApplication, kEventAppQuit };

	// From http://developer.apple.com/qa/qa2001/qa1061.html
	// Trick to move main event queue to use ReceiveNextEvent in an event
	//  handler called by RunApplicationEventLoop

	err = InstallApplicationEventHandler(g_quit_handler_UPP, 1, &event_spec,
		NULL, &installed_handler);

	kegsmain(g_mac_argc, g_mac_argv);

	return noErr;
}

void
mac_update_modifiers(word32 state)
{
	word32	state_xor;
	int	is_up;

	state = state & (cmdKey | shiftKey | alphaLock | optionKey |
				controlKey);
	state_xor = g_mac_shift_control_state ^ state;
	is_up = 0;
	if(state_xor & controlKey) {
		is_up = ((state & controlKey) == 0);
		adb_physical_key_update(0x36, is_up);
	}
	if(state_xor & alphaLock) {
		is_up = ((state & alphaLock) == 0);
		adb_physical_key_update(0x39, is_up);
	}
	if(state_xor & shiftKey) {
		is_up = ((state & shiftKey) == 0);
		adb_physical_key_update(0x38, is_up);
	}
	if(state_xor & cmdKey) {
		is_up = ((state & cmdKey) == 0);
		adb_physical_key_update(0x37, is_up);
	}
	if(state_xor & optionKey) {
		is_up = ((state & optionKey) == 0);
		adb_physical_key_update(0x3a, is_up);
	}

	g_mac_shift_control_state = state;
}

void
mac_warp_mouse()
{
	Rect		port_rect;
	Point		win_origin_pt;
	CGPoint		cgpoint;
	CGDisplayErr	cg_err;

	GetPortBounds(GetWindowPort(g_main_window), &port_rect);
	SetPt(&win_origin_pt, port_rect.left, port_rect.top);
	LocalToGlobal(&win_origin_pt);

	cgpoint = CGPointMake( (float)(win_origin_pt.h + X_A2_WINDOW_WIDTH/2),
			(float)(win_origin_pt.v + X_A2_WINDOW_HEIGHT/2));
	cg_err = CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, cgpoint);
}

void
check_input_events()
{
	OSStatus	err;
	EventTargetRef	target;
	EventRef	event;
	UInt32		event_class, event_kind;
	byte		mac_keycode;
	UInt32		keycode;
	UInt32		modifiers;
	Point		mouse_point, mouse_delta_point;
	WindowRef	window_ref;
	int	button, button_state;
	EventMouseButton	mouse_button;
	int		handled;
	int		mouse_events;
	int		is_up;
	int		in_win;
	int		ignore;

	if(g_quit_seen) {
		exit(0);
	}

	SetPortWindowPort(g_main_window);

	mouse_events = 0;
	target = GetEventDispatcherTarget();
	while(1) {
		err = ReceiveNextEvent(0, NULL, kEventDurationNoWait,
			true, &event);

		if(err == eventLoopTimedOutErr) {
			break;
		}
		if(err != noErr) {
			printf("ReceiveNextEvent err: %d\n", (int)err);
			break;
		}

		event_class = GetEventClass(event);
		event_kind = GetEventKind(event);
		handled = 0;
		switch(event_class) {
		case kEventClassKeyboard:
			handled = 1;
			keycode = 0;
			modifiers = 0;
			GetEventParameter(event, kEventParamKeyMacCharCodes,
				typeChar, NULL, sizeof(byte), NULL,
				&mac_keycode);
			GetEventParameter(event, kEventParamKeyCode,
				typeUInt32, NULL, sizeof(UInt32), NULL,
				&keycode);
			GetEventParameter(event, kEventParamKeyModifiers,
				typeUInt32, NULL, sizeof(UInt32), NULL,
				&modifiers);

			mac_update_modifiers((word32)modifiers);

			// Key up/down event
			is_up = -1;
			switch(event_kind) {
			case kEventRawKeyDown:
				is_up = 0;
				//printf("key down: %02x, %08x\n",
				//	(int)mac_keycode, (int)keycode);
				break;
			case kEventRawKeyUp:
				is_up = 1;
				//printf("key up: %02x, %08x\n",
				//	(int)mac_keycode, (int)keycode);
				break;
			case kEventRawKeyModifiersChanged:
				is_up = -1;
				//printf("key xxx: %08x\n", (int)modifiers);
				break;
			}
			if(is_up >= 0) {
				adb_physical_key_update((int)keycode, is_up);
			}
			break;
		case kEventClassMouse:
			handled = 2;
			mouse_events++;
			GetEventParameter(event, kEventParamMouseLocation,
				typeQDPoint, NULL, sizeof(Point), NULL,
				&mouse_point);
			GetWindowRegion(g_main_window, kWindowContentRgn,
				g_event_rgnhandle);
			in_win =  PtInRgn(mouse_point, g_event_rgnhandle);
			// in_win = 1 if it was in the contect region of window
			err = GetEventParameter(event, kEventParamMouseDelta,
				typeQDPoint, NULL, sizeof(Point), NULL,
				&mouse_delta_point);
			button = 0;
			button_state = -1;
			switch(event_kind) {
			case kEventMouseDown:
				button_state = 7;
				handled = 3;
				break;
			case kEventMouseUp:
				button_state = 0;
				handled = 3;
				break;
			}
			if(button_state >= 0) {
				GetEventParameter(event, kEventParamMouseButton,
					typeMouseButton, NULL,
					sizeof(EventMouseButton), NULL,
					&mouse_button);
				button = mouse_button;
				if(button > 1) {
					button = 4 - button;
					button = 1 << button;
				}
				ignore = (button_state != 0) &&
					(!in_win || g_ignore_next_click);
				ignore = ignore || !g_mainwin_active;
				if(ignore) {
					// Outside of A2 window, ignore clicks
					button = 0;
				}
				if(button_state == 0) {
					g_ignore_next_click = 0;
				}
			}

			GlobalToLocal(&mouse_point);

			if(g_warp_pointer) {
				if(err == 0) {
					g_mac_mouse_x += mouse_delta_point.h;
					g_mac_mouse_y += mouse_delta_point.v;
				}
				mac_warp_mouse();
			} else {
				g_mac_mouse_x = mouse_point.h -
							g_video_act_margin_left;
				g_mac_mouse_y = mouse_point.v -
							g_video_act_margin_top;
			}

#if 0
			printf("Mouse %d at: %d,%d button:%d, button_st:%d\n",
				mouse_events, g_mac_mouse_x, g_mac_mouse_y,
				button, button_state);
			printf("Mouse deltas: err:%d, %d,%d\n", (int)err,
				mouse_delta_point.h, mouse_delta_point.v);
#endif

			update_mouse(g_mac_mouse_x, g_mac_mouse_y,
				button_state, button & 7);
			if(g_warp_pointer) {
				g_mac_mouse_x = A2_WINDOW_WIDTH/2;
				g_mac_mouse_y = A2_WINDOW_HEIGHT/2;
				update_mouse(g_mac_mouse_x, g_mac_mouse_y,0,-1);
			}
			break;
		case kEventClassApplication:
			switch(event_kind) {
			case kEventAppActivated:
				handled = 1;
				g_mainwin_active = 1;
				window_ref = 0;
				GetEventParameter(event, kEventParamWindowRef,
					typeWindowRef, NULL, sizeof(WindowRef),
					NULL, &window_ref);
				if(window_ref == g_main_window) {
					g_ignore_next_click = 1;
				}
				break;
			case kEventAppDeactivated:
				handled = 1;
				g_mainwin_active = 0;
				g_ignore_next_click = 1;
				break;
			}
			break;
		}
		show_event(event_class, event_kind, handled);
		if(handled != 1) {
			(void)SendEventToEventTarget(event, target);
		}
		ReleaseEvent(event);
	}

	return;
}

void
temp_run_application_event_loop(void)
{
	OSStatus	err;
	EventRef	dummy_event;
	EventHandlerRef	install_handler;
	EventTypeSpec	event_spec = { 'KWIN', 'KWIN' };

	// Create UPP for dummy_event_handler and for quit_event_handler
	err = noErr;
	dummy_event = 0;

	g_dummy_event_handler_UPP = NewEventHandlerUPP(dummy_event_handler);
	g_quit_handler_UPP = NewEventHandlerUPP(quit_event_handler);
	if((g_dummy_event_handler_UPP == 0) || (g_quit_handler_UPP == 0)) {
		err = memFullErr;
	}

	if(err == noErr) {
		err = InstallApplicationEventHandler(g_dummy_event_handler_UPP,
				1, &event_spec, 0, &install_handler);
		if(err == noErr) {
			err = MacCreateEvent(NULL, 'KWIN', 'KWIN',
				GetCurrentEventTime(), kEventAttributeNone,
				&dummy_event);
			if(err == noErr) {
				err = PostEventToQueue(GetMainEventQueue(),
					dummy_event, kEventPriorityHigh);
			}
			if(err == noErr) {
				RunApplicationEventLoop();
			}

			(void)RemoveEventHandler(install_handler);
		}
	}

	if(dummy_event != NULL) {
		ReleaseEvent(dummy_event);
	}
}

int
main(int argc, char* argv[])
{
	ProcessSerialNumber my_psn;
	IBNibRef	nibRef;
	EventHandlerUPP	handlerUPP;
	EventTypeSpec	cmd_event[3];
#if 0
	MenuBarHandle	mbar_handle;
	MenuRef		menu_ref;
#endif
	Rect		win_rect;
	OSStatus	err;
	char	*argptr;
	int	slash_cnt;
	int	i;

	/* Prepare argv0 */
	slash_cnt = 0;
	argptr = argv[0];
	for(i = strlen(argptr); i >= 0; i--) {
		if(argptr[i] == '/') {
			slash_cnt++;
			if(slash_cnt == 3) {
				strncpy(&(g_argv0_path[0]), argptr, i);
				g_argv0_path[i] = 0;
			}
		}
	}

	printf("g_argv0_path is %s\n", g_argv0_path);

	g_mac_argv[0] = argv[0];
	g_mac_argc = 1;
	i = 1;
	while((i < argc) && (g_mac_argc < MAX_MAC_ARGS)) {
		if(!strncmp(argv[i], "-psn", 4)) {
			/* skip this argument */
		} else {
			g_mac_argv[g_mac_argc++] = argv[i];
		}
		i++;
	}

	InitCursor();
	g_event_rgnhandle = NewRgn();
	g_status_font_family = FMGetFontFamilyFromName("\pCourier");
	SetRect(&win_rect, 0, 0, X_A2_WINDOW_WIDTH, X_A2_WINDOW_HEIGHT +
				MAX_STATUS_LINES*16 + 8);
	OffsetRect(&win_rect, 64, 50);

	// Create a Nib reference passing the name of the nib file
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr( err, CantGetNibRef );

	// Once the nib reference is created, set the menu bar.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr( err, CantSetMenuBar );


#if 0
	mbar_handle = GetMenuBar();
	SetMenuBar(mbar_handle);
	printf("mbar_handle: %p\n", mbar_handle);

	menu_ref = NewMenu(1, "\pTest");
	printf("menu_ref: %p\n", menu_ref);
	AppendMenu(menu_ref, "\pTest item 1");
	InsertMenu(menu_ref, 0);
//	ShowMenuBar();	// Don't call ShowMenuBar: it prevents menubar update!
	DrawMenuBar();
	InvalMenuBar();
#endif

	err = CreateNewWindow(kDocumentWindowClass,
		kWindowStandardDocumentAttributes |
					kWindowStandardHandlerAttribute,
		&win_rect, &g_main_window);

	//printf("CreateNewWindow ret: %d, g_main_window: %p\n", (int)err,
	//		g_main_window);

	err = SetWindowTitleWithCFString(g_main_window, CFSTR("GSportMac"));

	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);

	SysBeep(120);

	handlerUPP = NewEventHandlerUPP( my_cmd_handler );

	cmd_event[0].eventClass = kEventClassCommand;
	cmd_event[0].eventKind = kEventProcessCommand;
	InstallWindowEventHandler(g_main_window, handlerUPP, 1, &cmd_event[0],
					(void *)g_main_window, NULL);

	handlerUPP = NewEventHandlerUPP(my_win_handler);
	cmd_event[0].eventClass = kEventClassWindow;
	cmd_event[0].eventKind = kEventWindowDrawContent;
	cmd_event[1].eventClass = kEventClassWindow;
	cmd_event[1].eventKind = kEventWindowUpdate;
	cmd_event[2].eventClass = kEventClassWindow;
	cmd_event[2].eventKind = kEventWindowClose;
	err = InstallWindowEventHandler(g_main_window, handlerUPP, 3,
				&cmd_event[0], (void *)g_main_window, NULL);
	require_noerr(err, CantCreateWindow);

	// Get screen depth
	g_gdhandle = GetGDevice();
	g_screen_mdepth = (**((**g_gdhandle).gdPMap)).pixelSize;

	g_screen_depth = g_screen_mdepth;

	//printf("g_screen_depth = %d, depth: %d, bytes: %d\n", g_screen_depth,
	//		(**g_gdhandle).gdCCDepth, (**g_gdhandle).gdCCBytes);

	if(g_screen_depth > 16) {
		/* 32-bit display */
		g_red_mask = 0xff;
		g_green_mask = 0xff;
		g_blue_mask = 0xff;
		g_red_left_shift = 16;
		g_green_left_shift = 8;
		g_blue_left_shift = 0;
		g_red_right_shift = 0;
		g_green_right_shift = 0;
		g_blue_right_shift = 0;
	} else if(g_screen_depth > 8) {
		/* 16-bit display */
		g_red_mask = 0x1f;
		g_green_mask = 0x1f;
		g_blue_mask = 0x1f;
		g_red_left_shift = 10;
		g_green_left_shift = 5;
		g_blue_left_shift = 0;
		g_red_right_shift = 3;
		g_green_right_shift = 3;
		g_blue_right_shift = 3;
	}

	// show_alert("About to show window", (int)g_main_window);
	update_main_window_size();

	update_window();

	// The window was created hidden so show it.
	ShowWindow( g_main_window );
	BringToFront( g_main_window );
	update_window();

	// Make us pop to the front a different way
	err = GetCurrentProcess(&my_psn);
	if(err == noErr) {
		(void)SetFrontProcess(&my_psn);
	}

	// Call the event loop
	temp_run_application_event_loop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	show_simple_alert("ending", "", "error code", err);
	return err;
}


void
x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
}


void
x_update_physical_colormap()
{
}

void
show_xcolor_array()
{
	int i;

	for(i = 0; i < 256; i++) {
		printf("%02x: %08x\n", i, g_palette_8to1624[i]);
	}
}

void
xdriver_end()
{

	printf("xdriver_end\n");
	if(g_fatal_log >= 0) {
		x_show_alert(1, 0);
	}
}

void
x_get_kimage(Kimage *kimage_ptr)
{
	PixMapHandle	pixmap_handle;
	GWorldPtr	world;
	Rect		world_rect;
	OSStatus	err;
	word32	*wptr;
	byte	*ptr;
	int	row_bytes;
	int	width;
	int	height;
	int	depth, mdepth;
	int	size;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	size = 0;
	if(depth == g_screen_depth) {
		SetRect(&world_rect, 0, 0, width, height);
		err = NewGWorld( &world, 0, &world_rect, NULL, NULL, 0);
		pixmap_handle = GetGWorldPixMap(world);
		err = LockPixels(pixmap_handle);
		ptr = (byte *)GetPixBaseAddr(pixmap_handle);
		row_bytes = ((*pixmap_handle)->rowBytes & 0x3fff);
		kimage_ptr->width_act = row_bytes / (mdepth >> 3);
		mac_printf("Got depth: %d, bitmap_ptr: %p, width: %d\n", depth,
				ptr, kimage_ptr->width_act);
		mac_printf("pixmap->base: %08x, rowbytes: %08x, pixType:%08x\n",
			(int)(*pixmap_handle)->baseAddr,
			(*pixmap_handle)->rowBytes,
			(*pixmap_handle)->pixelType);
		wptr = (word32 *)(*pixmap_handle);
		mac_printf("wptr: %p=%08x %08x %08x %08x %08x %08x %08x %08x\n",
			wptr,
			wptr[0], wptr[1], wptr[2], wptr[3],
			wptr[4], wptr[5], wptr[6], wptr[7]);
		kimage_ptr->dev_handle = pixmap_handle;
		kimage_ptr->data_ptr = ptr;
	} else {

		/* allocate buffers for video.c to draw into */

		size = (width*height*mdepth) >> 3;
		ptr = (byte *)malloc(size);

		if(ptr == 0) {
			mac_printf("malloc for data fail, mdepth:%d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;

		kimage_ptr->dev_handle = (void *)-1;
	}

	mac_printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,
		kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);

}

void
dev_video_init()
{
	int	lores_col;
	int	i;

	printf("Preparing graphics system\n");

	video_get_kimages();

	if(g_screen_depth != 8) {
		// Get g_mainwin_kimage
		video_get_kimage(&g_mainwin_kimage, 0, g_screen_depth,
							g_screen_mdepth);
	}

	for(i = 0; i < 256; i++) {
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	g_installed_full_superhires_colormap = 1;

	fflush(stdout);
}


void
x_redraw_status_lines()
{
	Rect	rect;
	Pattern	white_pattern;
	char	tmp_buf[256];
	char	*buf;
	int	len;
	int	line;
	int	height;
	int	margin;

	SetPortWindowPort(g_main_window);
	PenNormal();
	height = 16;
	margin = 0;
	TextFont(g_status_font_family);
	TextFace(normal);
	TextSize(12);

	SetRect(&rect, 0, X_A2_WINDOW_HEIGHT + margin, X_A2_WINDOW_WIDTH,
		X_A2_WINDOW_HEIGHT + margin + MAX_STATUS_LINES*height);
	GetQDGlobalsWhite(&white_pattern);
	FillRect(&rect, &white_pattern);

	for(line = 0; line < MAX_STATUS_LINES; line++) {
		buf = g_status_ptrs[line];
		if(buf == 0) {
			/* skip it */
			continue;
		}
		MoveTo(10, X_A2_WINDOW_HEIGHT + height*line + margin + height);
		len = MIN(250, strlen(buf));
		strncpy(&tmp_buf[1], buf, len);
		tmp_buf[0] = len;
		DrawString(&tmp_buf[0]);
	}

}


void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
		int width, int height)
{
	PixMapHandle pixmap_handle;
	Rect	src_rect, dest_rect;
	CGrafPtr window_port;

	SetPortWindowPort(g_main_window);

	pixmap_handle = kimage_ptr->dev_handle;
	window_port = GetWindowPort(g_main_window);
	SetRect(&src_rect, srcx, srcy, srcx + width, srcy + height);
	SetRect(&dest_rect, destx, desty, destx + width, desty + height);

	CopyBits( (BitMap *)(*pixmap_handle),
		GetPortBitMapForCopyBits(window_port), &src_rect, &dest_rect,
		srcCopy, NULL);

}

void
x_push_done()
{
	CGrafPtr window_port;

	SetPortWindowPort(g_main_window);
	window_port = GetWindowPort(g_main_window);

	QDFlushPortBuffer(window_port, 0);
}

void
x_auto_repeat_on(int must)
{
}

void
x_auto_repeat_off(int must)
{
}

void
x_hide_pointer(int do_hide)
{
	if(do_hide) {
		HideCursor();
	} else {
		ShowCursor();
	}
}

void
x_full_screen(int do_full)
{
	WindowRef new_window;
	short	width, height;
	OSErr	ret;

	width = 640;
	height = 480;
	if(do_full && (g_mac_fullscreen_state == 0)) {
		g_main_window_saved = g_main_window;
		
		GetWindowBounds(g_main_window, kWindowContentRgn,
						&g_main_window_saved_rect);
		ret = BeginFullScreen(&g_mac_fullscreen_state, 0,
			&width, &height, &new_window, 0, 0);
		printf("Ret beginfullscreen: %d\n", (int)ret);
		printf("New width: %d, new height: %d\n", width, height);
		if(ret == noErr) {
			g_main_window = new_window;
		} else {
			g_mac_fullscreen_state = 0;
		}
	} else if(!do_full && (g_mac_fullscreen_state != 0)) {
		ret = EndFullScreen(g_mac_fullscreen_state, 0);
		printf("ret endfullscreen: %d\n", (int)ret);
		g_main_window = g_main_window_saved;
		g_mac_fullscreen_state = 0;
		//InitCursor();
		SetWindowBounds(g_main_window, kWindowContentRgn,
						&g_main_window_saved_rect);
	}

	update_main_window_size();

	ShowWindow(g_main_window);
	BringToFront(g_main_window);
	update_window();
}

void
update_main_window_size()
{
	Rect	win_rect;
	int	width, height;
	int	left, excess_height;
	int	top, bottom;

	GetPortBounds(GetWindowPort(g_main_window), &win_rect);
	width = win_rect.right - win_rect.left;
	height = win_rect.bottom - win_rect.top;
	g_video_act_width = width;
	g_video_act_height = height;

	left = MAX(0, (width - A2_WINDOW_WIDTH) / 2);
	left = MIN(left, BASE_MARGIN_LEFT);
	g_video_act_margin_left = left;
	g_video_act_margin_right = left;


	excess_height = (height - A2_WINDOW_HEIGHT) / 2;
	bottom = MAX(0, excess_height / 2);		// No less than 0
	bottom = MIN(BASE_MARGIN_BOTTOM, bottom);	// No more than 30
	g_video_act_margin_bottom = bottom;
	excess_height -= bottom;
	top = MAX(0, excess_height);
	top = MIN(BASE_MARGIN_TOP, top);
	g_video_act_margin_top = top;
}
