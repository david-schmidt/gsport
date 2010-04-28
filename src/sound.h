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

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined (__OS2__)
# include <sys/ipc.h>
# include <sys/shm.h>
#endif

#define SOUND_SHM_SAMP_SIZE		(32*1024)

#define SAMPLE_SIZE		2
#define NUM_CHANNELS		2
#define SAMPLE_CHAN_SIZE	(SAMPLE_SIZE * NUM_CHANNELS)

STRUCT(Doc_reg) {
	double	dsamp_ev;
	double	dsamp_ev2;
	double	complete_dsamp;
	int	samps_left;
	word32	cur_acc;
	word32	cur_inc;
	word32	cur_start;
	word32	cur_end;
	word32	cur_mask;
	int	size_bytes;
	int	event;
	int	running;
	int	has_irq_pending;
	word32	freq;
	word32	vol;
	word32	waveptr;
	word32	ctl;
	word32	wavesize;
	word32	last_samp_val;
};

/* prototypes for win32snd_driver.c functions */
void win32snd_init(word32 *);
void win32snd_shutdown();
void win32snd_shutdown();
void child_sound_init_win32();
int win32_send_audio(byte *ptr, int size);


/* Prototypes for macsnd_driver.c functions */
int mac_send_audio(byte *ptr, int in_size);
void child_sound_init_mac();
void macsnd_init(word32 *shmaddr);
