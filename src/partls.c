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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define BUF_SIZE	65536
char	buf[BUF_SIZE];

void
read_block(int fd, char *buf, int blk, int blk_size)
{
	int	ret;

	ret = lseek(fd, blk * blk_size, SEEK_SET);
	if(ret != blk * blk_size) {
		printf("lseek: %d, errno: %d\n", ret, errno);
		exit(1);
	}

	ret = read(fd, buf, blk_size);
	if(ret != blk_size) {
		printf("ret: %d, errno: %d\n", ret, errno);
		exit(1);
	}
}

int
main(int argc, char **argv)
{
	Driver_desc *driver_desc_ptr;
	Part_map *part_map_ptr;
	double	dsize;
	int	fd;
	int	block_size;
	word32	sig;
	word32	map_blk_cnt;
	word32	phys_part_start;
	word32	part_blk_cnt;
	word32	data_start;
	word32	data_cnt;
	int	map_blocks;
	int	cur;
	int	long_form;
	int	last_arg;
	int	i;

	/* parse args */
	long_form = 0;
	last_arg = 1;
	for(i = 1; i < argc; i++) {
		if(!strcmp("-l", argv[i])) {
			long_form = 1;
		} else {
			last_arg = i;
			break;
		}
	}


	fd = open(argv[last_arg], O_RDONLY | O_BINARY, 0x1b6);
	if(fd < 0) {
		printf("open %s, ret: %d, errno:%d\n", argv[last_arg],fd,errno);
		exit(1);
	}
	if(long_form) {
		printf("fd: %d\n", fd);
	}

	block_size = 512;
	read_block(fd, buf, 0, block_size);

	driver_desc_ptr = (Driver_desc *)buf;
	sig = GET_BE_WORD16(driver_desc_ptr->sig);
	block_size = GET_BE_WORD16(driver_desc_ptr->blk_size);
	if(long_form) {
		printf("sig: %04x, blksize: %04x\n", sig, block_size);
	}
	if(block_size == 0) {
		block_size = 512;
	}

	if(sig == 0x4552 && block_size >= 0x200) {
		if(long_form) {
			printf("good!\n");
		}
	} else {
		printf("bad sig:%04x or block_size:%04x!\n", sig, block_size);
		exit(1);
	}

	map_blocks = 1;
	cur = 0;
	while(cur < map_blocks) {
		read_block(fd, buf, cur + 1, block_size);
		part_map_ptr = (Part_map *)buf;
		sig = GET_BE_WORD16(part_map_ptr->sig);
		map_blk_cnt = GET_BE_WORD32(part_map_ptr->map_blk_cnt);
		phys_part_start = GET_BE_WORD32(part_map_ptr->phys_part_start);
		part_blk_cnt = GET_BE_WORD32(part_map_ptr->part_blk_cnt);
		data_start = GET_BE_WORD32(part_map_ptr->data_start);
		data_cnt = GET_BE_WORD32(part_map_ptr->data_cnt);

		if(cur == 0) {
			map_blocks = MIN(100, map_blk_cnt);
		}

		if(long_form) {
			printf("%2d: sig: %04x, map_blk_cnt: %d, "
				"phys_part_start: %08x, part_blk_cnt: %08x\n",
				cur, sig, map_blk_cnt, phys_part_start,
				part_blk_cnt);
			printf("  part_name: %s, part_type: %s\n",
				part_map_ptr->part_name,
				part_map_ptr->part_type);
			printf("  data_start:%08x, data_cnt:%08x status:%08x\n",
				GET_BE_WORD32(part_map_ptr->data_start),
				GET_BE_WORD32(part_map_ptr->data_cnt),
				GET_BE_WORD32(part_map_ptr->part_status));
			printf("  processor: %s\n", part_map_ptr->processor);
		} else {
			dsize = (double)GET_BE_WORD32(part_map_ptr->data_cnt);
			printf("%2d:%-20s  size=%6.2fMB type=%s\n", cur,
				part_map_ptr->part_name,
				(dsize/(1024.0*2.0)),
				part_map_ptr->part_type);
		}

		cur++;
	}

	close(fd);
	return 0;
}
