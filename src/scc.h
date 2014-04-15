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

#include <ctype.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#if defined(HPUX) || defined(__linux__) || defined(SOLARIS) || defined(MAC) || defined(__MACH__) || defined(_WIN32)
# define SCC_SOCKETS
#endif


/* my scc port 0 == channel A, port 1 = channel B */

// LLAP may have packets up to 603 bytes, and the buffers must be large enough to contain a single packet.
#define	SCC_INBUF_SIZE		1024	/* must be a power of 2 */
#define	SCC_OUTBUF_SIZE		1024	/* must be a power of 2 */

#define SCC_MODEM_MAX_CMD_STR	128

#ifndef SOCKET
# define SOCKET		word32		/* for non-windows */
#endif

STRUCT(Scc) {
	int	port;
	int	state /* 0 == disconnected, 1 == real serial port, 2 == socket, 3 == LocalTalk */;
	int	accfd;
	SOCKET	sockfd;
	int	socket_state;
	int	rdwrfd;
	void	*host_handle;
	void	*host_handle2;
	int	host_aux1;
	int	read_called_this_vbl;
	int	write_called_this_vbl;

	int	mode;
	int	reg_ptr;
	int	reg[16];

	int	rx_queue_depth;
	byte	rx_queue[4];
	unsigned int lad;

	int	in_rdptr;
	int	in_wrptr;
	byte	in_buf[SCC_INBUF_SIZE];

	int	out_rdptr;
	int	out_wrptr;
	byte	out_buf[SCC_OUTBUF_SIZE];

	int	br_is_zero;
	int	tx_buf_empty;
	int	wantint_rx;
	int	wantint_tx;
	int	wantint_zerocnt;
	int did_int_rx_first;
	int	dcd;
	int sdlc_eof;
	int eom;

	double	br_dcycs;
	double	tx_dcycs;
	double	rx_dcycs;

	int	br_event_pending;
	int	rx_event_pending;
	int	tx_event_pending;
	byte irq_pending;

	int	char_size;
	int	baud_rate;
	double	out_char_dcycs;

	int	socket_num_rings;
	int	socket_last_ring_dcycs;
	word32	modem_mode;
	int	modem_dial_or_acc_mode;
	int	modem_plus_mode;
	int	modem_s0_val;
	int	telnet_mode;
	int	telnet_iac;
	word32	telnet_local_mode[2];
	word32	telnet_remote_mode[2];
	word32	telnet_reqwill_mode[2];
	word32	telnet_reqdo_mode[2];
	int	modem_cmd_len;
	byte	modem_cmd_str[SCC_MODEM_MAX_CMD_STR + 5];
};

#define SCCMODEM_NOECHO		0x0001
#define SCCMODEM_NOVERBOSE	0x0002
