/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2019 by GSport contributors
 Copyright (C) 2016 - 2018  Dagen Brock
 
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

#define CONF_BUF_LEN				1024
#define COPY_BUF_SIZE				4096
#define CFG_PRINTF_BUFSIZE	2048
#define CFG_PATH_MAX				1024
#define CFG_NUM_SHOWENTS		16

#define CFGTYPE_MENU		1
#define CFGTYPE_INT		  2
#define CFGTYPE_DISK		3
#define CFGTYPE_FUNC		4
#define CFGTYPE_FILE		5
#define CFGTYPE_STR			6
#define CFGTYPE_DIR			7
/* CFGTYPE limited to just 4 bits: 0-15 */

/* Cfg_menu, Cfg_dirent and Cfg_listhdr are defined in defc.h */

STRUCT(Cfg_defval) {
	Cfg_menu *menuptr;
	int	intval;
	char	*strval;
};


int cfg_get_fd_size(char *filename);
int cfg_partition_read_block(FILE *file, void *buf, int blk, int blk_size);
int cfg_partition_find_by_name_or_num(FILE *file, const char *partnamestr, int part_num, Disk *dsk);
int cfg_maybe_insert_disk(int slot, int drive, const char *namestr);
int cfg_stat(char *path, struct stat *sb);
int cfg_partition_make_list(char *filename, FILE *file);
void cfg_htab_vtab(int x, int y);
void cfg_home(void);
void cfg_cleol(void);
void cfg_putchar(int c);
void cfg_printf(const char *fmt, ...);
void cfg_print_num(int num, int max_len);
void cfg_get_disk_name(char *outstr, int maxlen, int type_ext, int with_extras);
void cfg_parse_menu(Cfg_menu *menuptr, int menu_pos, int highlight_pos, int change);
void cfg_get_base_path(char *pathptr, const char *inptr, int go_up);
void cfg_file_init(void);
void cfg_free_alldirents(Cfg_listhdr *listhdrptr);
void cfg_file_add_dirent(Cfg_listhdr *listhdrptr, const char *nameptr, int is_dir, int size, int image_start, int part_num);
int cfg_dirent_sortfn(const void *obj1, const void *obj2);
int cfg_str_match(const char *str1, const char *str2, int len);
void cfg_file_readdir(const char *pathptr);
char *cfg_shorten_filename(const char *in_ptr, int maxlen);
void cfg_fix_topent(Cfg_listhdr *listhdrptr);
void cfg_file_draw(void);
void cfg_partition_selected(void);
void cfg_file_update_ptr(char *str);
void cfg_file_selected(int);
void cfg_file_handle_key(int key);
