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

#include "defc.h"
#include <ctype.h>

#include "prodos.h"

#include <errno.h>


#define DEF_DISK_SIZE	(800*1024)
#define MAX_FILE_NAMES	51

char out_name[] = "POOF1";

int g_def_file_type = -1;
int g_def_aux_type = -1;

void make_legal_prodos_name(char *new_name, char *old_name);

int
main(int argc, char **argv)
{
	char	*files[MAX_FILE_NAMES];
	char	*name;
	char	*new_name_end;
	int	new_name_len;
	struct stat stat_buf;
	int	in;
	int	in_size;
	int	ret;
	ProDisk	*disk;
	char	new_name[128];
	byte	in_buf[1024];
	int	disk_size;
	int	i;
	int	done;
	int	pos;
	int	size;
	int	num_files;
	int	file_type;
	int	aux_type;

	disk_size = DEF_DISK_SIZE;
	if(argc < 2) {
		fprintf(stderr, "%s: {-[size in K]} {unix_files}\n",
			argv[0]);
		exit(1);
	}

	num_files = 0;
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			/* I smell a -size_in_K */
			disk_size = strtoul(&(argv[i][1]), 0, 10) * 1024;
			printf("disk_size: %d, 0x%x\n", disk_size, disk_size);
			if(disk_size < 40*1024) {
				printf("Too small!\n");
				exit(1);
			}
		} else {
			files[num_files] = argv[i];
			num_files++;
			if(num_files >= MAX_FILE_NAMES) {
				printf("Too many filenames: %d\n", num_files);
				exit(2);
			}
		}
	}

	disk = allocate_memdisk(out_name, disk_size);
	format_memdisk(disk, out_name);

	for(i = 0; i < num_files; i++) {
		name = files[i];
		in = open(name, O_RDONLY | O_BINARY);
		if(in < 0) {
			fprintf(stderr, "opening %s returned %d, errno: %d\n",
				name, in, errno);
			exit(1);
		}

		ret = fstat(in, &stat_buf);
		if(ret != 0) {
			fprintf(stderr, "fstat returned %d, errno: %d\n",
				ret, errno);
		}

		in_size = stat_buf.st_size;
		printf("in size: %d\n", in_size);

		if(in_size > disk->disk_bytes_left) {
			printf("File bigger than %d, too big!\n", disk_size);
			exit(2);
		}

		make_legal_prodos_name(new_name, name);

		new_name_len = strlen(new_name);
		new_name_end = new_name + new_name_len;

		file_type = g_def_file_type;
		aux_type = g_def_aux_type;
		while(g_def_file_type < 0) {
			/* choose file type */
			if(new_name_len >= 5) {
				if(strcmp(new_name_end - 4, ".SHK") == 0) {
					file_type = 0xe0;
					aux_type = 0x8002;
					break;
				}
				if(strcmp(new_name_end - 4, ".SDK") == 0) {
					file_type = 0xe0;
					aux_type = 0x8002;
					break;
				}
			}
			file_type = 0x04;	/* TXT */
			aux_type = 0;
			break;
		}

		create_new_file(disk, 2, 1, new_name, file_type,
			0, 0, 0, 0xc3, aux_type, 0, in_size);


		done = 0;
		pos = 0;
		while(pos < in_size) {
			size = 512;
			if(pos + size > in_size) {
				size = in_size - pos;
			}
			ret = read(in, in_buf, size);
			if(ret != size || ret <= 0) {
				fprintf(stderr, "read returned %d, errno: %d\n",
					ret, errno);
				exit(2);
			}
			ret = pro_write_file(disk, in_buf, pos, size);
			if(ret != 0) {
				printf("pro_write returned %d!\n", ret);
				exit(3);
			}
			pos += size;
		}

		close_file(disk);

		close(in);
	}

	flush_disk(disk);
	return 0;
}

void
make_legal_prodos_name(char *new_name, char *old_name)
{
	char	*ptr;
	int	start_len, start_char;
	int	len;
	int	pos;
	int	c;
	int	j;

	for(j = 0; j < 16; j++) {
		/* make sure it ends with null == 15 + 1 */
		new_name[j] = 0;
	}

	start_char = 0;
	start_len = strlen(old_name);
	len = 0;
	ptr = &old_name[start_len - 1];
	for(j = start_len - 1; j >= 0; j--) {
		if(*ptr == '/' || *ptr == ':') {
			break;
		}
		ptr--;
		len++;
	}
	ptr++;

	if(len <= 0) {
		printf("Filename: %s has len: %d!\n", old_name, len);
		exit(1);
	}

	printf("mid_name: %s, len:%d\n", ptr, len);

	pos = 0;
	for(j = 0; j < 15; j++) {
		c = ptr[pos];
		if(isalnum(c)) {
			c = toupper(c);
		} else if(c != 0) {
			c = '.';
		}

		if(j == 0 && !isalpha(c)) {
			c = 'A';
		}

		new_name[j] = c;

		pos++;
		if(pos == 7 && len > 15) {
			pos = len - 8;
		}
	}

	printf("new_name: %s\n", new_name);
}

void
flush_disk(ProDisk *disk)
{
	disk_write_data(disk, 6, disk->bitmap_ptr,
					disk->size_bitmap_blocks * 512);
	close(disk->fd);
	disk->fd = -1;
}

void
close_file(ProDisk *disk)
{
	write_ind_block(disk);
	write_master_ind_block(disk);
	disk_write_data(disk, disk->dir_blk_num, &(disk->dir_blk_data[0]), 512);
	disk->file_ptr = 0;
}

ProDisk *
allocate_memdisk(char *out_name, int size)
{
	ProDisk	*disk;
	int	out;

	out = open(out_name, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0x1b6);
	if(out < 0) {
		fprintf(stderr, "opening %s returned %d, errno: %d\n",
			out_name, out, errno);
		exit(1);
	}

	disk = (ProDisk *)malloc(sizeof(ProDisk));
	if(disk == 0) {
		printf("allocate_memdisk failed, errno: %d\n", errno);
	}

	disk->fd = out;
	disk->total_blocks = (size + 511) / 512;
	disk->bitmap_ptr = 0;
	disk->disk_bytes_left = 0;
	disk->size_bitmap_blocks = 0;
	disk->dir_blk_num = -1;
	disk->ind_blk_num = -1;
	disk->master_ind_blk_num = -1;

	return disk;
}

void
format_memdisk(ProDisk *disk, char *name)
{
	byte	zero_buf[1024];
	int	total_blocks;
	byte	*bitmap_ptr;
	Vol_hdr	*vol_hdr;
	Directory *dir;
	int	size_bitmap_bytes;
	int	size_bitmap_blocks;
	int	disk_blocks_left;
	int	i, j;

	total_blocks = disk->total_blocks;

	/* Zero out blocks 0 and 1 */
	for(i = 0; i < 2*512; i++) {
		zero_buf[i] = 0;
	}
	disk_write_data(disk, 0x00000, zero_buf, 2*512);

	/* and make the image the right size */
	disk_write_data(disk, total_blocks - 1, zero_buf, 512);

	dir = disk_read_dir(disk, 2);
	set_l2byte(&(dir->prev_blk), 0);
	set_l2byte(&(dir->next_blk), 3);
	vol_hdr = (Vol_hdr *)&(dir->file_entries[0]);
	vol_hdr->storage_type_name_len = 0xf0 + strlen(name);
	strncpy((char *)vol_hdr->vol_name, name, strlen(name));
	vol_hdr->version = 0;
	vol_hdr->min_version = 0;
	vol_hdr->access = 0xc3;
	vol_hdr->entry_length = 0x27;
	vol_hdr->entries_per_block = 0x0d;
	set_l2byte(&(vol_hdr->file_count), 0);
	vol_hdr->entries_per_block = 0x0d;
	set_l2byte(&(vol_hdr->bit_map), 6);
	set_l2byte(&(vol_hdr->total_blocks), total_blocks);
	for(i = 1; i < 13; i++) {
		set_file_entry(&(dir->file_entries[i]), 0, "", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0);
	}
	disk_write_dir(disk, 2);


	for(i = 3; i < 6; i++) {
		dir = disk_read_dir(disk, i);
		set_l2byte(&(dir->prev_blk), i - 1);
		set_l2byte(&(dir->next_blk), i + 1);
		if(i == 5) {
			set_l2byte(&(dir->next_blk), 0);
		}
		for(j = 0; j < 13; j++) {
			set_file_entry(&(dir->file_entries[j]), 0, "", 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0);
		}

		disk_write_dir(disk, i);
	}

	size_bitmap_bytes = (total_blocks + 7)/ 8;
	size_bitmap_blocks = (size_bitmap_bytes + 511)/ 512;
	bitmap_ptr = (byte *)malloc(size_bitmap_blocks * 512);
	for(i = 0; i < 6+size_bitmap_blocks; i++) {
		set_bitmap_used(bitmap_ptr, i);
	}
	for(i = 6+size_bitmap_blocks; i < total_blocks; i++) {
		set_bitmap_free(bitmap_ptr, i);
	}
	for(i = total_blocks; i < size_bitmap_blocks*512*8; i++) {
		set_bitmap_used(bitmap_ptr, i);
	}

	disk_write_data(disk, 6, bitmap_ptr, size_bitmap_blocks * 512);
	disk->bitmap_ptr = bitmap_ptr;
	disk->size_bitmap_blocks = size_bitmap_blocks;
	disk->bitmap_bytes = size_bitmap_blocks * 512;
	disk->bitmap_cur_pos = 0;

	disk_blocks_left = total_blocks - 6 - size_bitmap_blocks;
	disk->disk_bytes_left = disk_blocks_left * 512;
}

void
disk_write_data(ProDisk *disk, int blk_num, byte *buf, int size)
{
	int	size_in_blocks;
	int	ret;

#if 0
	printf("Writing blk %04x from buf: %08x, %03x\n", blk_num, buf, size);
#endif

	size_in_blocks = size >> 9;
	if(size_in_blocks * 512 != size) {
		printf("disk_write: blk: %04x, buf: %08x, size: %08x\n",
			blk_num, (word32)buf, size);
		exit(1);
	}

	ret = lseek(disk->fd, 512*blk_num, SEEK_SET);
	if(ret != 512*blk_num) {
		printf("disk_write: seek: %d, errno: %d, blk: %04x, buf: "
			"%08x, sz: %08x\n", ret, errno, blk_num,
			(word32)buf, size);
		exit(1);
	}

	ret = write(disk->fd, buf, size);
	if(ret != size) {
		printf("disk_write: write: %d, errno: %d, blk: %04x, buf: "
			"%08x, sz: %08x\n", ret, errno, blk_num,
			(word32)buf, size);
		exit(1);
	}
}

void
disk_read_data(ProDisk *disk, int blk_num, byte *buf, int size)
{
	int	size_in_blocks;
	int	ret;
	int	i;

	size_in_blocks = size >> 9;
	if(size_in_blocks * 512 != size) {
		printf("disk_read: blk: %04x, buf: %08x, size: %08x\n",
			blk_num, (word32)buf, size);
		exit(1);
	}

	ret = lseek(disk->fd, 512*blk_num, SEEK_SET);
	if(ret != 512*blk_num) {
		printf("disk_read: seek: %d, errno: %d, blk: %04x, buf: "
			"%08x, sz: %08x\n", ret, errno, blk_num,
			(word32)buf, size);
		exit(1);
	}

	ret = read(disk->fd, buf, size);
	if(ret != size) {
		printf("disk_read: read: %d, errno: %d, blk: %04x, buf: "
			"%08x, sz: %08x\n", ret, errno, blk_num,
			(word32)buf, size);
		for(i = 0; i < size; i++) {
			buf[i] = 0;
		}
	}
}

Directory *
disk_read_dir(ProDisk *disk, int blk_num)
{
	disk_write_dir(disk, blk_num);

	disk->dir_blk_num = blk_num;
	disk_read_data(disk, blk_num, &(disk->dir_blk_data[0]), 512);

	return (Directory *)&(disk->dir_blk_data[0]);
}

void
disk_write_dir(ProDisk *disk, int blk_num)
{
	if(disk->dir_blk_num >= 0) {
		if(disk->dir_blk_num != blk_num) {
			printf("disk_write_dir: %04x != %04x\n",
				disk->dir_blk_num, blk_num);
		}
		disk_write_data(disk, disk->dir_blk_num,
				&(disk->dir_blk_data[0]), 512);
		disk->dir_blk_num = -1;
	}
}

void
create_new_file(ProDisk *disk, int dir_block, int storage_type, char *name,
	int file_type, word32 creation_time, int version, int min_version,
	int access, int aux_type, word32 last_mod, word32 eof)
{
	Vol_hdr	*vol_ptr;
	int	val;
	Directory *dir_ptr;
	File_entry *file_ptr;
	int	file_count;
	int	pos;
	int	last_pos;
	int	done;
	int	next_blk;
	int	name_len;

	name_len = strlen(name);
	dir_ptr = disk_read_dir(disk, dir_block);
	next_blk = dir_block;
	val = dir_ptr->file_entries[0].storage_type_name_len;
	last_pos = 13;
	pos = 0;
	if(((val & 0xf0) == 0xf0) || ((val & 0xf0) == 0xe0)) {
		/* vol dir or subdir header */
		vol_ptr = (Vol_hdr *)&(dir_ptr->file_entries[0]);
		file_count = get_l2byte(&(vol_ptr->file_count));
		pos = 1;
		last_pos = vol_ptr->entries_per_block;
	} else {
		printf("dir_block: %04x not subdir or voldir\n", dir_block);
		exit(6);
	}

	vol_ptr = 0;

	done = 0;
	while(!done) {
		file_ptr = &(dir_ptr->file_entries[pos]);
		if(((file_ptr->storage_type_name_len) & 0xf0) == 0) {
			/* Got it! */
			file_ptr->storage_type_name_len =
				(storage_type << 4) | name_len;
			strncpy((char *)file_ptr->file_name, name, 15);
			file_ptr->file_type = file_type;
			set_l2byte(&(file_ptr->key_pointer),
				find_next_free_block(disk));
			set_l2byte(&(file_ptr->blocks_used), 0);
			set_pro_time(&(file_ptr->creation_time),
				creation_time);
			file_ptr->version = version;
			file_ptr->min_version = min_version;
			file_ptr->access = access;
			set_l2byte(&(file_ptr->aux_type), aux_type);
			set_pro_time(&(file_ptr->last_mod), last_mod);
			set_l2byte(&(file_ptr->header_pointer),
					dir_block);
			set_l3byte(&(file_ptr->eof), eof);
			disk_write_dir(disk, next_blk);

			dir_ptr = disk_read_dir(disk, dir_block);
			vol_ptr = (Vol_hdr *)&(dir_ptr->file_entries[0]);
			set_l2byte(&(vol_ptr->file_count), file_count+1);
			disk_write_dir(disk, dir_block);

			disk_read_dir(disk, next_blk);
			/* re-read dir so that ptrs are set up right */
			disk->file_ptr = file_ptr;
			disk->file_open = 1;
			disk->ind_blk_num = -1;
			disk->master_ind_blk_num = -1;
			done = 1;
			break;
		} else {
			/* check to make sure name is unique */
			if((file_ptr->storage_type_name_len & 0x0f)== name_len){
				if(!memcmp(file_ptr->file_name, name,name_len)){
					printf("Name %s already on disk!\n",
						name);
					exit(8);
				}
			}
			pos++;
			if(pos >= last_pos) {
				/* Go to next block */
				next_blk = get_l2byte(&(dir_ptr->next_blk));
				if(next_blk) {
					dir_ptr = disk_read_dir(disk, next_blk);
					pos = 0;
				} else {
					printf("Top directory full!\n");
					exit(2);
				}
			}
		}
	}
}

int
pro_write_file(ProDisk *disk, byte *in_buf, int pos, int size)
{
	int	block;
	int	i;

	block = get_disk_block(disk, pos, 1);
	if(block < 7) {
		printf("pro_write_file, get_disk_block: %d\n", block);
		exit(3);
	}
	if(size < 512) {
		for(i = size; i < 512; i++) {
			in_buf[i] = 0;
		}
	} else if(size > 512) {
		printf("error, pro_write_file size: %d too big\n", size);
		exit(4);
	}

	disk_write_data(disk, block, in_buf, 512);

	return 0;
}



int
get_disk_block(ProDisk *disk, int pos, int create)
{
	File_entry *file_ptr;
	int	storage_type;
	word32	eof;
	int	lo, hi;
	int	offset;
	int	master_ind_block, ind_block;
	int	ret_block;
	int	key_block;

	if(pos >= 128*256*512) {
		printf("offset too big\n");
		exit(3);
	}

	file_ptr = disk->file_ptr;

	eof = get_l3byte(&(file_ptr->eof));
	storage_type = (file_ptr->storage_type_name_len) >> 4;

	key_block = get_l2byte(&(file_ptr->key_pointer));

	if(storage_type == 1 && pos >= 512) {
		/* make it sapling */
		get_new_ind_block(disk);
		inc_l2byte(&(file_ptr->blocks_used));
		disk->ind_blk_data[0] = key_block & 0xff;
		disk->ind_blk_data[0x100] = key_block >> 8;
		key_block = disk->ind_blk_num;
		set_l2byte(&(file_ptr->key_pointer), key_block);
		file_ptr->storage_type_name_len += 0x10;
		storage_type++;
	}
	if(storage_type == 2 && pos >= 256*512) {
		/* make it tree */
		get_new_master_ind_block(disk);
		inc_l2byte(&(file_ptr->blocks_used));
		disk->master_ind_blk_data[0] = key_block & 0xff;
		disk->master_ind_blk_data[0x100] = key_block >> 8;
		key_block = disk->master_ind_blk_num;
		set_l2byte(&(file_ptr->key_pointer), key_block);
		file_ptr->storage_type_name_len += 0x10;
		storage_type++;
	}

	switch(storage_type) {
	case 1:
		if(pos >= 512) {
			printf("Error1!\n");
			exit(3);
		}
		ret_block = key_block;
		if(ret_block == 0) {
			ret_block = find_next_free_block(disk);
			inc_l2byte(&(file_ptr->blocks_used));
			set_l2byte(&(file_ptr->key_pointer), ret_block);
		}
		return ret_block;
	case 2:
		ind_block = key_block;
		if(ind_block <= 0) {
			printf("write failure, ind_block: %d!\n", ind_block);
			exit(3);
		}
		offset = pos >> 9;
		if(offset >= 256) {
			printf("pos too big!\n");
			exit(3);
		}

		lo = disk->ind_blk_data[offset];
		hi = disk->ind_blk_data[offset + 0x100];
		ret_block = hi*256 + lo;
		if(ret_block == 0) {
			/* Need to alloc a block for this guy */
			ret_block = find_next_free_block(disk);
			inc_l2byte(&(file_ptr->blocks_used));
			disk->ind_blk_data[offset] = ret_block & 0xff;
			disk->ind_blk_data[offset + 0x100] =
						((ret_block >> 8) & 0xff);
		}
		return ret_block;
	case 3:
		/* tree */
		master_ind_block = key_block;
		if(master_ind_block <= 0) {
			printf("write failure, master_ind_block: %d!\n",
				master_ind_block);
			exit(3);
		}
		offset = pos >> 17;
		if(offset >= 128) {
			printf("master too big!\n");
			exit(4);
		}
		lo = disk->master_ind_blk_data[offset];
		hi = disk->master_ind_blk_data[offset + 0x100];
		ind_block = hi*256 + lo;
		if(ind_block == 0) {
			/* Need to alloc an ind block */
			get_new_ind_block(disk);
			ind_block = disk->ind_blk_num;
			inc_l2byte(&(file_ptr->blocks_used));
			disk->master_ind_blk_data[offset] = ind_block & 0xff;
			disk->master_ind_blk_data[offset + 0x100] =
				((ind_block >> 8) & 0xff);
		}

		offset = (pos >> 9) & 0xff;
		lo = disk->ind_blk_data[offset];
		hi = disk->ind_blk_data[offset + 0x100];
		ret_block = hi*256 + lo;

		if(ret_block == 0) {
			/* Need to alloc a block for this guy */
			ret_block = find_next_free_block(disk);
			inc_l2byte(&(file_ptr->blocks_used));
			disk->ind_blk_data[offset] = ret_block & 0xff;
			disk->ind_blk_data[offset + 0x100] =
						((ret_block >> 8) & 0xff);
		}
		return ret_block;
	default:
		printf("unknown storage type: %d\n", storage_type);
		exit(4);
	}

	printf("Can't get here!\n");
	exit(5);
}

void
get_new_ind_block(ProDisk *disk)
{
	int	ind_blk_num;
	int	i;

	write_ind_block(disk);

	ind_blk_num = find_next_free_block(disk);
	for(i = 0; i < 512; i++) {
		disk->ind_blk_data[i] = 0;
	}

	disk->ind_blk_num = ind_blk_num;
}

void
write_ind_block(ProDisk *disk)
{
	int	ind_blk_num;

	ind_blk_num = disk->ind_blk_num;
	if(ind_blk_num > 0) {
		printf("Write ind block: %04x\n", ind_blk_num);
		disk_write_data(disk, ind_blk_num, &(disk->ind_blk_data[0]),
			512);
		disk->ind_blk_num = -1;
	}
}

void
get_new_master_ind_block(ProDisk *disk)
{
	int	master_ind_blk_num;
	int	i;

	write_master_ind_block(disk);

	master_ind_blk_num = find_next_free_block(disk);
	for(i = 0; i < 512; i++) {
		disk->master_ind_blk_data[i] = 0;
	}

	disk->master_ind_blk_num = master_ind_blk_num;
}

void
write_master_ind_block(ProDisk *disk)
{
	int	master_ind_blk_num;

	master_ind_blk_num = disk->master_ind_blk_num;
	if(master_ind_blk_num > 0) {
		printf("Write master_ind block: %04x\n", master_ind_blk_num);
		disk_write_data(disk, master_ind_blk_num,
				&(disk->master_ind_blk_data[0]), 512);
		disk->master_ind_blk_num = -1;
	}
}

int
find_next_free_block(ProDisk *disk)
{
	byte	*bitmap_ptr;
	int	pos;
	int	bitmap_bytes;
	int	i, j;
	word32	val;

	bitmap_ptr = disk->bitmap_ptr;
	bitmap_bytes = disk->bitmap_bytes;
	pos = disk->bitmap_cur_pos;

	for(i = pos; i < bitmap_bytes; i++) {
		val = bitmap_ptr[i];
		if(val == 0) {
			continue;
		}
		for(j = 0; j < 8; j++) {
			if(val & (0x80 >> j)) {
				set_bitmap_used(bitmap_ptr, 8*i+j);
				disk->bitmap_cur_pos = i;
				return 8*i + j;
			}
		}
		return -1;
	}
	return -1;
}



void
set_bitmap_used(byte *ptr, int i)
{
	word32	offset, bit;
	word32	val;

	offset = i >> 3;
	bit = i & 7;

	val = ~(0x80 >> bit);
	ptr[offset] &= val;
}

void
set_bitmap_free(byte *ptr, int i)
{
	int offset, bit;
	int val;

	offset = i >> 3;
	bit = i & 7;

	val = (0x80 >> bit);
	ptr[offset] |= val;
}

void
set_file_entry(File_entry *entry, int storage_type_name_len, char *file_name,
	int file_type, int key_pointer, int blocks_used, int eof,
	word32 creation_time, int version, int min_version, int access,
	int aux_type, word32 last_mod, int header_pointer)
{

	entry->storage_type_name_len = storage_type_name_len;
	strncpy((char *)entry->file_name, file_name, 15);
	entry->file_type = file_type;
	set_l2byte(&(entry->key_pointer), key_pointer);
	set_l2byte(&(entry->blocks_used), blocks_used);
	set_l3byte(&(entry->eof), eof);
	set_pro_time(&(entry->creation_time), creation_time);
	entry->version = version;
	entry->min_version = min_version;
	entry->access = access;
	set_l2byte(&(entry->aux_type), aux_type);
	set_pro_time(&(entry->last_mod), last_mod);
	set_l2byte(&(entry->aux_type), header_pointer);

}

void
set_l2byte(L2byte *ptr, int val)
{
	ptr->low = (val & 0xff);
	ptr->hi = ((val >> 8) & 0xff);

}

void
set_l3byte(L3byte *ptr, int val)
{
	ptr->low = (val & 0xff);
	ptr->hi = ((val >> 8) & 0xff);
	ptr->higher = ((val >> 16) & 0xff);

}

void
set_pro_time(Pro_time *ptr, word32 val)
{
	ptr->times[0] = ((val >> 24) & 0xff);
	ptr->times[1] = ((val >> 16) & 0xff);
	ptr->times[2] = ((val >> 8) & 0xff);
	ptr->times[3] = ((val) & 0xff);
}

int
get_l2byte(L2byte *ptr)
{
	int val;

	val = ((ptr->hi) * 256) + ptr->low;
	return val;
}

int
get_l3byte(L3byte *ptr)
{
	int val;

	val = ((ptr->higher) * 65536) + ((ptr->hi) * 256) + ptr->low;
	return val;
}

void
inc_l2byte(L2byte *ptr)
{
	set_l2byte(ptr, get_l2byte(ptr) + 1);
}
