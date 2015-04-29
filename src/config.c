/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2014 by GSport contributors
 
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

#include "defc.h"
#include <stdarg.h>
#include "config.h"
#include "imagewriter.h"
#if defined(__OS2__)
#include "arch\os2\src\dirport.h"
#elif defined(_MSC_VER)
#include "arch\win32\dirent-win32.h"
#else
#include <dirent.h>
#endif

#ifdef HAVE_TFE
#include "tfe/tfesupp.h"
#include "tfe/protos_tfe.h" 
#endif

#if defined _MSC_VER
#include <direct.h>
#define snprintf _snprintf
typedef unsigned int mode_t;
#endif

extern int Verbose;
extern word32 g_vbl_count;
extern Iwm iwm;

extern int g_track_bytes_35[];
extern int g_track_nibs_35[];
extern int g_c031_disk35;

extern int g_cur_a2_stat;
extern byte *g_slow_memory_ptr;
extern byte *g_rom_fc_ff_ptr;
extern byte *g_rom_cards_ptr;
extern double g_cur_dcycs;
extern int g_rom_version;
extern int g_fatal_log;

extern word32 g_adb_repeat_vbl;

extern int halt_sim;
extern int g_limit_speed;
extern int g_force_depth;
extern int g_serial_type[];
extern int g_serial_out_masking;
extern int g_serial_modem[];
extern word32 g_mem_size_base;
extern word32 g_mem_size_exp;
extern int g_video_line_update_interval;
extern int g_video_extra_check_inputs;
extern int g_user_halt_bad;
extern int g_joystick_type;
extern int g_joystick_scale_factor_x;
extern int g_joystick_scale_factor_y;
extern int g_joystick_trim_amount_x;
extern int g_joystick_trim_amount_y;
extern int g_swap_paddles;
extern int g_invert_paddles;
extern int g_ethernet;
extern int g_ethernet_interface;
extern int g_appletalk_bridging;
extern int g_appletalk_turbo;
extern int g_appletalk_diagnostics;
extern int g_appletalk_network_hint;
extern int g_parallel;
extern int g_parallel_out_masking;
extern int g_printer;
extern int g_printer_dpi;
extern char* g_printer_output;
extern int g_printer_multipage;
extern char* g_printer_font_roman;
extern char* g_printer_font_sans;
extern char* g_printer_font_prestige;
extern char* g_printer_font_courier;
extern char* g_printer_font_script;
extern char* g_printer_font_ocra;
extern int g_printer_timeout;

extern int g_imagewriter;
extern int g_imagewriter_dpi;
extern char* g_imagewriter_output;
extern int g_imagewriter_multipage;
extern int g_imagewriter_timeout;
extern char* g_imagewriter_fixed_font;
extern char* g_imagewriter_prop_font;
extern int g_imagewriter_paper;
extern int g_imagewriter_banner;

#if defined(_WIN32) || defined(__CYGWIN__)
extern int g_win_show_console_request;
extern int g_win_status_debug_request;
#endif

extern int g_screen_index[];
extern word32 g_full_refresh_needed;
extern word32 g_a2_screen_buffer_changed;
extern int g_a2_new_all_stat[];
extern int g_new_a2_stat_cur_line;
extern byte g_bram[2][256];
extern byte* g_bram_ptr;
extern byte g_temp_boot_slot;
extern byte g_orig_boot_slot;

extern int g_key_down;
extern const char g_gsport_version_str[];
int g_config_control_panel = 0;
char g_config_gsport_name[1024];
char g_cfg_cwd_str[CFG_PATH_MAX] = { 0 };

int g_config_gsport_auto_update = 1;
int g_config_gsport_update_needed = 0;

const char *g_config_gsport_name_list[] = {
		"config.txt", "config.gsport", "gsport_conf", ".config.gsport", 0
};

int	g_highest_smartport_unit = -1;
int	g_reparse_delay = 0;
int	g_user_page2_shadow = 1;

byte	g_save_text_screen_bytes[0x800];
int	g_save_cur_a2_stat = 0;
char	g_cfg_printf_buf[CFG_PRINTF_BUFSIZE];
char	g_config_gsport_buf[CONF_BUF_LEN];

word32	g_cfg_vbl_count = 0;

int	g_cfg_curs_x = 0;
int	g_cfg_curs_y = 0;
int	g_cfg_curs_inv = 0;
int	g_cfg_curs_mousetext = 0;
int g_cfg_triggeriwreset = 0;

#define CFG_MAX_OPTS	16
#define CFG_OPT_MAXSTR	100

int g_cfg_opts_vals[CFG_MAX_OPTS];
char g_cfg_opts_strs[CFG_MAX_OPTS][CFG_OPT_MAXSTR];
char g_cfg_opts_strvals[CFG_MAX_OPTS][CFG_OPT_MAXSTR];
char g_cfg_opt_buf[CFG_OPT_MAXSTR];

char *g_cfg_rom_path = "ROM";
char *g_cfg_file_def_name = "Undefined";
char **g_cfg_file_strptr = 0;
int g_cfg_file_min_size = 1024;
int g_cfg_file_max_size = 2047*1024*1024;

#define MAX_PARTITION_BLK_SIZE		65536

extern Cfg_menu g_cfg_main_menu[];

#define KNMP(a)		&a, #a, 0

Cfg_menu g_cfg_disk_menu[] = {
{ "Disk Configuration", g_cfg_disk_menu, 0, 0, CFGTYPE_MENU },
{ "s5d1 = ", 0, 0, 0, CFGTYPE_DISK + 0x5000 },
{ "s5d2 = ", 0, 0, 0, CFGTYPE_DISK + 0x5010 },
{ "", 0, 0, 0, 0 },
{ "s6d1 = ", 0, 0, 0, CFGTYPE_DISK + 0x6000 },
{ "s6d2 = ", 0, 0, 0, CFGTYPE_DISK + 0x6010 },
{ "", 0, 0, 0, 0 },
{ "s7d1 = ", 0, 0, 0, CFGTYPE_DISK + 0x7000 },
{ "s7d2 = ", 0, 0, 0, CFGTYPE_DISK + 0x7010 },
{ "s7d3 = ", 0, 0, 0, CFGTYPE_DISK + 0x7020 },
{ "s7d4 = ", 0, 0, 0, CFGTYPE_DISK + 0x7030 },
{ "s7d5 = ", 0, 0, 0, CFGTYPE_DISK + 0x7040 },
{ "s7d6 = ", 0, 0, 0, CFGTYPE_DISK + 0x7050 },
{ "s7d7 = ", 0, 0, 0, CFGTYPE_DISK + 0x7060 },
{ "s7d8 = ", 0, 0, 0, CFGTYPE_DISK + 0x7070 },
{ "s7d9 = ", 0, 0, 0, CFGTYPE_DISK + 0x7080 },
{ "s7d10 = ", 0, 0, 0, CFGTYPE_DISK + 0x7090 },
{ "s7d11 = ", 0, 0, 0, CFGTYPE_DISK + 0x70a0 },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

// OG Use define instead of const for joystick_types
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

Cfg_menu g_cfg_joystick_menu[] = {
{ "Joystick Configuration", g_cfg_joystick_menu, 0, 0, CFGTYPE_MENU },
{ "Joystick Emulation,"TOSTRING(JOYSTICK_TYPE_KEYPAD)",Keypad Joystick,"TOSTRING(JOYSTICK_TYPE_MOUSE)",Mouse Joystick,"TOSTRING(JOYSTICK_TYPE_NATIVE_1)",Native Joystick 1,"
	TOSTRING(JOYSTICK_TYPE_NATIVE_2)",Native Joystick 2,"TOSTRING(JOYSTICK_TYPE_NONE)",No Joystick", KNMP(g_joystick_type), CFGTYPE_INT },
{ "Joystick Scale X,0x100,Standard,0x119,+10%,0x133,+20%,"
	"0x150,+30%,0xb0,-30%,0xcd,-20%,0xe7,-10%",
		KNMP(g_joystick_scale_factor_x), CFGTYPE_INT },
{ "Joystick Scale Y,0x100,Standard,0x119,+10%,0x133,+20%,"
	"0x150,+30%,0xb0,-30%,0xcd,-20%,0xe7,-10%",
		KNMP(g_joystick_scale_factor_y), CFGTYPE_INT },
{ "Joystick Trim X", KNMP(g_joystick_trim_amount_x), CFGTYPE_INT },
{ "Joystick Trim Y", KNMP(g_joystick_trim_amount_y), CFGTYPE_INT },
{ "Swap Joystick X and Y,0,Normal operation,1,Paddle 1 and Paddle 0 swapped",
		KNMP(g_swap_paddles), CFGTYPE_INT },
{ "Invert Joystick,0,Normal operation,1,Left becomes right and up becomes down",
		KNMP(g_invert_paddles), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_rom_menu[] = {
{ "ROM File Selection", g_cfg_rom_menu, 0, 0, CFGTYPE_MENU },
{ "ROM File", KNMP(g_cfg_rom_path), CFGTYPE_FILE },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_serial_menu[] = {
{ "Serial Port Configuration", g_cfg_serial_menu, 0, 0, CFGTYPE_MENU },
{ "Port 0 (slot 1),0,Only use socket 6501,1,Use real port if avail,2,Virtual Imagewriter",
		KNMP(g_serial_type[0]), CFGTYPE_INT },
{ "Port 1 (slot 2),0,Only use socket 6502,1,Use real port if avail,2,Virtual Imagewriter",
		KNMP(g_serial_type[1]), CFGTYPE_INT },
{ "Serial Output,0,Send full 8-bit data,1,Mask off high bit",
		KNMP(g_serial_out_masking), CFGTYPE_INT },
{ "Modem on port 0 (slot 1),0,Simple socket emulation mode,1,Modem with "
		"incoming and outgoing emulation", KNMP(g_serial_modem[0]),
		CFGTYPE_INT },
{ "Modem on port 1 (slot 2),0,Simple socket emulation mode,1,Modem with "
		"incoming and outgoing emulation", KNMP(g_serial_modem[1]),
		CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_parallel_menu[] = {
{ "Parallel Card Configuration", g_cfg_parallel_menu, 0, 0, CFGTYPE_MENU },
{ "Parallel Card in Slot 1,0,Off,1,On",
		KNMP(g_parallel), CFGTYPE_INT },
{ "Parallel Output,0,Send full 8-bit data,1,Mask off high bit",
		KNMP(g_parallel_out_masking), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_ethernet_menu[] = {
{ "Ethernet Card Configuration", g_cfg_ethernet_menu, 0, 0, CFGTYPE_MENU },
{ "Use Interface Number,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10",
	KNMP(g_ethernet_interface), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Uthernet Card in Slot 3,0,Off,1,On",
		KNMP(g_ethernet), CFGTYPE_INT },
#ifdef HAVE_ATBRIDGE
{ "", 0, 0, 0, 0 },
{ "AppleTalk Bridging,0,Off,1,On",
        KNMP(g_appletalk_bridging), CFGTYPE_INT },
{ "AppleTalk Speed,0,Normal (230.4 kbps),1,Turbo",
		KNMP(g_appletalk_turbo), CFGTYPE_INT },
#endif
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_printer_menu[] = {
{ "Virtual Printer Configuration", g_cfg_printer_menu, 0, 0, CFGTYPE_MENU },
{ "Virtual Printer Type,0,Epson LQ",
		KNMP(g_printer), CFGTYPE_INT },
{ "Printer DPI,60,60x60 dpi,180,180x180 dpi,360,360x360 dpi",
		KNMP(g_printer_dpi), CFGTYPE_INT },
{ "Printer Output Type,bmp,Windows Bitmap,ps,Postscript (B&W),printer,Direct to host printer,text,Text file",
		KNMP(g_printer_output), CFGTYPE_STR },
{ "Multipage Files? (PS Only),0,No,1,Yes",
		KNMP(g_printer_multipage), CFGTYPE_INT },
{ "Printer Timeout,0,Never,2,2 sec.,15,15 sec.,30,30 sec.,60, 1 min.",
		KNMP(g_printer_timeout), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Epson LQ Fonts", 0, 0, 0, 0 },
{ "--------------", 0, 0, 0, 0 },
{ "", 0, 0, 0, 0 },
{ "Roman", KNMP(g_printer_font_roman), CFGTYPE_FILE },
{ "Sans Serif", KNMP(g_printer_font_sans), CFGTYPE_FILE },
{ "Courier", KNMP(g_printer_font_courier), CFGTYPE_FILE },
{ "Prestige", KNMP(g_printer_font_prestige), CFGTYPE_FILE },
{ "Script", KNMP(g_printer_font_script), CFGTYPE_FILE },
{ "OCR A/B", KNMP(g_printer_font_ocra), CFGTYPE_FILE },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_imagewriter_menu[] = {
{ "Virtual Imagewriter Configuration", g_cfg_imagewriter_menu, 0, 0, CFGTYPE_MENU },
{ "Virtual Printer Type,0,Imagewriter II,1,Imagewriter LQ",
		KNMP(g_imagewriter), CFGTYPE_INT },
{ "Paper Size,0,US Letter (8.5x11in),1,US Legal (8.5x14in),2,ISO A4 (210 x 297mm),3,ISO B5 (176 x 250mm),4,Wide Fanfold (14 x 11in),5,Ledger (11 x 17in),6,ISO A3 (297 x 420mm)",
		KNMP(g_imagewriter_paper), CFGTYPE_INT },
{ "Printer DPI,360,360x360 dpi (Best for 8-bit software),720,720x720 dpi (Best for GS/OS & IW LQ Modes),1440,1440x1440 dpi",
		KNMP(g_imagewriter_dpi), CFGTYPE_INT },
{ "Banner Printing (Limited To 144x144 dpi Output),0,Banner Printing Off,3,3 Pages Long,4,4 Pages Long,5,5 Pages Long,6,6 Pages Long,7,7 Pages Long,8,8 Pages Long,9,9 Pages Long,10,10 Pages Long",
		KNMP(g_imagewriter_banner), CFGTYPE_INT },
{ "Printer Output Type,bmp,Windows Bitmap,ps,Postscript (B&W),colorps,Postscript (Color),printer,Direct to host printer,text,Text file",
		KNMP(g_imagewriter_output), CFGTYPE_STR },
{ "Multipage Files? (PS Only),0,No,1,Yes",
		KNMP(g_imagewriter_multipage), CFGTYPE_INT },
{ "Printer Timeout,0,Never,2,2 sec.,15,15 sec.,30,30 sec.,60, 1 min.",
		KNMP(g_imagewriter_timeout), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Imagewriter Fonts", 0, 0, 0, 0 },
{ "-----------------", 0, 0, 0, 0 },
{ "", 0, 0, 0, 0 },
{ "Fixed Width Font", KNMP(g_imagewriter_fixed_font), CFGTYPE_FILE },
{ "", 0, 0, 0, 0 },
{ "Proportional Font", KNMP(g_imagewriter_prop_font), CFGTYPE_FILE },
{ "", 0, 0, 0, 0 },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_devel_menu[] = {
{ "Developer Options", g_cfg_devel_menu, 0, 0, CFGTYPE_MENU },
#if defined(_WIN32) || defined(__CYGWIN__)
{ "Status lines,0,Hide,1,Show", KNMP(g_win_status_debug_request), CFGTYPE_INT },
{ "Console,0,Hide,1,Show", KNMP(g_win_show_console_request), CFGTYPE_INT },
#endif
#ifdef HAVE_ATBRIDGE
{ "", 0, 0, 0, 0 },
{ "Show AppleTalk Diagnostics,0,No,1,Yes", KNMP(g_appletalk_diagnostics), CFGTYPE_INT },
{ "AppleTalk Network Hint", KNMP(g_appletalk_network_hint), CFGTYPE_INT },
#endif
{ "", 0, 0, 0, 0 },
#ifndef _WIN32
{ "Force X-windows display depth", KNMP(g_force_depth), CFGTYPE_INT },
#endif
{ "Code Red Halts,0,Do not stop on bad accesses,1,Enter debugger on bad accesses", KNMP(g_user_halt_bad), CFGTYPE_INT },
{ "3200 Color Enable,0,Auto (Full if fast enough),1,Full (Update every line),8,Off (Update video every 8 lines)", KNMP(g_video_line_update_interval), CFGTYPE_INT },
{ "Keyboard and mouse poll rate,0,60 times per second,1,240 times per second", KNMP(g_video_extra_check_inputs), CFGTYPE_INT },
{ "Enable Text Page 2 Shadow,0,Disabled on ROM 01 (matches real hardware),1,Enabled on ROM 01 and 03", KNMP(g_user_page2_shadow), CFGTYPE_INT },
{ "", 0, 0, 0, 0 },
{ "Back to Main Config", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ 0, 0, 0, 0, 0 },
};

Cfg_menu g_cfg_main_menu[] = {
{ "GSport Configuration", g_cfg_main_menu, 0, 0, CFGTYPE_MENU },
{ "Disk Configuration", g_cfg_disk_menu, 0, 0, CFGTYPE_MENU },
{ "Joystick Configuration", g_cfg_joystick_menu, 0, 0, CFGTYPE_MENU },
{ "ROM File Selection", g_cfg_rom_menu, 0, 0, CFGTYPE_MENU },
{ "Serial Port Configuration", g_cfg_serial_menu, 0, 0, CFGTYPE_MENU },
{ "Ethernet Card Configuration", g_cfg_ethernet_menu, 0, 0, CFGTYPE_MENU },
{ "Parallel Card Configuration", g_cfg_parallel_menu, 0, 0, CFGTYPE_MENU },
{ "Virtual Printer Configuration", g_cfg_printer_menu, 0, 0, CFGTYPE_MENU },
{ "Virtual Imagewriter Configuration", g_cfg_imagewriter_menu, 0, 0, CFGTYPE_MENU },
{ "Developer Options", g_cfg_devel_menu, 0, 0, CFGTYPE_MENU },
{ "Auto-update configuration file,0,Manual,1,Immediately",
		KNMP(g_config_gsport_auto_update), CFGTYPE_INT },
{ "Speed,0,Unlimited,1,1.0MHz,2,2.8MHz,3,8.0MHz (Zip)",
		KNMP(g_limit_speed), CFGTYPE_INT },
{ "Expansion Mem Size,0,0MB,0x100000,1MB,0x200000,2MB,0x300000,3MB,"
	"0x400000,4MB,0x600000,6MB,0x800000,8MB,0xa00000,10MB,0xc00000,12MB,"
	"0xe00000,14MB", KNMP(g_mem_size_exp), CFGTYPE_INT },
{ "Dump text screen to file", (void *)cfg_text_screen_dump, 0, 0, CFGTYPE_FUNC},
{ "Reset Virtual Imagewriter", (void *)cfg_iwreset, 0, 0, CFGTYPE_FUNC },
{ "", 0, 0, 0, 0 },
{ "Save changes to configuration file", (void *)config_write_config_gsport_file, 0, 0, 
		CFGTYPE_FUNC },
{ "", 0, 0, 0, 0 },
{ "Exit Config (or press F4)", (void *)cfg_exit, 0, 0, CFGTYPE_FUNC },
{ 0, 0, 0, 0, 0 },
};


#define CFG_MAX_DEFVALS	128
Cfg_defval g_cfg_defvals[CFG_MAX_DEFVALS];
int g_cfg_defval_index = 0;

int g_cfg_slotdrive = -1;
int g_cfg_select_partition = -1;
char g_cfg_tmp_path[CFG_PATH_MAX];
char g_cfg_file_path[CFG_PATH_MAX];
char g_cfg_file_cachedpath[CFG_PATH_MAX];
char g_cfg_file_cachedreal[CFG_PATH_MAX];
char g_cfg_file_curpath[CFG_PATH_MAX];
char g_cfg_file_shortened[CFG_PATH_MAX];
char g_cfg_file_match[CFG_PATH_MAX];

Cfg_listhdr g_cfg_dirlist = { 0 };
Cfg_listhdr g_cfg_partitionlist = { 0 };

int g_cfg_file_pathfield = 0;

const char *g_gsport_rom_names[] = { "ROM", "ROM", "ROM.01", "ROM.03", 0 };
	/* First entry is special--it will be overwritten by g_cfg_rom_path */

const char *g_gsport_c1rom_names[] = { "parallel.rom", 0 };
const char *g_gsport_c2rom_names[] = { 0 };
const char *g_gsport_c3rom_names[] = { 0 };
const char *g_gsport_c4rom_names[] = { 0 };
const char *g_gsport_c5rom_names[] = { 0 };
const char *g_gsport_c6rom_names[] = { "c600.rom", "controller.rom", "disk.rom",
				"DISK.ROM", "diskII.prom", 0 };
const char *g_gsport_c7rom_names[] = { 0 };

const char **g_gsport_rom_card_list[8] = {
	0,			g_gsport_c1rom_names,
	g_gsport_c2rom_names,	g_gsport_c3rom_names,
	g_gsport_c4rom_names,	g_gsport_c5rom_names,
	g_gsport_c6rom_names,	g_gsport_c7rom_names };

byte g_rom_c600_rom01_diffs[256] = {
	0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0xe2, 0x00,
	0xd0, 0x50, 0x0f, 0x77, 0x5b, 0xb9, 0xc3, 0xb1,
	0xb1, 0xf8, 0xcb, 0x4e, 0xb8, 0x60, 0xc7, 0x2e,
	0xfc, 0xe0, 0xbf, 0x1f, 0x66, 0x37, 0x4a, 0x70,
	0x55, 0x2c, 0x3c, 0xfc, 0xc2, 0xa5, 0x08, 0x29,
	0xac, 0x21, 0xcc, 0x09, 0x55, 0x03, 0x17, 0x35,
	0x4e, 0xe2, 0x0c, 0xe9, 0x3f, 0x9d, 0xc2, 0x06,
	0x18, 0x88, 0x0d, 0x58, 0x57, 0x6d, 0x83, 0x8c,
	0x22, 0xd3, 0x4f, 0x0a, 0xe5, 0xb7, 0x9f, 0x7d,
	0x2c, 0x3e, 0xae, 0x7f, 0x24, 0x78, 0xfd, 0xd0,
	0xb5, 0xd6, 0xe5, 0x26, 0x85, 0x3d, 0x8d, 0xc9,
	0x79, 0x0c, 0x75, 0xec, 0x98, 0xcc, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x39, 0x00, 0x35, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00,
	0x6c, 0x44, 0xce, 0x4c, 0x01, 0x08, 0x00, 0x00
};


void
config_init_menus(Cfg_menu *menuptr)
{
	void	*voidptr;
	const char *name_str;
	Cfg_defval *defptr;
	char	**str_ptr;
	char	*str;
	int	type;
	int	pos;
	int	val;

	if(menuptr[0].defptr != 0) {
		return;
	}
	menuptr[0].defptr = (void *)1;
	pos = 0;
	while(pos < 100) {
		type = menuptr->cfgtype;
		voidptr = menuptr->ptr;
		name_str = menuptr->name_str;
		if(menuptr->str == 0) {
			break;
		}
		if(name_str != 0) {
			defptr = &(g_cfg_defvals[g_cfg_defval_index++]);
			if(g_cfg_defval_index >= CFG_MAX_DEFVALS) {
				fatal_printf("CFG_MAX_DEFVAL overflow\n");
				my_exit(5);
			}
			defptr->menuptr = menuptr;
			defptr->intval = 0;
			defptr->strval = 0;
			switch(type) {
			case CFGTYPE_INT:
				val = *((int *)voidptr);
				defptr->intval = val;
				menuptr->defptr = &(defptr->intval);
				break;
			case CFGTYPE_STR:
				str_ptr = (char **)menuptr->ptr;
				str = *str_ptr;
				// We need to malloc this string since all
				//  string values must be dynamically alloced
				defptr->strval = str;	// this can have a copy
				*str_ptr = gsport_malloc_str(str);
				menuptr->defptr = &(defptr->strval);
				break;
			case CFGTYPE_FILE:
				str_ptr = (char **)menuptr->ptr;
				str = *str_ptr;
				// We need to malloc this string since all
				//  string values must be dynamically alloced
				defptr->strval = str;	// this can have a copy
				*str_ptr = gsport_malloc_str(str);
				menuptr->defptr = &(defptr->strval);
				break;
			default:
				fatal_printf("name_str is %p = %s, but type: "
					"%d\n", name_str, name_str, type);
				my_exit(5);
			}
		}
		if(type == CFGTYPE_MENU) {
			config_init_menus((Cfg_menu *)voidptr);
		}
		pos++;
		menuptr++;
	}
}

void
config_init()
{
	int	can_create;

	config_init_menus(g_cfg_main_menu);

	// Find the configuration file
	g_config_gsport_name[0] = 0;
	can_create = 1;
	setup_gsport_file(&g_config_gsport_name[0], sizeof(g_config_gsport_name), 0,
				can_create, &g_config_gsport_name_list[0]);

	config_parse_config_gsport_file();
}

void
cfg_exit()
{
	/* printf("In cfg exit\n"); */
	if(g_rom_version >= 1) {
		g_config_control_panel = 0;
	}
}

void
cfg_toggle_config_panel()
{
	g_config_control_panel = !g_config_control_panel;
	if(g_rom_version < 0) {
		g_config_control_panel = 1;	/* Stay in config mode */
	}
}

void
cfg_text_screen_dump()
{
	char	buf[85];
	char	*filename;
	FILE	*ofile;
	int	offset;
	int	c;
	int	pos;
	int	i, j;

	filename = "gsport.screen.dump";
	printf("Writing text screen to the file %s\n", filename);
	ofile = fopen(filename, "w");
	if(ofile == 0) {
		fatal_printf("Could not write to file %s, (%d)\n", filename,
				errno);
		return;
	}

	for(i = 0; i < 24; i++) {
		pos = 0;
		for(j = 0; j < 40; j++) {
			offset = g_screen_index[i] + j;
			if(g_save_cur_a2_stat & ALL_STAT_VID80) {
				c = g_save_text_screen_bytes[0x400+offset];
				c = c & 0x7f;
				if(c < 0x20) {
					c += 0x40;
				}
				buf[pos++] = c;
			}
			c = g_save_text_screen_bytes[offset] & 0x7f;
			if(c < 0x20) {
				c += 0x40;
			}
			buf[pos++] = c;
		}
		while((pos > 0) && (buf[pos-1] == ' ')) {
			/* try to strip out trailing spaces */
			pos--;
		}
		buf[pos++] = '\n';
		buf[pos++] = 0;
		fputs(buf, ofile);
	}
	fclose(ofile);
}
void
cfg_iwreset()
{
		imagewriter_feed();
		imagewriter_close();
		imagewriter_init(g_imagewriter_dpi,g_imagewriter_paper,g_imagewriter_banner, g_imagewriter_output,g_imagewriter_multipage);
		return;
}
#ifdef HAVE_TFE
void 
cfg_get_tfe_name()
{   
	int i = 0;
	char *ppname = NULL;
	char *ppdes = NULL;
	cfg_htab_vtab(0,11);
	if (tfe_enumadapter_open())
	{
	cfg_printf("Interface List:\n---------------");
	while(tfe_enumadapter(&ppname,&ppdes))
	{
		cfg_htab_vtab(0, 13+i);
		cfg_printf("%2d: %s",i,ppdes);
		i++;
		lib_free(ppname);
		lib_free(ppdes);
	}
	tfe_enumadapter_close();
	}
	else
	{
		#if defined(_WIN32) || defined(__CYGWIN__)
		cfg_printf("ERROR: Install/Enable WinPcap for Ethernet Support!!");	
		#else
		cfg_printf("ERROR: Install/Enable LibPcap for Ethernet Support!!");	
		#endif
	}
	return;
}
#endif

void
config_vbl_update(int doit_3_persec)
{
	if(doit_3_persec) {
		if(g_config_gsport_auto_update && g_config_gsport_update_needed) {
			config_write_config_gsport_file();
		}
	}
	return;
}

void
config_parse_option(char *buf, int pos, int len, int line)
{
	Cfg_menu *menuptr;
	Cfg_defval *defptr;
	char	*nameptr;
	char 	**strptr;
	int	*iptr;
	int	num_equals;
	int	type;
	int	val;
	int	c;
	int	i;

// warning: modifies buf (turns spaces to nulls)
// parse buf from pos into option, "=" and then "rest"
	if(pos >= len) {
		/* blank line */
		return;
	}

	if(strncmp(&buf[pos], "bram", 4) == 0) {
		config_parse_bram(buf, pos+4, len);
		return;
	}

	// find "name" as first contiguous string
	printf("...parse_option: line %d, %p,%p = %s (%s) len:%d\n", line,
		&buf[pos], buf, &buf[pos], buf, len);

	nameptr = &buf[pos];
	while(pos < len) {
		c = buf[pos];
		if(c == 0 || c == ' ' || c == '\t' || c == '\n') {
			break;
		}
		pos++;
	}
	buf[pos] = 0;
	pos++;

	// Eat up all whitespace and '='
	num_equals = 0;
	while(pos < len) {
		c = buf[pos];
		if((c == '=') && num_equals == 0) {
			pos++;
			num_equals++;
		} else if(c == ' ' || c == '\t') {
			pos++;
		} else {
			break;
		}
	}

	/* Look up nameptr to find type */
	type = -1;
	defptr = 0;
	menuptr = 0;
	for(i = 0; i < g_cfg_defval_index; i++) {
		defptr = &(g_cfg_defvals[i]);
		menuptr = defptr->menuptr;
		if(strcmp(menuptr->name_str, nameptr) == 0) {
			type = menuptr->cfgtype;
			break;
		}
	}

	switch(type) {
	case CFGTYPE_INT:
		/* use strtol */
		val = (int)strtol(&buf[pos], 0, 0);
		iptr = (int *)menuptr->ptr;
		*iptr = val;
		break;
	case CFGTYPE_STR:
		strptr = (char **)menuptr->ptr;
		if(strptr && *strptr) {
			free(*strptr);
		}
		*strptr = gsport_malloc_str(&buf[pos]);
		break;
	case CFGTYPE_FILE:
		strptr = (char **)menuptr->ptr;
		if(strptr && *strptr) {
			free(*strptr);
		}
		*strptr = gsport_malloc_str(&buf[pos]);
		break;
	default:
		printf("Config file variable %s is unknown type: %d\n",
			nameptr, type);
	}

}

void
config_parse_bram(char *buf, int pos, int len)
{
	int	bram_num;
	int	offset;
	int	val;

	if((len < (pos+5)) || (buf[pos+1] != '[') || (buf[pos+4] != ']')) {
		fatal_printf("While reading configuration file, found malformed bram "
			"statement: %s\n", buf);
		return;
	}
	bram_num = buf[pos] - '0';
	if(bram_num != 1 && bram_num != 3) {
		fatal_printf("While reading configuration file, found bad bram "
			"num: %s\n", buf);
		return;
	}

	bram_num = bram_num >> 1;	// turn 3->1 and 1->0

	offset = strtoul(&(buf[pos+2]), 0, 16);
	pos += 5;
	while(pos < len) {
		while(buf[pos] == ' ' || buf[pos] == '\t' || buf[pos] == 0x0a ||
				buf[pos] == 0x0d || buf[pos] == '=') {
			pos++;
		}
		val = strtoul(&buf[pos], 0, 16);
		clk_bram_set(bram_num, offset, val);
		offset++;
		pos += 2;
	}
}

void
config_load_roms()
{
	struct stat stat_buf;
	const char **names_ptr;
	int	more_than_8mb;
	int	changed_rom;
	int	len;
	FILE *file;
	int	ret;
	int	i;

	g_rom_version = -1;

	/* set first entry of g_gsport_rom_names[] to g_cfg_rom_path so that */
	/*  it becomes the first place searched. */
	g_gsport_rom_names[0] = g_cfg_rom_path;
	setup_gsport_file(&g_cfg_tmp_path[0], CFG_PATH_MAX, -1, 0,
							&g_gsport_rom_names[0]);

	if(g_cfg_tmp_path[0] == 0) {
		// Just get out, let config interface select ROM
		g_config_control_panel = 1;
		return;
	}
	file = fopen(&g_cfg_tmp_path[0], "rb");
	if(!file) {
		fatal_printf("Open ROM file %s failed; errno:%d\n",
				&g_cfg_tmp_path[0], errno);
		g_config_control_panel = 1;
		return;
	}

	ret = stat(&g_cfg_tmp_path[0], &stat_buf);
	if(ret != 0) {
		fatal_printf("stat returned %d; errno: %d\n",
			ret, errno);
		g_config_control_panel = 1;
		return;
	}

	len = stat_buf.st_size;
	if(len == 128*1024) {
		g_rom_version = 1;
		g_mem_size_base = 256*1024;
		memset(&g_rom_fc_ff_ptr[0], 0, 2*65536);
				/* Clear banks fc and fd to 0 */
		ret = fread(&g_rom_fc_ff_ptr[2*65536], 1, len, file);
	} else if(len == 256*1024) {
		g_rom_version = 3;
		g_mem_size_base = 1024*1024;
		ret = fread(&g_rom_fc_ff_ptr[0], 1, len, file);
	} else {
		fatal_printf("The ROM size should be 128K or 256K, this file "
						"is %d bytes\n", len);
		g_config_control_panel = 1;
		return;
	}

	printf("Read: %d bytes of ROM\n", ret);
	if(ret != len) {
		fatal_printf("errno: %d\n", errno);
		g_config_control_panel = 1;
		return;
	}
	fclose(file);

	memset(&g_rom_cards_ptr[0], 0, 256*16);

	/* initialize c600 rom to be diffs from the real ROM, to build-in */
	/*  Apple II compatibility without distributing ROMs */
	for(i = 0; i < 256; i++) {
		g_rom_cards_ptr[0x600 + i] = g_rom_fc_ff_ptr[0x3c600 + i] ^
				g_rom_c600_rom01_diffs[i];
	}
	if(g_rom_version >= 3) {
		/* some patches */
		g_rom_cards_ptr[0x61b] ^= 0x40;
		g_rom_cards_ptr[0x61c] ^= 0x33;
		g_rom_cards_ptr[0x632] ^= 0xc0;
		g_rom_cards_ptr[0x633] ^= 0x33;
	}

	for(i = 1; i < 8; i++) {
		names_ptr = g_gsport_rom_card_list[i];
		if(names_ptr == 0) {
			continue;
		}
		if(*names_ptr == 0) {
			continue;
		}
		setup_gsport_file(&g_cfg_tmp_path[0], CFG_PATH_MAX, 1, 0,
								names_ptr);
		if(g_cfg_tmp_path[0] != 0) {
			file = fopen(&(g_cfg_tmp_path[0]), "rb");
			if(!file) {
				fatal_printf("Open card ROM file %s failed; errno:%d\n",
					&g_cfg_tmp_path[0], errno);
				continue;
			}

			len = 256;
			ret = fread(&g_rom_cards_ptr[i*0x100], 1, len, file);

			if(ret != len) {
				fatal_printf("While reading card ROM %s, file "
					"is too short. (%d) Expected %d bytes, "
					"read %d bytes\n", &g_cfg_tmp_path[0], errno, len, ret);
				continue;
			}
			printf("Read: %d bytes of ROM in slot %d from file %s.\n", ret, i, &g_cfg_tmp_path[0]);
			fclose(file);
		}
	}
	more_than_8mb = (g_mem_size_exp > 0x800000);
	/* Only do the patch if users wants more than 8MB of expansion mem */

	changed_rom = 0;
	if(g_rom_version == 1) {
		/* make some patches to ROM 01 */
#if 0
		/* 1: Patch ROM selftest to not do speed test */
		printf("Patching out speed test failures from ROM 01\n");
		g_rom_fc_ff_ptr[0x3785a] = 0x18;
		changed_rom = 1;
#endif

#if 0
		/* 2: Patch ROM selftests not to do tests 2,4 */
		/* 0 = skip, 1 = do it, test 1 is bit 0 of LSByte */
		g_rom_fc_ff_ptr[0x371e9] = 0xf5;
		g_rom_fc_ff_ptr[0x371ea] = 0xff;
		changed_rom = 1;
#endif

		if(more_than_8mb) {
			/* Geoff Weiss patch to use up to 14MB of RAM */
			g_rom_fc_ff_ptr[0x30302] = 0xdf;
			g_rom_fc_ff_ptr[0x30314] = 0xdf;
			g_rom_fc_ff_ptr[0x3031c] = 0x00;
			changed_rom = 1;
		}

		/* Patch ROM selftest to not do ROM cksum if any changes*/
		if(changed_rom) {
			g_rom_fc_ff_ptr[0x37a06] = 0x18;
			g_rom_fc_ff_ptr[0x37a07] = 0x18;
		}
	} else if(g_rom_version == 3) {
		/* patch ROM 03 */
		printf("Patching ROM 03 smartport bug\n");
		/* 1: Patch Smartport code to fix a stupid bug */
		/*   that causes it to write the IWM status reg into c036, */
		/*   which is the system speed reg...it's "safe" since */
		/*   IWM status reg bit 4 must be 0 (7MHz)..., otherwise */
		/*   it might have turned on shadowing in all banks! */
		g_rom_fc_ff_ptr[0x357c9] = 0x00;
		changed_rom = 1;

#if 0
		/* patch ROM 03 to not to speed test */
		/*  skip fast speed test */
		g_rom_fc_ff_ptr[0x36ad7] = 0x18;
		g_rom_fc_ff_ptr[0x36ad8] = 0x18;
		changed_rom = 1;
#endif

#if 0
		/*  skip slow speed test */
		g_rom_fc_ff_ptr[0x36ae7] = 0x18;
		g_rom_fc_ff_ptr[0x36ae8] = 0x6b;
		changed_rom = 1;
#endif

#if 0
		/* 4: Patch ROM 03 selftests not to do tests 1-4 */
		g_rom_fc_ff_ptr[0x364a9] = 0xf0;
		g_rom_fc_ff_ptr[0x364aa] = 0xff;
		changed_rom = 1;
#endif

		/* ROM tests are in ff/6403-642x, where 6403 = addr of */
		/*  test 1, etc. */

		if(more_than_8mb) {
			/* Geoff Weiss patch to use up to 14MB of RAM */
			g_rom_fc_ff_ptr[0x30b] = 0xdf;
			g_rom_fc_ff_ptr[0x31d] = 0xdf;
			g_rom_fc_ff_ptr[0x325] = 0x00;
			changed_rom = 1;
		}

		if(changed_rom) {
			/* patch ROM 03 selftest to not do ROM cksum */
			g_rom_fc_ff_ptr[0x36cb0] = 0x18;
			g_rom_fc_ff_ptr[0x36cb1] = 0x18;
		}

	}
}

void
config_parse_config_gsport_file()
{
	FILE	*fconf;
	char	*buf;
	char	*ptr;
	char	*name_ptr;
	char	*partition_name;
	int	part_num;
	int	ejected;
	int	line;
	int	pos;
	int	slot;
	int	drive;
	int	size;
	int	len;
	int	ret;
	int	i;

	printf("Parsing configuration file\n");

	clk_bram_zero();

	g_highest_smartport_unit = -1;

	cfg_get_base_path(&g_cfg_cwd_str[0], g_config_gsport_name, 0);
#ifndef __OS2__
	if(g_cfg_cwd_str[0] != 0) {
		ret = chdir(&g_cfg_cwd_str[0]);
		if(ret != 0) {
			printf("chdir to %s, errno:%d\n", g_cfg_cwd_str, errno);
		}
	}
	/* In any case, copy the directory path to g_cfg_cwd_str */
	(void)getcwd(&g_cfg_cwd_str[0], CFG_PATH_MAX);
#endif

	fconf = fopen(g_config_gsport_name, "r");
	if(fconf == 0) {
		fatal_printf("cannot open configuration file at %s!  Stopping!\n",
				g_config_gsport_name);
		my_exit(3);
	}

	line = 0;
	while(1) {
		buf = &g_config_gsport_buf[0];
		ptr = fgets(buf, CONF_BUF_LEN, fconf);
		if(ptr == 0) {
			iwm_printf("Done reading disk_conf\n");
			break;
		}

		line++;
		/* strip off newline(s) */
		len = strlen(buf);
		for(i = len - 1; i >= 0; i--) {
			if((buf[i] != 0x0d) && (buf[i] != 0x0a)) {
				break;
			}
			len = i;
			buf[i] = 0;
		}

		iwm_printf("disk_conf[%d]: %s\n", line, buf);
		if(len > 0 && buf[0] == '#') {
			iwm_printf("Skipping comment\n");
			continue;
		}

		/* determine what this is */
		pos = 0;

		while(pos < len && (buf[pos] == ' ' || buf[pos] == '\t') ) {
			pos++;
		}
		if((pos + 4) > len || buf[pos] != 's' || buf[pos+2] != 'd' ||
				buf[pos+1] > '9' || buf[pos+1] < '0') {
			config_parse_option(buf, pos, len, line);
			continue;
		}

		slot = buf[pos+1] - '0';
		drive = buf[pos+3] - '0';

		/* skip over slot, drive */
		pos += 4;
		if(buf[pos] >= '0' && buf[pos] <= '9') {
			drive = drive * 10 + buf[pos] - '0';
			pos++;
		}

		/*	make s6d1 mean index 0 */
		drive--;

		while(pos < len && (buf[pos] == ' ' || buf[pos] == '\t' ||
					buf[pos] == '=') ) {
			pos++;
		}

		ejected = 0;
		if(buf[pos] == '#') {
			/* disk is ejected, but read all the info anyway */
			ejected = 1;
			pos++;
		}

		size = 0;
		if(buf[pos] == ',') {
			/* read optional size parameter */
			pos++;
			while(pos < len && buf[pos] >= '0' && buf[pos] <= '9'){
				size = size * 10 + buf[pos] - '0';
				pos++;
			}
			size = size * 1024;
			if(buf[pos] == ',') {
				pos++;		/* eat trailing ',' */
			}
		}

		/* see if it has a partition name */
		partition_name = 0;
		part_num = -1;
		if(buf[pos] == ':') {
			pos++;
			/* yup, it's got a partition name! */
			partition_name = &buf[pos];
			while((pos < len) && (buf[pos] != ':')) {
				pos++;
			}
			buf[pos] = 0;	/* null terminate partition name */
			pos++;
		}
		if(buf[pos] == ';') {
			pos++;
			/* it's got a partition number */
			part_num = 0;
			while((pos < len) && (buf[pos] != ':')) {
				part_num = (10*part_num) + buf[pos] - '0';
				pos++;
			}
			pos++;
		}

		/* Get filename */
		name_ptr = &(buf[pos]);
		if(name_ptr[0] == 0) {
			continue;
		}

		insert_disk(slot, drive, name_ptr, ejected, size,
						partition_name, part_num);

	}

	ret = fclose(fconf);
	if(ret != 0) {
		fatal_printf("Closing configuration file ret: %d, errno: %d\n", ret,
						errno);
		my_exit(4);
	}

	iwm_printf("Done parsing disk_conf file\n");
}


Disk *
cfg_get_dsk_from_slot_drive(int slot, int drive)
{
	Disk	*dsk;
	int	max_drive;

	/* Get dsk */
	max_drive = 2;
	switch(slot) {
	case 5:
		dsk = &(iwm.drive35[drive]);
		break;
	case 6:
		dsk = &(iwm.drive525[drive]);
		break;
	default:
		max_drive = MAX_C7_DISKS;
		dsk = &(iwm.smartport[drive]);
	}

	if(drive >= max_drive) {
		dsk -= drive;	/* move back to drive 0 effectively */
	}

	return dsk;
}

void
config_generate_config_gsport_name(char *outstr, int maxlen, Disk *dsk,
		int with_extras)
{
	char	*str;

	str = outstr;

	if(with_extras && (!dsk->file)) {
		snprintf(str, maxlen - (str - outstr), "#");
		str = &outstr[strlen(outstr)];
	}
	if(with_extras && dsk->force_size > 0) {
		snprintf(str, maxlen - (str - outstr), ",%d,", dsk->force_size);
		str = &outstr[strlen(outstr)];
	}
	if(with_extras && dsk->partition_name != 0) {
		snprintf(str, maxlen - (str - outstr), ":%s:",
							dsk->partition_name);
		str = &outstr[strlen(outstr)];
	} else if(with_extras && dsk->partition_num >= 0) {
		snprintf(str, maxlen - (str - outstr), ";%d:",
							dsk->partition_num);
		str = &outstr[strlen(outstr)];
	}
	snprintf(str, maxlen - (str - outstr), "%s", dsk->name_ptr);
}

void
config_write_config_gsport_file()
{
	FILE	*fconf;
	Disk	*dsk;
	Cfg_defval *defptr;
	Cfg_menu *menuptr;
	char	*curstr, *defstr;
	int	defval, curval;
	int	type;
	int	slot, drive;
	int	i;

	printf("Writing configuration file to %s\n", g_config_gsport_name);

	fconf = fopen(g_config_gsport_name, "w+");
	if(fconf == 0) {
		halt_printf("cannot open %s!  Stopping!\n",g_config_gsport_name);
		return;
	}

	fprintf(fconf, "# GSport configuration file version %s\n",
						g_gsport_version_str);

	for(i = 0; i < MAX_C7_DISKS + 4; i++) {
		slot = 7;
		drive = i - 4;
		if(i < 4) {
			slot = (i >> 1) + 5;
			drive = i & 1;
		}
		if(drive == 0) {
			fprintf(fconf, "\n");	/* an extra blank line */
		}

		dsk = cfg_get_dsk_from_slot_drive(slot, drive);
		if(dsk->name_ptr == 0 && (i > 4)) {
			/* No disk, not even ejected--just skip */
			continue;
		}
		fprintf(fconf, "s%dd%d = ", slot, drive + 1);
		if(dsk->name_ptr == 0) {
			fprintf(fconf, "\n");
			continue;
		}
		config_generate_config_gsport_name(&g_cfg_tmp_path[0],
							CFG_PATH_MAX, dsk, 1);
		fprintf(fconf, "%s\n", &g_cfg_tmp_path[0]);
	}

	fprintf(fconf, "\n");

	/* See if any variables are different than their default */
	for(i = 0; i < g_cfg_defval_index; i++) {
		defptr = &(g_cfg_defvals[i]);
		menuptr = defptr->menuptr;
		defval = defptr->intval;
		type = menuptr->cfgtype;
		if(type == CFGTYPE_INT) {
			curval = *((int *)menuptr->ptr);
			if(curval != defval) {
				fprintf(fconf, "%s = %d\n", menuptr->name_str,
								curval);
			}
		}
		if(type == CFGTYPE_STR) {
			curstr = *((char **)menuptr->ptr);
			defstr = *((char **)menuptr->defptr);
			if(strcmp(curstr, defstr) != 0) {
				fprintf(fconf, "%s = %s\n", menuptr->name_str,
								curstr);
			}
		}
		if(type == CFGTYPE_FILE) {
			curstr = *((char **)menuptr->ptr);
			defstr = *((char **)menuptr->defptr);
			if(strcmp(curstr, defstr) != 0) {
				fprintf(fconf, "%s = %s\n", menuptr->name_str,
								curstr);
			}
		}
	}

	fprintf(fconf, "\n");

	/* write bram state */
	clk_write_bram(fconf);

	fclose(fconf);

	g_config_gsport_update_needed = 0;
}

void
insert_disk(int slot, int drive, const char *name, int ejected, int force_size,
		const char *partition_name, int part_num)
{
	byte	buf_2img[512];
	Disk	*dsk;
	char	*name_ptr, *uncomp_ptr, *system_str;
	char	*part_ptr;
	int	size;
	int	system_len;
	int	part_len;
	int	cmp_o, cmp_p, cmp_dot;
	int	cmp_b, cmp_i, cmp_n;
	int	can_write;
	int	len;
	int	nibs;
	int	unix_pos;
	int	name_len;
	int	image_identified;
	int	exp_size;
	int	save_track;
	int	ret;
	int	tmp;
	int	i;

	g_config_gsport_update_needed = 1;

	if((slot < 5) || (slot > 7)) {
		fatal_printf("Invalid slot for inserting disk: %d\n", slot);
		return;
	}
	if(drive < 0 || ((slot == 7) && (drive >= MAX_C7_DISKS)) ||
					((slot < 7) && (drive > 1))) {
		fatal_printf("Invalid drive for inserting disk: %d\n", drive);
		return;
	}

	dsk = cfg_get_dsk_from_slot_drive(slot, drive);

#if 0
	printf("Inserting disk %s (%s or %d) in slot %d, drive: %d\n", name,
		partition_name, part_num, slot, drive);
#endif

	dsk->just_ejected = 0;
	dsk->force_size = force_size;

	if(!dsk->file) {
		eject_disk(dsk);
	}

	/* Before opening, make sure no other mounted disk has this name */
	/* If so, unmount it */
	if(!ejected) {
		for(i = 0; i < 2; i++) {
			eject_named_disk(&iwm.drive525[i], name,partition_name);
			eject_named_disk(&iwm.drive35[i], name, partition_name);
		}
		for(i = 0; i < MAX_C7_DISKS; i++) {
			eject_named_disk(&iwm.smartport[i],name,partition_name);
		}
	}

	if(dsk->name_ptr != 0) {
		/* free old name_ptr */
		free(dsk->name_ptr);
	}

	name_len = strlen(name);
	name_ptr = (char *)malloc(name_len + 1);
#if defined(_WIN32) || defined(__CYGWIN__)
	// On Windows, we need to change backslashes to forward slashes.
	for (i = 0; i < name_len; i++) {
		if (name[i] == '\\') {
			name_ptr[i] = '/';
		} else {
			name_ptr[i] = name[i];
		}
	}
	name_ptr[name_len] = 0;
#else
	strncpy(name_ptr, name, name_len + 1);
#endif
	dsk->name_ptr = name_ptr;

	dsk->partition_name = 0;
	if(partition_name != 0) {
		part_len = strlen(partition_name) + 1;
		part_ptr = (char *)malloc(part_len);
		strncpy(part_ptr, partition_name, part_len);
		dsk->partition_name = part_ptr;
	}
	dsk->partition_num = part_num;

	iwm_printf("Opening up disk image named: %s\n", name_ptr);

	if(ejected) {
		/* just get out of here */
		dsk->file = 0;
		return;
	}

	dsk->file = 0;
	can_write = 1;

	if((name_len > 3) && (strcmp(&name_ptr[name_len - 3], ".gz") == 0)) {

		/* it's gzip'ed, try to gunzip it, then unlink the */
		/*   uncompressed file */

		can_write = 0;

		uncomp_ptr = (char *)malloc(name_len + 1);
		strncpy(uncomp_ptr, name_ptr, name_len + 1);
		uncomp_ptr[name_len - 3] = 0;

		system_len = 2*name_len + 100;
		system_str = (char *)malloc(system_len + 1);
		snprintf(system_str, system_len,
			"set -o noclobber;gunzip -c %c%s%c > %c%s%c",
			0x22, name_ptr, 0x22,
			0x22, uncomp_ptr, 0x22);
		/* 0x22 are " to allow spaces in filenames */
		printf("I am uncompressing %s into %s for mounting\n",
							name_ptr, uncomp_ptr);
		ret = system(system_str);
		if(ret == 0) {
			/* successfully ran */
			dsk->file = fopen(uncomp_ptr, "rb");
			iwm_printf("Opening .gz file %s\n",	uncomp_ptr);

			/* and, unlink the temporary file */
			(void)unlink(uncomp_ptr);
		}
		free(system_str);
		free(uncomp_ptr);
		/* Reduce name_len by 3 so that subsequent compares for .po */
		/*  look at the correct chars */
		name_len -= 3;
	}

	if((!dsk->file) && can_write) {
		dsk->file = fopen(name_ptr, "rb+");
	}

	if((!dsk->file) && can_write) {
		printf("Trying to open %s read-only, errno: %d\n", name_ptr,
								errno);
		dsk->file = fopen(name_ptr, "rb");
		can_write = 0;
	}

	if(!dsk->file) {
		fatal_printf("Disk image %s does not exist!\n", name_ptr);
		return;
	}

	if(can_write != 0) {
		dsk->write_prot = 0;
		dsk->write_through_to_unix = 1;
	} else {
		dsk->write_prot = 1;
		dsk->write_through_to_unix = 0;
	}

	save_track = dsk->cur_qtr_track;	/* save arm position */
	dsk->image_type = DSK_TYPE_PRODOS;
	dsk->image_start = 0;

	/* See if it is in 2IMG format */
	ret = fread((char *)&buf_2img[0], 1, 512, dsk->file);
	size = force_size;
	if(size <= 0) {
		size = cfg_get_fd_size(name_ptr);
	}

	/* Try to guess that there is a Mac Binary header of 128 bytes */
	/* See if image size & 0xfff = 0x080 which indicates extra 128 bytes */
	if((size & 0xfff) == 0x080) {
		printf("Assuming Mac Binary header on %s\n", dsk->name_ptr);
		dsk->image_start += 0x80;
	}
	image_identified = 0;
	if(buf_2img[0] == '2' && buf_2img[1] == 'I' && buf_2img[2] == 'M' &&
			buf_2img[3] == 'G') {
		/* It's a 2IMG disk */
		printf("Image named %s is in 2IMG format\n", dsk->name_ptr);
		image_identified = 1;

		if(buf_2img[12] == 0) {
			printf("2IMG is in DOS 3.3 sector order\n");
			dsk->image_type = DSK_TYPE_DOS33;
		}
		if(buf_2img[19] & 0x80) {
			/* disk is locked */
			printf("2IMG is write protected\n");
			dsk->write_prot = 1;
			dsk->write_through_to_unix = 0;
		}
		if((buf_2img[17] & 1) && (dsk->image_type == DSK_TYPE_DOS33)) {
			dsk->vol_num = buf_2img[16];
			printf("Setting DOS 3.3 vol num to %d\n", dsk->vol_num);
		}
		//	Some 2IMG archives have the size byte reversed
		size = (buf_2img[31] << 24) + (buf_2img[30] << 16) +
				(buf_2img[29] << 8) + buf_2img[28];
		unix_pos = (buf_2img[27] << 24) + (buf_2img[26] << 16) +
				(buf_2img[25] << 8) + buf_2img[24];
		if(size == 0x800c00) {
			//	Byte reversed 0x0c8000
			size = 0x0c8000;
		}
		dsk->image_start = unix_pos;
		dsk->image_size = size;
	}
	exp_size = 800*1024;
	if(dsk->disk_525) {
		exp_size = 140*1024;
	}
	if(!image_identified) {
		/* See if it might be the Mac diskcopy format */
		tmp = (buf_2img[0x40] << 24) + (buf_2img[0x41] << 16) +
				(buf_2img[0x42] << 8) + buf_2img[0x43];
		if((size >= (exp_size + 0x54)) && (tmp == exp_size)) {
			/* It's diskcopy since data size field matches */
			printf("Image named %s is in Mac diskcopy format\n",
								dsk->name_ptr);
			image_identified = 1;
			dsk->image_start += 0x54;
			dsk->image_size = exp_size;
			dsk->image_type = DSK_TYPE_PRODOS;	/* ProDOS */
		}
	}
	if(!image_identified) {
		/* Assume raw image */
		dsk->image_size = size;
		dsk->image_type = DSK_TYPE_PRODOS;
		if(dsk->disk_525) {
			dsk->image_type = DSK_TYPE_DOS33;
			if(name_len >= 4) {
				cmp_o = dsk->name_ptr[name_len-1];
				cmp_p = dsk->name_ptr[name_len-2];
				cmp_dot = dsk->name_ptr[name_len-3];
				if(cmp_dot == '.' &&
					  (cmp_p == 'p' || cmp_p == 'P') &&
					  (cmp_o == 'o' || cmp_o == 'O')) {
					dsk->image_type = DSK_TYPE_PRODOS;
				}

				cmp_b = dsk->name_ptr[name_len-1];
				cmp_i = dsk->name_ptr[name_len-2];
				cmp_n = dsk->name_ptr[name_len-3];
				cmp_dot = dsk->name_ptr[name_len-4];
				if(cmp_dot == '.' &&
					  (cmp_n == 'n' || cmp_n == 'N') &&
					  (cmp_i == 'i' || cmp_i == 'I') &&
					  (cmp_b == 'b' || cmp_b == 'B')) {
					dsk->image_type = DSK_TYPE_NIB;
					dsk->write_prot = 1;
					dsk->write_through_to_unix = 0;
				}
			}
		}
	}

	dsk->disk_dirty = 0;
	dsk->nib_pos = 0;
	dsk->trks = 0;

	if(dsk->smartport) {
		g_highest_smartport_unit = MAX(dsk->drive,
						g_highest_smartport_unit);

		if(partition_name != 0 || part_num >= 0) {
			ret = cfg_partition_find_by_name_or_num(dsk->file,
						partition_name, part_num, dsk);
			printf("partition %s (num %d) mounted, wr_prot: %d\n",
				partition_name, part_num, dsk->write_prot);

			if(ret < 0) {
				fclose(dsk->file);
				dsk->file = 0;
				return;
			}
		}
		iwm_printf("adding smartport device[%d], size:%08x, "
			"img_sz:%08x\n", dsk->drive, dsk->trks[0].unix_len,
			dsk->image_size);
	} else if(dsk->disk_525) {
		unix_pos = dsk->image_start;
		size = dsk->image_size;
		disk_set_num_tracks(dsk, 4*35);
		len = 0x1000;
		nibs = NIB_LEN_525;
		if(dsk->image_type == DSK_TYPE_NIB) {
			len = dsk->image_size / 35;;
			nibs = len;
		}
		if(size != 35*len) {
			fatal_printf("Disk 5.25 error: size is %d, not 140K.  "
				"Will try to mount anyway\n", size, 35*len);
		}
		for(i = 0; i < 35; i++) {
			iwm_move_to_track(dsk, 4*i);
			disk_unix_to_nib(dsk, 4*i, unix_pos, len, nibs);
			unix_pos += len;
		}
	} else {
		/* disk_35 */
		unix_pos = dsk->image_start;
		size = dsk->image_size;
		if(size != 800*1024) {
			fatal_printf("Disk 3.5 error: size is %d, not 800K.  "
				"Will try to mount anyway\n", size);
		}
		disk_set_num_tracks(dsk, 2*80);
		for(i = 0; i < 2*80; i++) {
			iwm_move_to_track(dsk, i);
			len = g_track_bytes_35[i >> 5];
			nibs = g_track_nibs_35[i >> 5];
			iwm_printf("Trk: %d.%d = unix: %08x, %04x, %04x\n",
				i>>1, i & 1, unix_pos, len, nibs);
			disk_unix_to_nib(dsk, i, unix_pos, len, nibs);
			unix_pos += len;

			iwm_printf(" trk_len:%05x\n", dsk->trks[i].track_len);
		}
	}

	iwm_move_to_track(dsk, save_track);

}

void
eject_named_disk(Disk *dsk, const char *name, const char *partition_name)
{

	if(!dsk->file) {
		return;
	}

	/* If name matches, eject the disk! */
	if(!strcmp(dsk->name_ptr, name)) {
		/* It matches, eject it */
		if((partition_name != 0) && (dsk->partition_name != 0)) {
			/* If both have partitions, and they differ, then */
			/*  don't eject.  Otherwise, eject */
			if(strcmp(dsk->partition_name, partition_name) != 0) {
				/* Don't eject */
				return;
			}
		}
		eject_disk(dsk);
	}
}

void
eject_disk_by_num(int slot, int drive)
{
	Disk	*dsk;

	dsk = cfg_get_dsk_from_slot_drive(slot, drive);

	eject_disk(dsk);
}

void
eject_disk(Disk *dsk)
{
	int	motor_on;
	int	i;

	if(!dsk->file) {
		return;
	}

	g_config_gsport_update_needed = 1;

	motor_on = iwm.motor_on;
	if(g_c031_disk35 & 0x40) {
		motor_on = iwm.motor_on35;
	}
	if(motor_on) {
		halt_printf("Try eject dsk:%s, but motor_on!\n", dsk->name_ptr);
	}

	iwm_flush_disk_to_unix(dsk);

	printf("Ejecting disk: %s\n", dsk->name_ptr);

	/* Free all memory, close file */

	/* free the tracks first */
	if(dsk->trks != 0) {
		for(i = 0; i < dsk->num_tracks; i++) {
			if(dsk->trks[i].nib_area) {
				free(dsk->trks[i].nib_area);
			}
			dsk->trks[i].nib_area = 0;
			dsk->trks[i].track_len = 0;
		}
		free(dsk->trks);
	}
	dsk->num_tracks = 0;
	dsk->trks = 0;

	/* close file, clean up dsk struct */
	fclose(dsk->file);

	dsk->image_start = 0;
	dsk->image_size = 0;
	dsk->nib_pos = 0;
	dsk->disk_dirty = 0;
	dsk->write_through_to_unix = 0;
	dsk->write_prot = 1;
	dsk->file = 0;
	dsk->just_ejected = 1;

	/* Leave name_ptr valid */
}

int
cfg_get_fd_size(char *filename)
{
	struct stat stat_buf;
	int	ret;

	ret = stat(filename, &stat_buf);
	if(ret != 0) {
		fprintf(stderr,"stat %s returned errno: %d\n",
			filename, errno);
		stat_buf.st_size = 0;
	}

	return stat_buf.st_size;
}

int
cfg_partition_read_block(FILE *file, void *buf, int blk, int blk_size)
{
	int	ret;

	ret = fseek(file, blk * blk_size, SEEK_SET);
	if(ret != 0) {
		printf("fseek: wanted: %08x, errno: %d\n",
			blk * blk_size, errno);
		return 0;
	}

	ret = fread((char *)buf, 1, blk_size, file);
	if(ret != blk_size) {
		printf("ret: %08x, wanted %08x, errno: %d\n", ret, blk_size,
			errno);
		return 0;
	}
	return ret;
}

int
cfg_partition_find_by_name_or_num(FILE *file, const char *partnamestr, int part_num,
		Disk *dsk)
{
	Cfg_dirent *direntptr;
	int	match;
	int	num_parts;
	int	i;

	num_parts = cfg_partition_make_list(dsk->name_ptr, file);

	if(num_parts <= 0) {
		return -1;
	}

	for(i = 0; i < g_cfg_partitionlist.last; i++) {
		direntptr = &(g_cfg_partitionlist.direntptr[i]);
		match = 0;
		if((strncmp(partnamestr, direntptr->name, 32) == 0) &&
							(part_num < 0)) {
			//printf("partition, match1, name:%s %s, part_num:%d\n",
			//	partnamestr, direntptr->name, part_num);

			match = 1;
		}
		if((partnamestr == 0) && (direntptr->part_num == part_num)) {
			//printf("partition, match2, n:%s, part_num:%d == %d\n",
			//	direntptr->name, direntptr->part_num, part_num);
			match = 1;
		}
		if(match) {
			dsk->image_start = direntptr->image_start;
			dsk->image_size = direntptr->size;
			//printf("match with image_start: %08x, image_size: "
			//	"%08x\n", dsk->image_start, dsk->image_size);

			return i;
		}
	}

	return -1;
}

int
cfg_partition_make_list(char *filename, FILE *file)
{
	Driver_desc *driver_desc_ptr;
	Part_map *part_map_ptr;
	word32	*blk_bufptr;
	word32	start;
	word32	len;
	word32	data_off;
	word32	data_len;
	word32	sig;
	int	size;
	int	image_start, image_size;
	int	is_dir;
	int	block_size;
	int	map_blks;
	int	cur_blk;

	block_size = 512;

	cfg_free_alldirents(&g_cfg_partitionlist);

	blk_bufptr = (word32 *)malloc(MAX_PARTITION_BLK_SIZE);

	cfg_partition_read_block(file, blk_bufptr, 0, block_size);

	driver_desc_ptr = (Driver_desc *)blk_bufptr;
	sig = GET_BE_WORD16(driver_desc_ptr->sig);
	block_size = GET_BE_WORD16(driver_desc_ptr->blk_size);
	if(block_size == 0) {
		block_size = 512;
	}
	if(sig != 0x4552 || block_size < 0x200 ||
				(block_size > MAX_PARTITION_BLK_SIZE)) {
		cfg_printf("Partition error: No driver descriptor map found\n");
		free(blk_bufptr);
		return 0;
	}

	map_blks = 1;
	cur_blk = 0;
	size = cfg_get_fd_size(filename);
	cfg_file_add_dirent(&g_cfg_partitionlist, "None - Whole image",
			is_dir=0, size, 0, -1);

	while(cur_blk < map_blks) {
		cur_blk++;
		cfg_partition_read_block(file, blk_bufptr, cur_blk, block_size);
		part_map_ptr = (Part_map *)blk_bufptr;
		sig = GET_BE_WORD16(part_map_ptr->sig);
		if(cur_blk <= 1) {
			map_blks = MIN(20,
				GET_BE_WORD32(part_map_ptr->map_blk_cnt));
		}
		if(sig != 0x504d) {
			printf("Partition entry %d bad signature:%04x\n",
				cur_blk, sig);
			free(blk_bufptr);
			return g_cfg_partitionlist.last;
		}

		/* found it, check for consistency */
		start = GET_BE_WORD32(part_map_ptr->phys_part_start);
		len = GET_BE_WORD32(part_map_ptr->part_blk_cnt);
		data_off = GET_BE_WORD32(part_map_ptr->data_start);
		data_len = GET_BE_WORD32(part_map_ptr->data_cnt);
		if(data_off + data_len > len) {
			printf("Poorly formed entry\n");
			continue;
		}

		if(data_len < 10 || start < 1) {
			printf("Poorly formed entry %d, datalen:%d, "
				"start:%08x\n", cur_blk, data_len, start);
			continue;
		}

		image_size = data_len * block_size;
		image_start = (start + data_off) * block_size;
		is_dir = 2*(image_size < 800*1024);
#if 0
		printf(" partition add entry %d = %s %d %08x %08x\n",
			cur_blk, part_map_ptr->part_name, is_dir,
			image_size, image_start);
#endif

		cfg_file_add_dirent(&g_cfg_partitionlist,
			part_map_ptr->part_name, is_dir, image_size,
			image_start, cur_blk);
	}

	free(blk_bufptr);
	return g_cfg_partitionlist.last;
}

int
cfg_maybe_insert_disk(int slot, int drive, const char *namestr)
{
	int	num_parts;
	FILE *file;

	file = fopen(namestr, "rb");
	if(!file) {
		fatal_printf("Cannot open disk image: %s\n", namestr);
		return 0;
	}

	num_parts = cfg_partition_make_list((char*)namestr, file);
	fclose(file);

	if(num_parts > 0) {
		printf("Choose a partition\n");
		g_cfg_select_partition = 1;
	} else {
		insert_disk(slot, drive, namestr, 0, 0, 0, -1);
		return 1;
	}
	return 0;
}

int
cfg_stat(char *path, struct stat *sb)
{
	int	removed_slash;
	int	len;
	int	ret;

	removed_slash = 0;
	len = 0;

#ifdef _WIN32
	/* Windows doesn't like to stat paths ending in a /, so remove it */
	len = strlen(path);
	if((len > 1) && (path[len - 1] == '/') ) {
		path[len - 1] = 0;	/* remove the slash */
		removed_slash = 1;
	}
#endif

	ret = stat(path, sb);

#ifdef _WIN32
	/* put the slash back */
	if(removed_slash) {
		path[len - 1] = '/';
	}
#endif

	return ret;
}

void
cfg_htab_vtab(int x, int y)
{
	if(x > 79) {
		x = 0;
	}
	if(y > 23) {
		y = 0;
	}
	g_cfg_curs_x = x;
	g_cfg_curs_y = y;
	g_cfg_curs_inv = 0;
	g_cfg_curs_mousetext = 0;
}

void
cfg_home()
{
	int	i;

	cfg_htab_vtab(0, 0);
	for(i = 0; i < 24; i++) {
		cfg_cleol();
	}
}

void
cfg_cleol()
{
	g_cfg_curs_inv = 0;
	g_cfg_curs_mousetext = 0;
	cfg_putchar(' ');
	while(g_cfg_curs_x != 0) {
		cfg_putchar(' ');
	}
}

void
cfg_putchar(int c)
{
	int	offset;
	int	x, y;

	if(c == '\n') {
		cfg_cleol();
		return;
	}
	if(c == '\b') {
		g_cfg_curs_inv = !g_cfg_curs_inv;
		return;
	}
	if(c == '\t') {
		g_cfg_curs_mousetext = !g_cfg_curs_mousetext;
		return;
	}
	y = g_cfg_curs_y;
	x = g_cfg_curs_x;

	offset = g_screen_index[g_cfg_curs_y];
	if((x & 1) == 0) {
		offset += 0x10000;
	}
	if(g_cfg_curs_inv) {
		if(c >= 0x40 && c < 0x60) {
			c = c & 0x1f;
		}
	} else {
		c = c | 0x80;
	}
	if(g_cfg_curs_mousetext) {
		c = (c & 0x1f) | 0x40;
	}
	set_memory_c(0xe00400 + offset + (x >> 1), c, 0);
	x++;
	if(x >= 80) {
		x = 0;
		y++;
		if(y >= 24) {
			y = 0;
		}
	}
	g_cfg_curs_y = y;
	g_cfg_curs_x = x;
}

void
cfg_printf(const char *fmt, ...)
{
	va_list ap;
	int	c;
	int	i;

	va_start(ap, fmt);
	(void)vsnprintf(g_cfg_printf_buf, CFG_PRINTF_BUFSIZE, fmt, ap);
	va_end(ap);

	for(i = 0; i < CFG_PRINTF_BUFSIZE; i++) {
		c = g_cfg_printf_buf[i];
		if(c == 0) {
			return;
		}
		cfg_putchar(c);
	}
}

void
cfg_print_num(int num, int max_len)
{
	char	buf[64];
	char	buf2[64];
	int	len;
	int	cnt;
	int	c;
	int	i, j;

	/* Prints right-adjusted "num" in field "max_len" wide */
	snprintf(&buf[0], 64, "%d", num);
	len = strlen(buf);
	for(i = 0; i < 64; i++) {
		buf2[i] = ' ';
	}
	j = max_len + 1;
	buf2[j] = 0;
	j--;
	cnt = 0;
	for(i = len - 1; (i >= 0) && (j >= 1); i--) {
		c = buf[i];
		if(c >= '0' && c <= '9') {
			if(cnt >= 3) {
				buf2[j--] = ',';
				cnt = 0;
			}
			cnt++;
		}
		buf2[j--] = c;
	}
	cfg_printf(&buf2[1]);
}

void
cfg_get_disk_name(char *outstr, int maxlen, int type_ext, int with_extras)
{
	Disk	*dsk;
	int	slot, drive;

	slot = type_ext >> 8;
	drive = type_ext & 0xff;
	dsk = cfg_get_dsk_from_slot_drive(slot, drive);

	outstr[0] = 0;
	if(dsk->name_ptr == 0) {
		return;
	}

	config_generate_config_gsport_name(outstr, maxlen, dsk, with_extras);
}

void
cfg_parse_menu(Cfg_menu *menuptr, int menu_pos, int highlight_pos, int change)
{
	char	valbuf[CFG_OPT_MAXSTR];
	char	**str_ptr;
	const char *menustr;
	char	*curstr, *defstr;
	char	*str;
	char	*outstr;
	int	*iptr;
	int	val;
	int	num_opts;
	int	opt_num;
	int	bufpos, outpos;
	int	curval, defval;
	int	type;
	int	type_ext;
	int	opt_get_str;
	int	separator;
	int	len;
	int	c;
	int	i;

	g_cfg_opt_buf[0] = 0;

	num_opts = 0;
	opt_get_str = 0;
	separator = ',';

	menuptr += menu_pos;		/* move forward to entry menu_pos */

	menustr = menuptr->str;
	type = menuptr->cfgtype;
	type_ext = (type >> 4);
	type = type & 0xf;
	len = strlen(menustr) + 1;

	bufpos = 0;
	outstr = &(g_cfg_opt_buf[0]);

	outstr[bufpos++] = ' ';
	outstr[bufpos++] = ' ';
	outstr[bufpos++] = '\t';
	outstr[bufpos++] = '\t';
	outstr[bufpos++] = ' ';
	outstr[bufpos++] = ' ';

	if(menu_pos == highlight_pos) {
		outstr[bufpos++] = '\b';
	}

	opt_get_str = 2;
	i = -1;
	outpos = bufpos;
#if 0
	printf("cfg menu_pos: %d str len: %d\n", menu_pos, len);
#endif
	while(++i < len) {
		c = menustr[i];
		if(c == separator) {
			if(i == 0) {
				continue;
			}
			c = 0;
		}
		outstr[outpos++] = c;
		outstr[outpos] = 0;
		if(outpos >= CFG_OPT_MAXSTR) {
			fprintf(stderr, "CFG_OPT_MAXSTR exceeded\n");
			my_exit(1);
		}
		if(c == 0) {
			if(opt_get_str == 2) {
				outstr = &(valbuf[0]);
				bufpos = outpos - 1;
				opt_get_str = 0;
			} else if(opt_get_str) {
#if 0
				if(menu_pos == highlight_pos) {
					printf("menu_pos %d opt %d = %s=%d\n",
						menu_pos, num_opts,
						g_cfg_opts_strs[num_opts],
						g_cfg_opts_vals[num_opts]);
				}
#endif
				num_opts++;
				outstr = &(valbuf[0]);
				opt_get_str = 0;
				if(num_opts >= CFG_MAX_OPTS) {
					fprintf(stderr, "CFG_MAX_OPTS oflow\n");
					my_exit(1);
				}
			} else {
				if (type == CFGTYPE_INT)
				{
				val = strtoul(valbuf, 0, 0);
				g_cfg_opts_vals[num_opts] = val;
				}

				if (type == CFGTYPE_STR)
				{
				strncpy(&(g_cfg_opts_strvals[num_opts][0]),&(valbuf[0]),CFG_OPT_MAXSTR);
				}
				outstr = &(g_cfg_opts_strs[num_opts][0]);
				opt_get_str = 1;
			}
			outpos = 0;
			outstr[0] = 0;
		}
	}

	if(menu_pos == highlight_pos) {
		g_cfg_opt_buf[bufpos++] = '\b';
	}

	g_cfg_opt_buf[bufpos] = 0;

	// Figure out if we should get a checkmark
	curval = -1;
	defval = -1;
	curstr = 0;
	if(type == CFGTYPE_INT) {
		iptr = (int*)menuptr->ptr;	// OG Added cast
		curval = *iptr;
		iptr = (int*)menuptr->defptr; // OG Added cast
		defval = *iptr;
		if(curval == defval) {
			g_cfg_opt_buf[3] = 'D';	/* checkmark */
			g_cfg_opt_buf[4] = '\t';
		}
	}
	if(type == CFGTYPE_STR) {
		str_ptr = (char **)menuptr->ptr;
		curstr = *str_ptr;
		str_ptr = (char **)menuptr->defptr;
		defstr = *str_ptr;
		if(strcmp(curstr,defstr) == 0) {
			g_cfg_opt_buf[3] = 'D';	/* checkmark */
			g_cfg_opt_buf[4] = '\t';
		}
	}
	if(type == CFGTYPE_FILE) {
		str_ptr = (char **)menuptr->ptr;
		curstr = *str_ptr;
		str_ptr = (char **)menuptr->defptr;
		defstr = *str_ptr;
		if(strcmp(curstr,defstr) == 0) {
			g_cfg_opt_buf[3] = 'D';	/* checkmark */
			g_cfg_opt_buf[4] = '\t';
		}
	}

	// If it's a menu, give it a special menu indicator
	if(type == CFGTYPE_MENU) {
		g_cfg_opt_buf[1] = '\t';
		g_cfg_opt_buf[2] = 'M';		/* return-like symbol */
		g_cfg_opt_buf[3] = '\t';
		g_cfg_opt_buf[4] = ' ';
	}

	// Decide what to display on the "right" side
	str = 0;
	opt_num = -1;
	if(type == CFGTYPE_INT || type == CFGTYPE_FILE) {
		g_cfg_opt_buf[bufpos++] = ' ';
		g_cfg_opt_buf[bufpos++] = '=';
		g_cfg_opt_buf[bufpos++] = ' ';
		g_cfg_opt_buf[bufpos] = 0;
		for(i = 0; i < num_opts; i++) {
			if(curval == g_cfg_opts_vals[i]) {
				opt_num = i;
				break;
			}
		}
	}
	if(type == CFGTYPE_STR) {
		g_cfg_opt_buf[bufpos++] = ' ';
		g_cfg_opt_buf[bufpos++] = '=';
		g_cfg_opt_buf[bufpos++] = ' ';
		g_cfg_opt_buf[bufpos] = 0;
		for(i = 0; i < num_opts; i++) {
			if(!strcmp(curstr,g_cfg_opts_strvals[i])) {
				opt_num = i;
				break;
			}
		}
	}

	if(change != 0) {
		if(type == CFGTYPE_INT) {
			if(num_opts > 0) {
				opt_num += change;
				if(opt_num >= num_opts) {
					opt_num = 0;
				}
				if(opt_num < 0) {
					opt_num = num_opts - 1;
				}
				curval = g_cfg_opts_vals[opt_num];
			} else {
				curval += change;
				/* HACK: min_val, max_val testing here */
			}
			iptr = (int *)menuptr->ptr;
			*iptr = curval;
		}
		if(type == CFGTYPE_STR) {
			if(num_opts > 0) {
				opt_num += change;
				if(opt_num >= num_opts) {
					opt_num = 0;
				}
				if(opt_num < 0) {
					opt_num = num_opts - 1;
				}
				curstr = g_cfg_opts_strvals[opt_num];
			} else {
				//curstr += change;
				/* HACK: min_val, max_val testing here */
			}
			str_ptr = (char **)menuptr->ptr;
			*str_ptr = curstr;
		}
		g_config_gsport_update_needed = 1;
	}

#if 0
	if(menu_pos == highlight_pos) {
		printf("menu_pos %d opt_num %d\n", menu_pos, opt_num);
	}
#endif

	if(opt_num >= 0) {
		str = &(g_cfg_opts_strs[opt_num][0]);
	} else {
		if(type == CFGTYPE_INT) {
			str = &(g_cfg_opts_strs[0][0]);
			snprintf(str, CFG_OPT_MAXSTR, "%d", curval);
		} else if (type == CFGTYPE_STR) {
			str = &(g_cfg_opts_strs[0][0]);
			printf("curstr is: %s str is: %s\n", curstr,str);
			snprintf(str, CFG_OPT_MAXSTR, "%s", curstr);
		} else if (type == CFGTYPE_DISK) {
			str = &(g_cfg_opts_strs[0][0]),
			cfg_get_disk_name(str, CFG_OPT_MAXSTR, type_ext, 1);
			str = cfg_shorten_filename(str, 68);
		} else if (type == CFGTYPE_FILE) {
			str = &(g_cfg_opts_strs[0][0]);
			snprintf(str, CFG_OPT_MAXSTR, "%s", curstr);
			str = cfg_shorten_filename(str, 68);
		} else {
			str = "";
		}
	}

#if 0
	if(menu_pos == highlight_pos) {
		printf("menu_pos %d buf_pos %d, str is %s, %02x, %02x, "
			"%02x %02x\n",
			menu_pos, bufpos, str, g_cfg_opt_buf[bufpos-1],
			g_cfg_opt_buf[bufpos-2],
			g_cfg_opt_buf[bufpos-3],
			g_cfg_opt_buf[bufpos-4]);
	}
#endif

	g_cfg_opt_buf[bufpos] = 0;
	strncpy(&(g_cfg_opt_buf[bufpos]), str, CFG_OPT_MAXSTR - bufpos - 1);
	g_cfg_opt_buf[CFG_OPT_MAXSTR-1] = 0;
}

void
cfg_get_base_path(char *pathptr, const char *inptr, int go_up)
{
	const char *tmpptr;
	char	*slashptr;
	char	*outptr;
	int	add_dotdot, is_dotdot;
	int	len;
	int	c;

	/* Take full filename, copy it to pathptr, and truncate at last slash */
	/* inptr and pathptr can be the same */
	/* if go_up is set, then replace a blank dir with ".." */
	/* but first, see if path is currently just ../ over and over */
	/* if so, just tack .. onto the end and return */
	//printf("cfg_get_base start with %s\n", inptr);

	g_cfg_file_match[0] = 0;
	tmpptr = inptr;
	is_dotdot = 1;
	while(1) {
		if(tmpptr[0] == 0) {
			break;
		}
		if(tmpptr[0] == '.' && tmpptr[1] == '.' && tmpptr[2] == '/') {
			tmpptr += 3;
		} else {
			is_dotdot = 0;
			break;
		}
	}
	slashptr = 0;
	outptr = pathptr;
	c = -1;
	while(c != 0) {
		c = *inptr++;
		if(c == '/') {
			if(*inptr != 0) {	/* if not a trailing slash... */
				slashptr = outptr;
			}
		}
		*outptr++ = c;
	}
	if(!go_up) {
		/* if not go_up, copy chopped part to g_cfg_file_match*/
		/* copy from slashptr+1 to end */
		tmpptr = slashptr+1;
		if(slashptr == 0) {
			tmpptr = pathptr;
		}
		strncpy(&g_cfg_file_match[0], tmpptr, CFG_PATH_MAX);
		/* remove trailing / from g_cfg_file_match */
		len = strlen(&g_cfg_file_match[0]);
		if((len > 1) && (len < (CFG_PATH_MAX - 1)) &&
					g_cfg_file_match[len - 1] == '/') {
			g_cfg_file_match[len - 1] = 0;
		}
		//printf("set g_cfg_file_match to %s\n", &g_cfg_file_match[0]);
	}
	if(!is_dotdot && (slashptr != 0)) {
		slashptr[0] = '/';
		slashptr[1] = 0;
		outptr = slashptr + 2;
	}
	add_dotdot = 0;
	if(pathptr[0] == 0 || is_dotdot) {
		/* path was blank, or consisted of just ../ pattern */
		if(go_up) {
			add_dotdot = 1;
		}
	} else if(slashptr == 0) {
		/* no slashes found, but path was not blank--make it blank */
		if(pathptr[0] == '/') {
			pathptr[1] = 0;
		} else {
			pathptr[0] = 0;
		}
	}

	if(add_dotdot) {
		--outptr;
		outptr[0] = '.';
		outptr[1] = '.';
		outptr[2] = '/';
		outptr[3] = 0;
	}

	//printf("cfg_get_base end with %s, is_dotdot:%d, add_dotdot:%d\n",
	//		pathptr, is_dotdot, add_dotdot);
}

void
cfg_file_init()
{
	int	slot, drive;
	int	i;

	if(g_cfg_slotdrive < 0xfff) {
		cfg_get_disk_name(&g_cfg_tmp_path[0], CFG_PATH_MAX,
							g_cfg_slotdrive, 0);

		slot = g_cfg_slotdrive >> 8;
		drive = g_cfg_slotdrive & 1;
		for(i = 0; i < 6; i++) {
			if(g_cfg_tmp_path[0] != 0) {
				break;
			}
			/* try to get a starting path from some mounted drive */
			drive = !drive;
			if(i & 1) {
				slot++;
				if(slot >= 8) {
					slot = 5;
				}
			}
			cfg_get_disk_name(&g_cfg_tmp_path[0], CFG_PATH_MAX,
							(slot << 8) + drive, 0);
		}
	} else {
		// Just use g_cfg_file_def_name
		strncpy(&g_cfg_tmp_path[0], g_cfg_file_def_name, CFG_PATH_MAX);
	}
	
	cfg_get_base_path(&g_cfg_file_curpath[0], &g_cfg_tmp_path[0], 0);
	g_cfg_dirlist.invalid = 1;
}

void
cfg_free_alldirents(Cfg_listhdr *listhdrptr)
{
	int	i;

	if(listhdrptr->max > 0) {
		// Free the old directory listing
		for(i = 0; i < listhdrptr->last; i++) {
			free(listhdrptr->direntptr[i].name);
		}
		free(listhdrptr->direntptr);
	}

	listhdrptr->direntptr = 0;
	listhdrptr->last = 0;
	listhdrptr->max = 0;
	listhdrptr->invalid = 0;

	listhdrptr->topent = 0;
	listhdrptr->curent = 0;
}


void
cfg_file_add_dirent(Cfg_listhdr *listhdrptr, const char *nameptr, int is_dir,
		int size, int image_start, int part_num)
{
	Cfg_dirent *direntptr;
	char	*ptr;
	int	inc_amt;
	int	namelen;

	namelen = strlen(nameptr);
	if(listhdrptr->last >= listhdrptr->max) {
		// realloc
		inc_amt = MAX(64, listhdrptr->max);
		inc_amt = MIN(inc_amt, 1024);
		listhdrptr->max += inc_amt;
		listhdrptr->direntptr = (Cfg_dirent*)realloc(listhdrptr->direntptr,
					listhdrptr->max * sizeof(Cfg_dirent));
	}
	ptr = (char*)malloc(namelen+1+is_dir); // OG Added cast
	strncpy(ptr, nameptr, namelen+1);
	if(is_dir) {
		strcat(ptr, "/");
	}
#if 0
	printf("...file entry %d is %s\n", g_cfg_dirlist.last, ptr);
#endif
	direntptr = &(listhdrptr->direntptr[listhdrptr->last]);
	direntptr->name = ptr;
	direntptr->is_dir = is_dir;
	direntptr->size = size;
	direntptr->image_start = image_start;
	direntptr->part_num = part_num;
	listhdrptr->last++;
}

int
cfg_dirent_sortfn(const void *obj1, const void *obj2)
{
	const Cfg_dirent *direntptr1, *direntptr2;
	int	ret;

	/* Called by qsort to sort directory listings */
	direntptr1 = (const Cfg_dirent *)obj1;
	direntptr2 = (const Cfg_dirent *)obj2;
#if defined(MAC) || defined(_WIN32)
	// OG
	ret = 0;
//	ret = strcasecmp(direntptr1->name, direntptr2->name); 
#else
	ret = strcmp(direntptr1->name, direntptr2->name);
#endif
	return ret;
}

int
cfg_str_match(const char *str1, const char *str2, int len)
{
	const byte *bptr1, *bptr2;
	int	c, c2;
	int	i;

	/* basically, work like strcmp, except if str1 ends first, return 0 */

	bptr1 = (const byte *)str1;
	bptr2 = (const byte *)str2;
	for(i = 0; i < len; i++) {
		c = *bptr1++;
		c2 = *bptr2++;
		if(c == 0) {
			if(i > 0) {
				return 0;
			} else {
				return c - c2;
			}
		}
		if(c != c2) {
			return c - c2;
		}
	}

	return 0;
}

void
cfg_file_readdir(const char *pathptr)
{
#ifndef __OS2__
	struct dirent *direntptr;
	struct stat stat_buf;
	DIR	*dirptr;
	mode_t	fmt;
	char	*str;
	const char *tmppathptr;
	int	size;
	int	ret;
	int	is_dir, is_gz;
	int	len;
	int	i;

	if(!strncmp(pathptr, &g_cfg_file_cachedpath[0], CFG_PATH_MAX) &&
			(g_cfg_dirlist.last > 0) && (g_cfg_dirlist.invalid==0)){
		return;
	}
	// No match, must read new directory

	// Free all dirents that were cached previously
	cfg_free_alldirents(&g_cfg_dirlist);

	strncpy(&g_cfg_file_cachedpath[0], pathptr, CFG_PATH_MAX);
	strncpy(&g_cfg_file_cachedreal[0], pathptr, CFG_PATH_MAX);

	str = &g_cfg_file_cachedreal[0];

	for(i = 0; i < 200; i++) {
		len = strlen(str);
		if(len <= 0) {
			break;
		} else if(len < CFG_PATH_MAX-2) {
			if(str[len-1] != '/') {
				// append / to make various routines work
				str[len] = '/';
				str[len+1] = 0;
			}
		}
		ret = cfg_stat(str, &stat_buf);
		is_dir = 0;
		if(ret == 0) {
			fmt = stat_buf.st_mode & S_IFMT;
			if(fmt == S_IFDIR) {
				/* it's a directory */
				is_dir = 1;
			}
		}
		if(is_dir) {
			break;
		} else {
			// user is entering more path, use base for display
			cfg_get_base_path(str, str, 0);
		}
	}
	tmppathptr = str;
	if(str[0] == 0) {
		tmppathptr = ".";
	}
	cfg_file_add_dirent(&g_cfg_dirlist, "..", 1, 0, -1, -1);
	dirptr = opendir(tmppathptr);
	if(dirptr == 0) {
		printf("Could not open %s as a directory\n", tmppathptr);
		return;
	}
	while(1) {
		direntptr = readdir(dirptr);
		if(direntptr == 0) {
			break;
		}
		if(!strcmp(".", direntptr->d_name)) {
			continue;
		}
		if(!strcmp("..", direntptr->d_name)) {
			continue;
		}
		/* Else, see if it is a directory or a file */
		snprintf(&g_cfg_tmp_path[0], CFG_PATH_MAX, "%s%s",
				&g_cfg_file_cachedreal[0], direntptr->d_name);
		ret = cfg_stat(&g_cfg_tmp_path[0], &stat_buf);
		len = strlen(g_cfg_tmp_path);
		is_dir = 0;
		is_gz = 0;
		if((len > 3) && (strcmp(&g_cfg_tmp_path[len - 3], ".gz") == 0)){
			is_gz = 1;
		}
		if(ret != 0) {
			printf("stat %s ret %d, errno:%d\n", &g_cfg_tmp_path[0],
						ret, errno);
			stat_buf.st_size = 0;
			continue;	/* skip it */
		} else {
			fmt = stat_buf.st_mode & S_IFMT;
			size = stat_buf.st_size;
			if(fmt == S_IFDIR) {
				/* it's a directory */
				is_dir = 1;
			} else if((fmt == S_IFREG) && (is_gz == 0)) {
				if(g_cfg_slotdrive < 0xfff) {
					if(size < 140*1024) {
						continue;	/* skip it */
					}
				} else {
					/* see if there are size limits */
					if((size < g_cfg_file_min_size) ||
						(size > g_cfg_file_max_size)) {
						continue;	/* skip it */
					}
				}
			}
		}
		cfg_file_add_dirent(&g_cfg_dirlist, direntptr->d_name, is_dir,
					stat_buf.st_size, -1, -1);
	}
	/* then sort the results (Mac's HFS+ is sorted, but other FS won't be)*/
	qsort(&(g_cfg_dirlist.direntptr[0]), g_cfg_dirlist.last,
					sizeof(Cfg_dirent), cfg_dirent_sortfn);
	g_cfg_dirlist.curent = g_cfg_dirlist.last - 1;
	for(i = g_cfg_dirlist.last - 1; i >= 0; i--) {
		ret = cfg_str_match(&g_cfg_file_match[0],
				g_cfg_dirlist.direntptr[i].name, CFG_PATH_MAX);
		if(ret <= 0) {
			/* set cur ent to closest filename to the match name */
			g_cfg_dirlist.curent = i;
		}
	}
#endif
}

void
cfg_inspect_maybe_insert_file(char *filename, int should_boot)
{
/*
Take a look at a file.  Based on its size, guess a slot/drive to insert it into.
Used for blind operations like dragging/dropping files.
Optionally boot from that slot. 
*/
	int rc = 0;
	int slot = 0;
	rc = cfg_guess_image_size(filename);
	switch (rc)
	{
		case 0:	slot = 7; break;
		case 1:	slot = 6; break;
		case 2: slot = 5; break;
		case 3: slot = 7; break;
		default: break;
	}
	if (slot > 0)
	{
		insert_disk(slot,0,filename,0,0,0,-1);	
		printf("Inserted disk in slot %d, drive 1.  Filename: %s\n", slot, filename);
		if (should_boot) {
			g_temp_boot_slot = slot;
			printf("That slot has been set to boot.\n");
		}
	}
	else
		printf("Unable to determine appropriate place to insert file %s.\n",filename);
}

int
cfg_guess_image_size(char *filename)
{
/*
Guess the image size.  Return values:
 -1 : invalid/unknown.  Can't guess.
  0 : Less than 140k; might be ram disk image.
  1 : 140k, 5.25" image.
  2 : 800k, 3.5" image.
  3 : Something bigger.
*/
	struct stat stat_buf;
	int	rc = -1;
	int	len;
	rc = stat(filename, &stat_buf);
	if(rc < 0)
	{
		printf("Can't get statistics on file %s; errno: %d\n",
			filename, errno);
		rc = -1;
	} else {
		len = stat_buf.st_size;
		printf("Found file %s, size %d; guessing ",
			filename, len);
		if (len <  140 * 1024) {
			/* Not enough for a 140k image */
			printf("small ProDOS image.\n");
			rc = 0;
		} else if (len <  140 * 1024 + 256 + 1) {
			/* Reasonable size for 140k image, maybe in 2mg format */
			printf("a 5-1/4\" image.\n");
			rc = 1;
		} else if (len < 800 * 1024 + 256 + 1) {
			/* Reasonable size for 800k image, maybe in 2mg format */
			printf("a 3-1/2\" image.\n");
			rc = 2;
		} else {
			/* Let's pretend it's an HDV image */
			printf("a hard drive image.\n");
			rc = 3;
		}
	}
	return rc;
}

char *
cfg_shorten_filename(const char *in_ptr, int maxlen)
{
	char	*out_ptr;
	int	len;
	int	c;
	int	i;
	/* Warning: uses a static string, not reentrant! */
	out_ptr = &(g_cfg_file_shortened[0]);
	len = strlen(in_ptr);
	maxlen = MIN(len, maxlen);
	for(i = 0; i < maxlen; i++) {
		c = in_ptr[i] & 0x7f;
		if(c < 0x20) {
			c = '*';
		}
		out_ptr[i] = c;
	}
	out_ptr[maxlen] = 0;
	if(len > maxlen) {
		for(i = 0; i < (maxlen/2); i++) {
			c = in_ptr[len-i-1] & 0x7f;
			if(c < 0x20) {
				c = '*';
			}
			out_ptr[maxlen-i-1] = c;
		}
		out_ptr[(maxlen/2) - 1] = '.';
		out_ptr[maxlen/2] = '.';
		out_ptr[(maxlen/2) + 1] = '.';
	}
	return out_ptr;
}
void
cfg_fix_topent(Cfg_listhdr *listhdrptr)
{
	int	num_to_show;
	num_to_show = listhdrptr->num_to_show;
	/* Force curent and topent to make sense */
	if(listhdrptr->curent >= listhdrptr->last) {
		listhdrptr->curent = listhdrptr->last - 1;
	}
	if(listhdrptr->curent < 0) {
		listhdrptr->curent = 0;
	}
	if(abs(listhdrptr->curent - listhdrptr->topent) >= num_to_show) {
		listhdrptr->topent = listhdrptr->curent - (num_to_show/2);
	}
	if(listhdrptr->topent > listhdrptr->curent) {
		listhdrptr->topent = listhdrptr->curent - (num_to_show/2);
	}
	if(listhdrptr->topent < 0) {
		listhdrptr->topent = 0;
	}
}
void
cfg_file_draw()
{
	Cfg_listhdr *listhdrptr;
	Cfg_dirent *direntptr;
	char	*str, *fmt;
	int	num_to_show;
	int	yoffset;
	int	x, y;
	int	i;
	cfg_file_readdir(&g_cfg_file_curpath[0]);
	for(y = 0; y < 21; y++) {
		cfg_htab_vtab(0, y);
		cfg_printf("\tZ\t");
		for(x = 1; x < 79; x++) {
			cfg_htab_vtab(x, y);
			cfg_putchar(' ');
		}
		cfg_htab_vtab(79, y);
		cfg_printf("\t_\t");
	}
	cfg_htab_vtab(1, 0);
	cfg_putchar('\b');
	for(x = 1; x < 79; x++) {
		cfg_putchar(' ');
	}
	if(g_cfg_slotdrive < 0xfff) {
		cfg_htab_vtab(30, 0);
		cfg_printf("\bSelect image for s%dd%d\b",
			(g_cfg_slotdrive >> 8), (g_cfg_slotdrive & 0xff) + 1);
	} else {
		cfg_htab_vtab(5, 0);
		cfg_printf("\bSelect file to use as %-40s\b",
			cfg_shorten_filename(g_cfg_file_def_name, 40));
	}
	cfg_htab_vtab(2, 1);
	cfg_printf("Configuration file path: %-56s",
			cfg_shorten_filename(&g_config_gsport_name[0], 56));
	cfg_htab_vtab(2, 2);
	cfg_printf("Current directory: %-50s",
			cfg_shorten_filename(&g_cfg_cwd_str[0], 50));
	cfg_htab_vtab(2, 3);
	str = "";
	if(g_cfg_file_pathfield) {
		str = "\b \b";
	}
	cfg_printf("Path: %s%s",
			cfg_shorten_filename(&g_cfg_file_curpath[0], 68), str);
	cfg_htab_vtab(0, 4);
	cfg_printf(" \t");
	for(x = 1; x < 79; x++) {
		cfg_putchar('\\');
	}
	cfg_printf("\t ");

	/* Force curent and topent to make sense */
	listhdrptr = &g_cfg_dirlist;
	num_to_show = CFG_NUM_SHOWENTS;
	yoffset = 5;
	if(g_cfg_select_partition > 0) {
		listhdrptr = &g_cfg_partitionlist;
		num_to_show -= 2;
		cfg_htab_vtab(2, yoffset);
		cfg_printf("Select partition of %-50s\n",
			cfg_shorten_filename(&g_cfg_file_path[0], 50), str);
		yoffset += 2;
	}
	listhdrptr->num_to_show = num_to_show;
	cfg_fix_topent(listhdrptr);
	for(i = 0; i < num_to_show; i++) {
		y = i + yoffset;
		if(listhdrptr->last > (i + listhdrptr->topent)) {
			direntptr = &(listhdrptr->
					direntptr[i + listhdrptr->topent]);
			cfg_htab_vtab(2, y);
			if(direntptr->is_dir) {
				cfg_printf("\tXY\t ");
			} else {
				cfg_printf("   ");
			}
			if(direntptr->part_num >= 0) {
				cfg_printf("%3d: ", direntptr->part_num);
			}
			str = cfg_shorten_filename(direntptr->name, 45);
			fmt = "%-45s";
			if(i + listhdrptr->topent == listhdrptr->curent) {
				if(g_cfg_file_pathfield == 0) {
					fmt = "\b%-45s\b";
				} else {
					fmt = "%-44s\b \b";
				}
			}
			cfg_printf(fmt, str);
			if(!direntptr->is_dir) {
				cfg_print_num(direntptr->size, 13);
			}
		}
	}
	cfg_htab_vtab(1, 5 + CFG_NUM_SHOWENTS);
	cfg_putchar('\t');
	for(x = 1; x < 79; x++) {
		cfg_putchar('L');
	}
	cfg_putchar('\t');
}
void
cfg_partition_selected()
{
	char	*str;
	const char *part_str;
	char	*part_str2;
	int	pos;
	int	part_num;
	pos = g_cfg_partitionlist.curent;
	str = g_cfg_partitionlist.direntptr[pos].name;
	part_num = -2;
	part_str = 0;
	if(str[0] == 0 || (str[0] >= '0' && str[0] <= '9')) {
		part_num = g_cfg_partitionlist.direntptr[pos].part_num;
	} else {
		part_str = str;
	}
	part_str2 = 0;
	if(part_str != 0) {
		part_str2 = (char *)malloc(strlen(part_str)+1);
		strcpy(part_str2, part_str);
	}
	insert_disk(g_cfg_slotdrive >> 8, g_cfg_slotdrive & 0xff,
			&(g_cfg_file_path[0]), 0, 0, part_str2, part_num);
	if(part_str2 != 0) {
		free(part_str2);
	}
	g_cfg_slotdrive = -1;
	g_cfg_select_partition = -1;
}
void
cfg_file_update_ptr(char *str)
{
	char	*newstr;
	int	len;
	len = strlen(str) + 1;
	newstr = (char*)malloc(len);
	memcpy(newstr, str, len);
	if(g_cfg_file_strptr) {
		if(*g_cfg_file_strptr) {
			free(*g_cfg_file_strptr);
		}
	}
	*g_cfg_file_strptr = newstr;
	if(g_cfg_file_strptr == &(g_cfg_rom_path)) {
		printf("Updated ROM file\n");
		load_roms_init_memory();
	}
	g_config_gsport_update_needed = 1;
}
void
cfg_file_selected()
{
#ifndef __OS2__
	struct stat stat_buf;
	char	*str;
	int	fmt;
	int	ret;
	if(g_cfg_select_partition > 0) {
		cfg_partition_selected();
		return;
	}
	if(g_cfg_file_pathfield == 0) {
		// in file section area of window
		str = g_cfg_dirlist.direntptr[g_cfg_dirlist.curent].name;
		if(!strcmp(str, "../")) {
			/* go up one directory */
			cfg_get_base_path(&g_cfg_file_curpath[0],
						&g_cfg_file_curpath[0], 1);
			return;
		}
		snprintf(&g_cfg_file_path[0], CFG_PATH_MAX, "%s%s",
					&g_cfg_file_cachedreal[0], str);
	} else {
		// just use cfg_file_curpath directly
		strncpy(&g_cfg_file_path[0], &g_cfg_file_curpath[0],
							CFG_PATH_MAX);
	}
	ret = cfg_stat(&g_cfg_file_path[0], &stat_buf);
	fmt = stat_buf.st_mode & S_IFMT;
	cfg_printf("Stat'ing %s, st_mode is: %08x\n", &g_cfg_file_path[0],
			(int)stat_buf.st_mode);
	if(ret != 0) {
		printf("stat %s returned %d, errno: %d\n", &g_cfg_file_path[0],
					ret, errno);
	} else {
		if(fmt == S_IFDIR) {
			/* it's a directory */
			strncpy(&g_cfg_file_curpath[0], &g_cfg_file_path[0],
						CFG_PATH_MAX);
		} else {
			/* select it */
			if(g_cfg_slotdrive < 0xfff) {
				ret = cfg_maybe_insert_disk(g_cfg_slotdrive>>8,
					g_cfg_slotdrive & 0xff,
					&g_cfg_file_path[0]);
				if(ret > 0) {
					g_cfg_slotdrive = -1;
				}
			} else {
				cfg_file_update_ptr(&g_cfg_file_path[0]);
				g_cfg_slotdrive = -1;
			}
		}
	}
#endif
}

void
cfg_file_handle_key(int key)
{
	Cfg_listhdr *listhdrptr;
	int	len;
	if(g_cfg_file_pathfield) {
		if(key >= 0x20 && key < 0x7f) {
			len = strlen(&g_cfg_file_curpath[0]);
			if(len < CFG_PATH_MAX-4) {
				g_cfg_file_curpath[len] = key;
				g_cfg_file_curpath[len+1] = 0;
			}
			return;
		}
	}
	listhdrptr = &g_cfg_dirlist;
	if(g_cfg_select_partition > 0) {
		listhdrptr = &g_cfg_partitionlist;
	}
	if( (g_cfg_file_pathfield == 0) &&
		 ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')) ) {
		/* jump to file starting with this letter */
		g_cfg_file_match[0] = key;
		g_cfg_file_match[1] = 0;
		g_cfg_dirlist.invalid = 1;	/* re-read directory */
	}
	switch(key) {
	case 0x1b:
		if(g_cfg_slotdrive < 0xfff) {
			eject_disk_by_num(g_cfg_slotdrive >> 8,
						g_cfg_slotdrive & 0xff);
		}
		g_cfg_slotdrive = -1;
		g_cfg_select_partition = -1;
		g_cfg_dirlist.invalid = 1;
		break;
	case 0x0a:	/* down arrow */
		if(g_cfg_file_pathfield == 0) {
			listhdrptr->curent++;
			cfg_fix_topent(listhdrptr);
		}
		break;
	case 0x0b:	/* up arrow */
		if(g_cfg_file_pathfield == 0) {
			listhdrptr->curent--;
			cfg_fix_topent(listhdrptr);
		}
		break;
	case 0x0d:	/* return */
		printf("handling return press\n");
		cfg_file_selected();
		break;
	case 0x09:	/* tab */
		g_cfg_file_pathfield = !g_cfg_file_pathfield;
		break;
	case 0x08:	/* left arrow */
	case 0x7f:	/* delete key */
		if(g_cfg_file_pathfield) {
			// printf("left arrow/delete\n");
			len = strlen(&g_cfg_file_curpath[0]) - 1;
			if(len >= 0) {
				g_cfg_file_curpath[len] = 0;
			}
		}
		break;
	default:
		printf("key: %02x\n", key);
	}
#if 0
	printf("curent: %d, topent: %d, last: %d\n",
		g_cfg_dirlist.curent, g_cfg_dirlist.topent, g_cfg_dirlist.last);
#endif
}
void
config_control_panel()
{
	void	(*fn_ptr)();
	const char *str;
	Cfg_menu *menuptr;
	void	*ptr;
	int	print_eject_help;
	int	line;
	int	type;
	int	match_found;
	int	menu_line;
	int	menu_inc;
	int	max_line;
	int	key;
	int	i, j;
	// First, save important text screen state
	g_save_cur_a2_stat = g_cur_a2_stat;
	for(i = 0; i < 0x400; i++) {
		g_save_text_screen_bytes[i] = g_slow_memory_ptr[0x400+i];
		g_save_text_screen_bytes[0x400+i] =g_slow_memory_ptr[0x10400+i];
	}
	g_cur_a2_stat = ALL_STAT_TEXT | ALL_STAT_VID80 | ALL_STAT_ANNUNC3 |
			(0xf << BIT_ALL_STAT_TEXT_COLOR) | ALL_STAT_ALTCHARSET;
	g_a2_new_all_stat[0] = g_cur_a2_stat;
	g_new_a2_stat_cur_line = 0;
	cfg_printf("In config_control_panel\n");
	for(i = 0; i < 20; i++) {
		// Toss any queued-up keypresses
		if(adb_read_c000() & 0x80) {
			(void)adb_access_c010();
		}
	}
	g_adb_repeat_vbl = 0;
	g_cfg_vbl_count = 0;
	// HACK: Force adb keyboard (and probably mouse) to "normal"...
	g_full_refresh_needed = -1;
	g_a2_screen_buffer_changed = -1;
	cfg_home();
	j = 0;
	menuptr = g_cfg_main_menu;
	if(g_rom_version < 0) {
		/* Must select ROM file */
		menuptr = g_cfg_rom_menu;
	}
	menu_line = 1;
	menu_inc = 1;
	g_cfg_slotdrive = -1;
	g_cfg_select_partition = -1;
	while(g_config_control_panel & !(halt_sim&HALT_WANTTOQUIT)) {
		if(g_fatal_log > 0) {
			x_show_alert(0, 0);
		}
		cfg_home();
		line = 1;
		max_line = 1;
		match_found = 0;
		print_eject_help = 0;
		cfg_printf("%s\n\n", menuptr[0].str);
		while(line < 24) {
			str = menuptr[line].str;
			type = menuptr[line].cfgtype;
			ptr = menuptr[line].ptr;
			if(str == 0) {
				break;
			}
			if((type & 0xf) == CFGTYPE_DISK) {
				print_eject_help = 1;
			}
			cfg_parse_menu(menuptr, line, menu_line, 0);
			if(line == menu_line) {
				if(type != 0) {
					match_found = 1;
				} else if(menu_inc) {
					menu_line++;
				} else {
					menu_line--;
				}
			}
			if(line > max_line) {
				max_line = line;
			}
			cfg_printf("%s\n", g_cfg_opt_buf);
			line++;
		}
		if((menu_line < 1) && !match_found) {
			menu_line = 1;
		}
		if((menu_line >= max_line) && !match_found) {
			menu_line = max_line;
		}
		if(g_rom_version < 0) {
			cfg_htab_vtab(0, 21);
			cfg_printf("\bYOU MUST SELECT A VALID ROM FILE\b\n");
		}
		cfg_htab_vtab(0, 23);
		cfg_printf("Move: \tJ\t \tK\t Change: \tH\t \tU\t \tM\t");
		if(print_eject_help) {
			cfg_printf("   Eject: ");
			if(g_cfg_slotdrive >= 0) {
				cfg_printf("\bESC\b");
			} else {
				cfg_printf("E");
			}
		}
#if 0
		cfg_htab_vtab(0, 22);
		cfg_printf("menu_line: %d line: %d, vbl:%d, adb:%d key_dn:%d\n",
			menu_line, line, g_cfg_vbl_count, g_adb_repeat_vbl,
			g_key_down);
#endif
		if(g_cfg_slotdrive >= 0) {
			cfg_file_draw();
		}
#ifdef HAVE_TFE
		/*HACK eh, at least I think it is. Display the available ethernet interfaces
		when in the ethernet control panel. This is the only way one can customize a menu pane.
		Kent did it with the directory browser, so why not.*/
		if(menuptr == g_cfg_ethernet_menu)
		{
			cfg_get_tfe_name();
		}
#endif
		/*If user enters the Virtual Imagewriter control panel, flag it so we can 
		automatically apply changes on exit.*/
		if(menuptr == g_cfg_imagewriter_menu)
		{
			g_cfg_triggeriwreset = 1;
		}
		key = -1;
		while(g_config_control_panel & !(halt_sim&HALT_WANTTOQUIT)) {
			video_update();
			key = adb_read_c000();
			if(key & 0x80) {
				key = key & 0x7f;
				(void)adb_access_c010();
				break;
			} else {
				key = -1;
			}
			micro_sleep(1.0/60.0);
			g_cfg_vbl_count++;
			if(!match_found) {
				break;
			}
		}
		if((key >= 0) && (g_cfg_slotdrive < 0)) {
			// Normal menu system
			switch(key) {
			case 0x0a: /* down arrow */
				menu_line++;
				menu_inc = 1;
				break;
			case 0x0b: /* up arrow */
				menu_line--;
				menu_inc = 0;
				if(menu_line < 1) {
					menu_line = 1;
				}
				break;
			case 0x15: /* right arrow */
				cfg_parse_menu(menuptr, menu_line,menu_line,1);
				break;
			case 0x08: /* left arrow */
				cfg_parse_menu(menuptr,menu_line,menu_line,-1);
				break;
			case 0x0d:
				type = menuptr[menu_line].cfgtype;
				ptr = menuptr[menu_line].ptr;
				switch(type & 0xf) {
				case CFGTYPE_MENU:
					menuptr = (Cfg_menu *)ptr;
					menu_line = 1;
					break;
				case CFGTYPE_DISK:
					g_cfg_slotdrive = type >> 4;
					cfg_file_init();
					break;
				case CFGTYPE_FUNC:
					fn_ptr = (void (*)())ptr;
					(*fn_ptr)();
					adb_all_keys_up(); //Needed otherwise menu function will continue to repeat until we move selection up or down
					break;
				case CFGTYPE_FILE:
					g_cfg_slotdrive = 0xfff;
					g_cfg_file_def_name = *((char **)ptr);
					g_cfg_file_strptr = (char **)ptr;
					cfg_file_init();
				}
				break;
			case 0x1b:
				// Jump to last menu entry
				menu_line = max_line;
				break;
			case 'e':
			case 'E':
				type = menuptr[menu_line].cfgtype;
				if((type & 0xf) == CFGTYPE_DISK) {
					eject_disk_by_num(type >> 12,
							(type >> 4) & 0xff);
				}
				break;
			default:
				printf("key: %02x\n", key);
			}
		} else if(key >= 0) {
			cfg_file_handle_key(key);
		}
	}
	for(i = 0; i < 0x400; i++) {
		set_memory_c(0xe00400+i, g_save_text_screen_bytes[i], 0);
		set_memory_c(0xe10400+i, g_save_text_screen_bytes[0x400+i], 0);
	}
	// And quit
	if (g_cfg_triggeriwreset)
	{
		g_cfg_triggeriwreset = 0;
		cfg_iwreset(); //Reset the virtual Imagewriter if the user was in the control panel.
	}
	g_config_control_panel = 0;
	g_adb_repeat_vbl = g_vbl_count + 60;
	g_cur_a2_stat = g_save_cur_a2_stat;
	change_display_mode(g_cur_dcycs);
	g_full_refresh_needed = -1;
	g_a2_screen_buffer_changed = -1;
}

void x_clk_setup_bram_version()
{
		if(g_rom_version < 3) {
		g_bram_ptr = (&g_bram[0][0]);	// ROM 01
	} else {
		g_bram_ptr = (&g_bram[1][0]);	// ROM 03
	}
}
