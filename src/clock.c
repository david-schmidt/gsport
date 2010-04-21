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

const char rcsid_clock_c[] = "@(#)$KmKId: clock.c,v 1.31 2004-10-19 17:32:07-04 kentd Exp $";

#include "defc.h"
#include <time.h>
#ifdef _WIN32
# include <windows.h>
# include <mmsystem.h>
#else
# include <sys/time.h>
#endif

extern int Verbose;
extern int g_vbl_count;
extern int g_rom_version;
extern int g_config_gsport_update_needed;

#define CLK_IDLE		1
#define CLK_TIME		2
#define CLK_INTERNAL		3
#define CLK_BRAM1		4
#define CLK_BRAM2		5

int	g_clk_mode = CLK_IDLE;
int	g_clk_read = 0;
int	g_clk_reg1 = 0;

extern int g_c033_data;
extern int g_c034_val;

byte	g_bram[2][256];
byte	*g_bram_ptr = &(g_bram[0][0]);

word32	g_clk_cur_time = 0xa0000000;
int	g_clk_next_vbl_update = 0;

double
get_dtime()
{
#ifndef _WIN32
	struct timeval tp1;
	double	dsec;
	double	dusec;
#endif
	double	dtime;

	/* Routine used to return actual system time as a double */
	/* No routine cares about the absolute value, only deltas--maybe */
	/*  take advantage of that in future to increase usec accuracy */

#ifdef _WIN32
	dtime = timeGetTime() / 1000.0;
#else

# ifdef SOLARIS
	gettimeofday(&tp1, (void *)0);
# else
	gettimeofday(&tp1, (struct timezone *)0);
# endif

	dsec = (double)tp1.tv_sec;
	dusec = (double)tp1.tv_usec;

	dtime = dsec + (dusec / (1000.0 * 1000.0));
#endif

	return dtime;
}

int
micro_sleep(double dtime)
{
#ifndef _WIN32
	struct timeval Timer;
	int	ret;
#endif

	if(dtime <= 0.0) {
		return 0;
	}
	if(dtime >= 1.0) {
		halt_printf("micro_sleep called with %f!!\n", dtime);
		return -1;
	}

#if 0
	printf("usleep: %f\n", dtime);
#endif

#ifdef _WIN32
	Sleep(dtime * 1000);
#else
	Timer.tv_sec = 0;
	Timer.tv_usec = (dtime * 1000000.0);
	if( (ret = select(0, 0, 0, 0, &Timer)) < 0) {
		fprintf(stderr, "micro_sleep (select) ret: %d, errno: %d\n",
			ret, errno);
		return -1;
	}
#endif
	return 0;
}

void
clk_bram_zero()
{
	int	i, j;

	/* zero out all bram */
	for(i = 0; i < 2; i++) {
		for(j = 0; j < 256; j++) {
			g_bram[i][j] = 0;
		}
	}
	g_bram_ptr = &(g_bram[0][0]);
}

void
clk_bram_set(int bram_num, int offset, int val)
{
	g_bram[bram_num][offset] = val;
}

void
clk_setup_bram_version()
{
	if(g_rom_version < 3) {
		g_bram_ptr = (&g_bram[0][0]);	// ROM 01
	} else {
		g_bram_ptr = (&g_bram[1][0]);	// ROM 03
	}
}

void
clk_write_bram(FILE *fconf)
{
	int	i, j, k;

	for(i = 0; i < 2; i++) {
		fprintf(fconf, "\n");
		for(j = 0; j < 256; j += 16) {
			fprintf(fconf, "bram%d[%02x] =", 2*i + 1, j);
			for(k = 0; k < 16; k++) {
				fprintf(fconf, " %02x", g_bram[i][j+k]);
			}
			fprintf(fconf, "\n");
		}
	}
}

void
update_cur_time()
{
	struct tm *tm_ptr;
	time_t	cur_time;
	unsigned int secs, secs2;

	cur_time = time(0);

	/* Figure out the timezone (effectively) by diffing two times. */
	/* this is probably not right for a few hours around daylight savings*/
	/*  time transition */
	secs2 = mktime(gmtime(&cur_time));
	tm_ptr = localtime(&cur_time);
	secs = mktime(tm_ptr);

#ifdef MAC
	/* Mac OS X's mktime function modifies the tm_ptr passed in for */
	/*  the CDT timezone and breaks this algorithm.  So on a Mac, we */
	/*  will use the tm_ptr->gmtoff member to correct the time */
	secs = secs + tm_ptr->tm_gmtoff;
#else
	secs = (unsigned int)cur_time - (secs2 - secs);

	if(tm_ptr->tm_isdst) {
		/* adjust for daylight savings time */
		secs += 3600;
	}
#endif

	/* add in secs to make date based on Apple Jan 1, 1904 instead of */
	/*   Unix's Jan 1, 1970 */
	/*  So add in 66 years and 17 leap year days (1904 is a leap year) */
	secs += ((66*365) + 17) * (24*3600);

	g_clk_cur_time = secs;

	clk_printf("Update g_clk_cur_time to %08x\n", g_clk_cur_time);
	g_clk_next_vbl_update = g_vbl_count + 5;
}

/* clock_update called by sim65816 every VBL */
void
clock_update()
{
	/* Nothing to do */
}

void
clock_update_if_needed()
{
	int	diff;

	diff = g_clk_next_vbl_update - g_vbl_count;
	if(diff < 0 || diff > 60) {
		/* Been a while, re-read the clock */
		update_cur_time();
	}
}

void
clock_write_c034(word32 val)
{
	g_c034_val = val & 0x7f;
	if((val & 0x80) != 0) {
		if((val & 0x20) == 0) {
			printf("c034 write not last = 1\n");
			/* set_halt(1); */
		}
		do_clock_data();
	}
}


void
do_clock_data()
{
	word32	mask;
	int	read;
	int	op;

	clk_printf("In do_clock_data, g_clk_mode: %02x\n", g_clk_mode);

	read = g_c034_val & 0x40;
	switch(g_clk_mode) {
	case CLK_IDLE:
		g_clk_read = (g_c033_data >> 7) & 1;
		g_clk_reg1 = (g_c033_data >> 2) & 3;
		op = (g_c033_data >> 4) & 7;
		if(!read) {
			/* write */
			switch(op) {
			case 0x0:	/* Read/write seconds register */
				g_clk_mode = CLK_TIME;
				clock_update_if_needed();
				break;
			case 0x3:	/* internal registers */
				g_clk_mode = CLK_INTERNAL;
				if(g_clk_reg1 & 0x2) {
					/* extend BRAM read */
					g_clk_mode = CLK_BRAM2;
					g_clk_reg1 = (g_c033_data & 7) << 5;
				}
				break;
			case 0x2:	/* read/write ram 0x10-0x13 */
				g_clk_mode = CLK_BRAM1;
				g_clk_reg1 += 0x10;
				break;
			case 0x4:	/* read/write ram 0x00-0x0f */
			case 0x5: case 0x6: case 0x7:
				g_clk_mode = CLK_BRAM1;
				g_clk_reg1 = (g_c033_data >> 2) & 0xf;
				break;
			default:
				halt_printf("Bad c033_data in CLK_IDLE: %02x\n",
					g_c033_data);
			}
		} else {
			printf("clk read from IDLE mode!\n");
			/* set_halt(1); */
			g_clk_mode = CLK_IDLE;
		}
		break;
	case CLK_BRAM2:
		if(!read) {
			/* get more bits of bram addr */
			if((g_c033_data & 0x83) == 0x00) {
				/* more address bits */
				g_clk_reg1 |= ((g_c033_data >> 2) & 0x1f);
				g_clk_mode = CLK_BRAM1;
			} else {
				halt_printf("CLK_BRAM2: c033_data: %02x!\n",
						g_c033_data);
				g_clk_mode = CLK_IDLE;
			}
		} else {
			halt_printf("CLK_BRAM2: clock read!\n");
			g_clk_mode = CLK_IDLE;
		}
		break;
	case CLK_BRAM1:
		/* access battery ram addr g_clk_reg1 */
		if(read) {
			if(g_clk_read) {
				/* Yup, read */
				g_c033_data = g_bram_ptr[g_clk_reg1];
				clk_printf("Reading BRAM loc %02x: %02x\n",
					g_clk_reg1, g_c033_data);
			} else {
				halt_printf("CLK_BRAM1: said wr, now read\n");
			}
		} else {
			if(g_clk_read) {
				halt_printf("CLK_BRAM1: said rd, now write\n");
			} else {
				/* Yup, write */
				clk_printf("Writing BRAM loc %02x with %02x\n",
					g_clk_reg1, g_c033_data);
				g_bram_ptr[g_clk_reg1] = g_c033_data;
				g_config_gsport_update_needed = 1;
			}
		}
		g_clk_mode = CLK_IDLE;
		break;
	case CLK_TIME:
		if(read) {
			if(g_clk_read == 0) {
				halt_printf("Reading time, but in set mode!\n");
			}
			g_c033_data = (g_clk_cur_time >> (g_clk_reg1 * 8)) &
									0xff;
			clk_printf("Returning time byte %d: %02x\n",
				g_clk_reg1, g_c033_data);
		} else {
			/* Write */
			if(g_clk_read) {
				halt_printf("Write time, but in read mode!\n");
			}
			clk_printf("Writing TIME loc %d with %02x\n",
				g_clk_reg1, g_c033_data);
			mask = 0xff << (8 * g_clk_reg1);

			g_clk_cur_time = (g_clk_cur_time & (~mask)) |
				((g_c033_data & 0xff) << (8 * g_clk_reg1));
		}
		g_clk_mode = CLK_IDLE;
		break;
	case CLK_INTERNAL:
		if(read) {
			printf("Attempting to read internal reg %02x!\n",
				g_clk_reg1);
		} else {
			switch(g_clk_reg1) {
			case 0x0:	/* test register */
				if(g_c033_data & 0xc0) {
					printf("Writing test reg: %02x!\n",
						g_c033_data);
					/* set_halt(1); */
				}
				break;
			case 0x1:	/* write protect reg */
				clk_printf("Writing clk wr_protect: %02x\n",
					g_c033_data);
				if(g_c033_data & 0x80) {
					printf("Stop, wr clk wr_prot: %02x\n",
						g_c033_data);
					/* set_halt(1); */
				}
				break;
			default:
				halt_printf("Writing int reg: %02x with %02x\n",
					g_clk_reg1, g_c033_data);
			}
		}
		g_clk_mode = CLK_IDLE;
		break;
	default:
		halt_printf("clk mode: %d unknown!\n", g_clk_mode);
		g_clk_mode = CLK_IDLE;
		break;
	}
}

