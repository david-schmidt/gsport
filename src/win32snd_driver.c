/************************************************************************/
/*			KEGS: Apple //gs Emulator			*/
/*			Copyright 2002 by Kent Dickey			*/
/*									*/
/*		This code is covered by the GNU GPL			*/
/*									*/
/*	The KEGS web page is kegs.sourceforge.net			*/
/*	You may contact the author at: kadickey@alumni.princeton.edu	*/
/************************************************************************/

const char rcsid_win32snd_driver_c[] = "@(#)$KmKId: win32snd_driver.c,v 1.5 2002-11-19 03:09:59-05 kadickey Exp $";

#include "defc.h"
#include "sound.h"

#ifndef __CYGWIN__
# include <windows.h>
# include <mmsystem.h>
#endif
//#include <unistd.h>

extern int Verbose;

extern int g_audio_rate;

unsigned int __stdcall child_sound_loop_win32(void *param);
void check_wave_error(int res, char *str);

#define NUM_WAVE_HEADERS	8

#ifndef __CYGWIN__
HWAVEOUT g_wave_handle;
WAVEHDR g_wavehdr[NUM_WAVE_HEADERS];
#endif

extern int g_audio_enable;
extern word32 *g_sound_shm_addr;
extern int g_preferred_rate;

int	g_win32snd_buflen = 0x1000;

void
win32snd_init(word32 *shmaddr)
{
	printf("win32snd_init\n");
	child_sound_loop(-1, -1, shmaddr);

	return;
}

void
win32snd_shutdown()
{
	/* hmm */

}

#ifndef __CYGWIN__

void CALLBACK
handle_wav_snd(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
		DWORD_PTR dwParam2)
{
	LPWAVEHDR	lpwavehdr;

	/* Only service "buffer done playing messages */
	if(uMsg == WOM_DONE) {
		lpwavehdr = (LPWAVEHDR)dwParam1;
		if(lpwavehdr->dwFlags == (WHDR_DONE | WHDR_PREPARED)) {
			lpwavehdr->dwUser = FALSE;
		}
	}

	return;
}
void
check_wave_error(int res, char *str)
{
	char	buf[256];

	if(res == MMSYSERR_NOERROR) {
		return;
	}

	waveOutGetErrorText(res, &buf[0], sizeof(buf));
	printf("%s: %s\n", str, buf);
	exit(1);
}

void
child_sound_init_win32()
{
	WAVEFORMATEX wavefmt;
	WAVEOUTCAPS caps;
	byte	*bptr;
	int	bits_per_sample, channels, block_align;
	int	blen;
	int	res;
	int	i;

	memset(&wavefmt, 0, sizeof(WAVEFORMATEX));

	wavefmt.wFormatTag = WAVE_FORMAT_PCM;
	bits_per_sample = 16;
	channels = 2;
	wavefmt.wBitsPerSample = bits_per_sample;
	wavefmt.nChannels = channels;
	wavefmt.nSamplesPerSec = g_audio_rate;
	block_align = channels * (bits_per_sample / 8);
	wavefmt.nBlockAlign = block_align;
	wavefmt.nAvgBytesPerSec = block_align * g_audio_rate;

	res = waveOutOpen(&g_wave_handle, WAVE_MAPPER, &wavefmt, 0, 0,
				WAVE_FORMAT_QUERY);

	if(res != MMSYSERR_NOERROR) {
		printf("Cannot open audio device\n");
		g_audio_enable = 0;
		return;
	}

	res = waveOutOpen(&g_wave_handle, WAVE_MAPPER, &wavefmt,
		(DWORD_PTR)handle_wav_snd, 0, CALLBACK_FUNCTION | WAVE_ALLOWSYNC);

	if(res != MMSYSERR_NOERROR) {
		printf("Cannot register audio\n");
		g_audio_enable = 0;
		return;
	}

	g_audio_rate = wavefmt.nSamplesPerSec;

	blen = (SOUND_SHM_SAMP_SIZE * 4 * 2) / NUM_WAVE_HEADERS;
	g_win32snd_buflen = blen;
	bptr = malloc(blen * NUM_WAVE_HEADERS);
	if(bptr == NULL) {
		printf("Unabled to allocate sound buffer\n");
		exit(1);
	}

	for(i = 0; i < NUM_WAVE_HEADERS; i++) {
		memset(&g_wavehdr[i], 0, sizeof(WAVEHDR));
		g_wavehdr[i].dwUser = FALSE;
		g_wavehdr[i].lpData = &(bptr[i*blen]);
		g_wavehdr[i].dwBufferLength = blen;
		g_wavehdr[i].dwFlags = 0;
		g_wavehdr[i].dwLoops = 0;
		res = waveOutPrepareHeader(g_wave_handle, &g_wavehdr[i],
						sizeof(WAVEHDR));
		check_wave_error(res, "waveOutPrepareHeader");
	}

	res = waveOutGetDevCaps((UINT)g_wave_handle, &caps, sizeof(caps));
	check_wave_error(res, "waveOutGetDevCaps");
	printf("Using %s\n", caps.szPname);
	printf(" Bits per Sample = %d.  Channels = %d\n",
		wavefmt.wBitsPerSample, wavefmt.nChannels);
	printf(" Sampling rate = %d, avg_bytes_per_sec = %d\n",
		(int)wavefmt.nSamplesPerSec, (int)wavefmt.nAvgBytesPerSec);

	set_audio_rate(g_audio_rate);

}

void
win32_send_audio2(byte *ptr, int size)
{
	int	found;
	int	res;
	int	i;

	found = 0;
	for(i = 0; i < NUM_WAVE_HEADERS; i++) {
		if(g_wavehdr[i].dwUser == FALSE) {
			found = 1;
			break;
		}
	}

	if(!found) {
		/* all audio buffers busy, just get out */
		return;
	}

	memcpy(g_wavehdr[i].lpData, ptr, size);
	g_wavehdr[i].dwBufferLength = size;
	g_wavehdr[i].dwUser = TRUE;

	res = waveOutWrite(g_wave_handle, &g_wavehdr[i], sizeof(g_wavehdr));
	check_wave_error(res, "waveOutWrite");

	return;
}

int
win32_send_audio(byte *ptr, int in_size)
{
	int	size;
	int	tmpsize;

	size = in_size;
	while(size > 0) {
		tmpsize = size;
		if(size > g_win32snd_buflen) {
			tmpsize = g_win32snd_buflen;
		}
		win32_send_audio2(ptr, tmpsize);
		ptr += tmpsize;
		size = size - tmpsize;
	}

	return in_size;
}

#endif /* __CYGWIN */
