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
#include "sound.h"

#ifdef HPUX
# include <sys/audio.h>
#endif

#if defined(__linux__) || defined(OSS)
# include <sys/soundcard.h>
#endif

#ifndef _WIN32
# include <sys/socket.h>
# include <netinet/in.h>
#endif
#ifndef UNDER_CE
#include <errno.h>
#endif


extern int Verbose;

extern int g_audio_rate;

int	g_preferred_rate = 48000;
int	g_audio_socket = -1;
int	g_bytes_written = 0;

#define ZERO_BUF_SIZE		2048

word32 g_snd_zero_buf[ZERO_BUF_SIZE];

#define ZERO_PAUSE_SAFETY_SAMPS		(g_audio_rate >> 5)
#define ZERO_PAUSE_NUM_SAMPS		(4*g_audio_rate)

int	g_zeroes_buffered = 0;
int	g_zeroes_seen = 0;
int	g_sound_paused = 0;
int	g_childsnd_vbl = 0;
int	g_childsnd_pos = 0;
word32 *g_childsnd_shm_addr = 0;

void child_sound_init_linux();
void child_sound_init_hpdev();
void child_sound_init_win32();
void child_sound_init_mac();

void
reliable_buf_write(word32 *shm_addr, int pos, int size)
{
	byte	*ptr;
	int	ret;

	if(size < 1 || pos < 0 || pos > SOUND_SHM_SAMP_SIZE ||
				size > SOUND_SHM_SAMP_SIZE ||
				(pos + size) > SOUND_SHM_SAMP_SIZE) {
		printf("reliable_buf_write: pos: %04x, size: %04x\n",
			pos, size);
		exit(1);
	}

	ptr = (byte *)&(shm_addr[pos]);
	size = size * 4;

	while(size > 0) {
#ifdef _WIN32
		ret = win32_send_audio(ptr, size);
#else
# ifdef MAC
		ret = mac_send_audio(ptr, size);
# else
		ret = write(g_audio_socket, ptr, size);
# endif
#endif

		if(ret < 0) {
			printf("audio write, errno: %d\n", errno);
			exit(1);
		}
		size = size - ret;
		ptr += ret;
		g_bytes_written += ret;
	}

}

void
reliable_zero_write(int amt)
{
	int	len;

	while(amt > 0) {
		len = MIN(amt, ZERO_BUF_SIZE);
		reliable_buf_write(g_snd_zero_buf, 0, len);
		amt -= len;
	}
}


void
child_sound_loop(int read_fd, int write_fd, word32 *shm_addr)
{
#ifdef HPUX
	long	status_return;
#endif
	word32	tmp;
	int	ret;

	doc_printf("Child pipe fd: %d\n", read_fd);

	g_audio_rate = g_preferred_rate;

	g_zeroes_buffered = 0;
	g_zeroes_seen = 0;
	g_sound_paused = 0;

	g_childsnd_pos = 0;
	g_childsnd_vbl = 0;
	g_childsnd_shm_addr = shm_addr;

#ifdef HPUX
	child_sound_init_hpdev();
#endif
#if defined(__linux__) || defined(OSS)
	child_sound_init_linux();
#endif
#ifdef _WIN32
	child_sound_init_win32();
	return;
#endif
#ifdef MAC
	child_sound_init_mac();
	return;
#endif

	tmp = g_audio_rate;
	ret = write(write_fd, &tmp, 4);
	if(ret != 4) {
		printf("Unable to send back audio rate to parent\n");
		printf("ret: %d fd: %d, errno: %d\n", ret, write_fd, errno);
		exit(1);
	}
	printf("Wrote to fd %d the audio rate\n", write_fd);

	close(write_fd);

	while(1) {
		errno = 0;
		ret = read(read_fd, (char*)&tmp, 4);
		if(ret <= 0) {
			printf("child dying from ret: %d, errno: %d\n",
				ret, errno);
			break;
		}

		child_sound_playit(tmp);
	}

#ifdef HPUX
	ioctl(g_audio_socket, AUDIO_DRAIN, 0);
#endif
	close(g_audio_socket);

	exit(0);
}

void
child_sound_playit(word32 tmp)
{
	int	size;

	size = tmp & 0xffffff;

	//printf("child_sound_playit: %08x\n", tmp);

	if((tmp >> 24) == 0xa2) {
		/* play sound here */


#if 0
		g_childsnd_pos += g_zeroes_buffered;
		while(g_childsnd_pos >= SOUND_SHM_SAMP_SIZE) {
			g_childsnd_pos -= SOUND_SHM_SAMP_SIZE;
		}
#endif

		if(g_zeroes_buffered) {
			reliable_zero_write(g_zeroes_buffered);
		}

		g_zeroes_buffered = 0;
		g_zeroes_seen = 0;

		if((size + g_childsnd_pos) > SOUND_SHM_SAMP_SIZE) {
			reliable_buf_write(g_childsnd_shm_addr, g_childsnd_pos,
					SOUND_SHM_SAMP_SIZE - g_childsnd_pos);
			size = (g_childsnd_pos + size) - SOUND_SHM_SAMP_SIZE;
			g_childsnd_pos = 0;
		}

		reliable_buf_write(g_childsnd_shm_addr, g_childsnd_pos, size);

		if(g_sound_paused) {
			printf("Unpausing sound, zb: %d\n", g_zeroes_buffered);
			g_sound_paused = 0;
		}

	} else if((tmp >> 24) == 0xa1) {
		if(g_sound_paused) {
			if(g_zeroes_buffered < ZERO_PAUSE_SAFETY_SAMPS) {
				g_zeroes_buffered += size;
			}
		} else {
			/* not paused, send it through */
			g_zeroes_seen += size;

			reliable_zero_write(size);

			if(g_zeroes_seen >= ZERO_PAUSE_NUM_SAMPS) {
				printf("Pausing sound\n");
				g_sound_paused = 1;
			}
		}
	} else {
		printf("tmp received bad: %08x\n", tmp);
		exit(3);
	}

	g_childsnd_pos += size;
	while(g_childsnd_pos >= SOUND_SHM_SAMP_SIZE) {
		g_childsnd_pos -= SOUND_SHM_SAMP_SIZE;
	}

	g_childsnd_vbl++;
	if(g_childsnd_vbl >= 60) {
		g_childsnd_vbl = 0;
#if 0
		printf("sound bytes written: %06x\n", g_bytes_written);
		printf("Sample samples[0]: %08x %08x %08x %08x\n",
			g_childsnd_shm_addr[0], g_childsnd_shm_addr[1],
			g_childsnd_shm_addr[2], g_childsnd_shm_addr[3]);
		printf("Sample samples[100]: %08x %08x %08x %08x\n",
			g_childsnd_shm_addr[100], g_childsnd_shm_addr[101],
			g_childsnd_shm_addr[102], g_childsnd_shm_addr[103]);
#endif
		g_bytes_written = 0;
	}
}


#ifdef HPUX
void
child_sound_init_hpdev()
{
	struct audio_describe audio_descr;
	int	output_channel;
	char	*str;
	int	speaker;
	int	ret;
	int	i;

	g_audio_socket = open("/dev/audio", O_WRONLY, 0);
	if(g_audio_socket < 0) {
		printf("open /dev/audio failed, ret: %d, errno:%d\n",
			g_audio_socket, errno);
		exit(1);
	}

	ret = ioctl(g_audio_socket, AUDIO_DESCRIBE, &audio_descr);
	if(ret < 0) {
		printf("ioctl AUDIO_DESCRIBE failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	for(i = 0; i < audio_descr.nrates; i++) {
		printf("Audio rate[%d] = %d\n", i,
			audio_descr.sample_rate[i]);
	}

	ret = ioctl(g_audio_socket, AUDIO_SET_DATA_FORMAT,
			AUDIO_FORMAT_LINEAR16BIT);
	if(ret < 0) {
		printf("ioctl AUDIO_SET_DATA_FORMAT failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	ret = ioctl(g_audio_socket, AUDIO_SET_CHANNELS, NUM_CHANNELS);
	if(ret < 0) {
		printf("ioctl AUDIO_SET_CHANNELS failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	ret = ioctl(g_audio_socket, AUDIO_SET_TXBUFSIZE, 16*1024);
	if(ret < 0) {
		printf("ioctl AUDIO_SET_TXBUFSIZE failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	ret = ioctl(g_audio_socket, AUDIO_SET_SAMPLE_RATE, g_audio_rate);
	if(ret < 0) {
		printf("ioctl AUDIO_SET_SAMPLE_RATE failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	ret = ioctl(g_audio_socket, AUDIO_GET_OUTPUT, &output_channel);
	if(ret < 0) {
		printf("ioctl AUDIO_GET_OUTPUT failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}


	speaker = 1;
	str = getenv("SPEAKER");
	if(str) {
		if(str[0] != 'i' && str[0] != 'I') {
			speaker = 0;
		}
	}

	if(speaker) {
		printf("Sending sound to internal speaker\n");
		output_channel |= AUDIO_OUT_SPEAKER;
	} else {
		printf("Sending sound to external jack\n");
		output_channel &= (~AUDIO_OUT_SPEAKER);
		output_channel |= AUDIO_OUT_HEADPHONE;
	}

	ret = ioctl(g_audio_socket, AUDIO_SET_OUTPUT, output_channel);
	if(ret < 0) {
		printf("ioctl AUDIO_SET_OUTPUT failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}
}
#endif	/* HPUX */

#if defined(__linux__) || defined(OSS)
void
child_sound_init_linux()
{
	int	stereo;
	int	sample_size;
	int	rate;
	int	fragment;
	int	fmt;
	int	ret;

	g_audio_socket = open("/dev/dsp", O_WRONLY, 0);
	if(g_audio_socket < 0) {
		printf("open /dev/dsp failed, ret: %d, errno:%d\n",
			g_audio_socket, errno);
		exit(1);
	}

	fragment = 0x00200009;
#if 0
	ret = ioctl(g_audio_socket, SNDCTL_DSP_SETFRAGMENT, &fragment);
	if(ret < 0) {
		printf("ioctl SETFRAGEMNT failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}
#endif

	sample_size = 16;
	ret = ioctl(g_audio_socket, SNDCTL_DSP_SAMPLESIZE, &sample_size);
	if(ret < 0) {
		printf("ioctl SNDCTL_DSP_SAMPLESIZE failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

#ifdef GSPORT_LITTLE_ENDIAN
	fmt = AFMT_S16_LE;
#else
	fmt = AFMT_S16_BE;
#endif
	ret = ioctl(g_audio_socket, SNDCTL_DSP_SETFMT, &fmt);
	if(ret < 0) {
		printf("ioctl SNDCTL_DSP_SETFMT failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	stereo = 1;
	ret = ioctl(g_audio_socket, SNDCTL_DSP_STEREO, &stereo);
	if(ret < 0) {
		printf("ioctl SNDCTL_DSP_STEREO failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}

	rate = g_audio_rate;
	ret = ioctl(g_audio_socket, SNDCTL_DSP_SPEED, &rate);
	if(ret < 0) {
		printf("ioctl SNDCTL_DSP_SPEED failed, ret:%d, errno:%d\n",
			ret, errno);
		exit(1);
	}
	if(ret > 0) {
		rate = ret;	/* rate is returned value */
	}
	if(rate < 8000) {
		printf("Audio rate of %d which is < 8000!\n", rate);
		exit(1);
	}
	
	g_audio_rate = rate;

	printf("Sound initialized\n");
}
#endif
