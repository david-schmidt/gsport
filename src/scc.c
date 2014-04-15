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
#include "scc_llap.h"

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif

extern int Verbose;
extern int g_code_yellow;
extern double g_cur_dcycs;
extern int g_serial_type[];
extern int g_serial_out_masking;
extern int g_irq_pending;
extern int g_c041_val;
extern int g_appletalk_bridging;
extern int g_appletalk_turbo;

/* my scc port 0 == channel A = slot 1 = c039/c03b */
/*        port 1 == channel B = slot 2 = c038/c03a */

#include "scc.h"
#define SCC_R14_DPLL_SOURCE_BRG		0x100
#define SCC_R14_FM_MODE			0x200

#define SCC_DCYCS_PER_PCLK	((DCYCS_1_MHZ) / ((DCYCS_28_MHZ) /8))
#define SCC_DCYCS_PER_XTAL	((DCYCS_1_MHZ) / 3686400.0)

#define SCC_BR_EVENT			1
#define SCC_TX_EVENT			2
#define SCC_RX_EVENT			3
#define SCC_MAKE_EVENT(port, a)		(((a) << 1) + (port))

Scc	scc_stat[2];

int g_baud_table[] = {
	110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400
};

static char* wr_names[] = {
	"command", // 0
	"interrupt and transfer mode", // 1
	"interrupt vector", // 2
	"receive params", // 3
	"misc params", // 4
	"trasmit params", // 5
	"sync/addr field", // 6
	"sync/flag", // 7
	"transmit", // 8
	"master interrupt", // 9
	"trans/recv control", // 10
	"clock mode", // 11
	"baud rate (lower)", // 12
	"baud rate (upper)", // 13
	"misc control", // 14
	"ext/status interrupt" // 15
};
static char* rr_names[] = {
	"status",
	"special condition",
	"interrupt vector",
	"pending",
	"RR4",
	"RR5",
	"RR6",
	"RR7",
	"receive data",
	"RR9",
	"misc status",
	"time constant (lower)",
	"time constant (upper)",
	"RR14",
	"ext/status interrupt"
};

int g_scc_overflow = 0;

void
scc_init()
{
	Scc	*scc_ptr;
	int	i, j;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(scc_stat[i]);
		scc_ptr->accfd = -1;
		scc_ptr->sockfd = -1;
		scc_ptr->socket_state = -1;
		scc_ptr->rdwrfd = -1;
		scc_ptr->state = 0;
		scc_ptr->host_handle = 0;
		scc_ptr->host_handle2 = 0;
		scc_ptr->br_event_pending = 0;
		scc_ptr->rx_event_pending = 0;
		scc_ptr->tx_event_pending = 0;
		scc_ptr->char_size = 8;
		scc_ptr->baud_rate = 115200;
		scc_ptr->telnet_mode = 0;
		scc_ptr->telnet_iac = 0;
		scc_ptr->out_char_dcycs = 0.0;
		scc_ptr->socket_num_rings = 0;
		scc_ptr->socket_last_ring_dcycs = 0;
		scc_ptr->modem_mode = 0;
		scc_ptr->modem_dial_or_acc_mode = 0;
		scc_ptr->modem_plus_mode = 0;
		scc_ptr->modem_s0_val = 0;
		scc_ptr->modem_cmd_len = 0;
		scc_ptr->modem_cmd_str[0] = 0;
		for(j = 0; j < 2; j++) {
			scc_ptr->telnet_local_mode[j] = 0;
			scc_ptr->telnet_remote_mode[j] = 0;
			scc_ptr->telnet_reqwill_mode[j] = 0;
			scc_ptr->telnet_reqdo_mode[j] = 0;
		}
	}

	scc_reset();
}

void
scc_reset()
{
	Scc	*scc_ptr;
	int	i;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(scc_stat[i]);

		scc_ptr->port = i;
		scc_ptr->mode = 0;
		scc_ptr->reg_ptr = 0;
		scc_ptr->in_rdptr = 0;
		scc_ptr->in_wrptr = 0;
		scc_ptr->lad = 0;
		scc_ptr->out_rdptr = 0;
		scc_ptr->out_wrptr = 0;
		scc_ptr->dcd = 0;
		scc_ptr->wantint_rx = 0;
		scc_ptr->wantint_tx = 0;
		scc_ptr->wantint_zerocnt = 0;
		scc_ptr->did_int_rx_first = 0;
		scc_ptr->irq_pending = 0;
		scc_ptr->read_called_this_vbl = 0;
		scc_ptr->write_called_this_vbl = 0;
		scc_evaluate_ints(i);
		scc_hard_reset_port(i);
	}
}

void
scc_hard_reset_port(int port)
{
	Scc	*scc_ptr;

	scc_reset_port(port);

	scc_ptr = &(scc_stat[port]);
	scc_ptr->reg[14] = 0;		/* zero bottom two bits */
	scc_ptr->reg[13] = 0;
	scc_ptr->reg[12] = 0;
	scc_ptr->reg[11] = 0x08;
	scc_ptr->reg[10] = 0;
	scc_ptr->reg[7] = 0;
	scc_ptr->reg[6] = 0;
	scc_ptr->reg[5] = 0;
	scc_ptr->reg[4] = 0x04;
	scc_ptr->reg[3] = 0;
	scc_ptr->reg[2] = 0;
	scc_ptr->reg[1] = 0;

	/* HACK HACK: */
	scc_stat[0].reg[9] = 0;		/* Clear all interrupts */

	scc_evaluate_ints(port);

	scc_regen_clocks(port);
}

void
scc_reset_port(int port)
{
	Scc	*scc_ptr;

	scc_ptr = &(scc_stat[port]);
	scc_ptr->reg[15] = 0xf8;
	scc_ptr->reg[14] &= 0x03;	/* 0 most (including >= 0x100) bits */
	scc_ptr->reg[10] = 0;
	scc_ptr->reg[5] &= 0x65;	/* leave tx bits and sdlc/crc bits */
	scc_ptr->reg[4] |= 0x04;	/* Set async mode */
	scc_ptr->reg[3] &= 0xfe;	/* clear receiver enable */
	scc_ptr->reg[1] &= 0xfe;	/* clear ext int enable */

	scc_ptr->br_is_zero = 0;
	scc_ptr->tx_buf_empty = 1;

	scc_ptr->wantint_rx = 0;
	scc_ptr->wantint_tx = 0;
	scc_ptr->wantint_zerocnt = 0;

	scc_ptr->rx_queue_depth = 0;
	scc_ptr->sdlc_eof = 0;
	scc_ptr->eom = 1;

	scc_evaluate_ints(port);

	scc_regen_clocks(port);

	scc_clr_tx_int(port);
	scc_clr_rx_int(port);
}

void
scc_regen_clocks(int port)
{
	Scc	*scc_ptr;
	double	br_dcycs, tx_dcycs, rx_dcycs;
	double	rx_char_size, tx_char_size;
	double	clock_mult;
	word32	reg4;
	word32	reg14;
	word32	reg11;
	word32	br_const;
	word32	baud;
	word32	max_diff;
	word32	diff;
	int sync_mode = 0;
	int	baud_entries;
	int	pos;
	int	i;

	/*	Always do baud rate generator */
	scc_ptr = &(scc_stat[port]);
	br_const = (scc_ptr->reg[13] << 8) + scc_ptr->reg[12];
	br_const += 2;	/* counts down past 0 */

	reg4 = scc_ptr->reg[4];
	clock_mult = 1.0;
	switch((reg4 >> 6) & 3) {
	case 0:		/* x1 */
		clock_mult = 1.0;
		break;
	case 1:		/* x16 */
		clock_mult = 16.0;
		break;
	case 2:		/* x32 */
		clock_mult = 32.0;
		break;
	case 3:		/* x64 */
		clock_mult = 64.0;
		break;
	}

	br_dcycs = 0.01;
	reg14 = scc_ptr->reg[14];
	if(reg14 & 0x1) {
		br_dcycs = SCC_DCYCS_PER_XTAL;
		if(reg14 & 0x2) {
			br_dcycs = SCC_DCYCS_PER_PCLK;
		}
	}

	br_dcycs = br_dcycs * (double)br_const;

	tx_dcycs = 1;
	rx_dcycs = 1;
	reg11 = scc_ptr->reg[11];
	if(((reg11 >> 3) & 3) == 2) {
		tx_dcycs = 2.0 * br_dcycs * clock_mult;
	}
	switch ((reg11 >> 5) & 3) {
	case 0:
		// Receive clock = RTxC pin (not emulated)
	case 1:
		// Receive clock = TRxC pin (not emulated)
		// The real SCC has external pins that could provide the clock.  But, this is not emulated.
		break;
	case 3:
		// Receive clock = DPLL output
		// Only LocalTalk uses the DPLL receive clock.  We do not, however, emulate the DPLL.
		// In this case, the receive clock should be about the same as the transmit clock.
		rx_dcycs = tx_dcycs;
		break;
	case 2:
		// Receive clock = BRG output
		rx_dcycs = 2.0 * br_dcycs * clock_mult;
		break;
	}

	tx_char_size = 8.0;
	switch((scc_ptr->reg[5] >> 5) & 0x3) {
	case 0:	// 5 bits
		tx_char_size = 5.0;
		break;
	case 1:	// 7 bits
		tx_char_size = 7.0;
		break;
	case 2:	// 6 bits
		tx_char_size = 6.0;
		break;
	}

	scc_ptr->char_size = (int)tx_char_size;

	switch((scc_ptr->reg[4] >> 2) & 0x3) {
	case 0: // sync mode (no start or stop bits)
		sync_mode = 1;
		break;
	case 1:	// 1 stop bit
		tx_char_size += 2.0;	// 1 stop + 1 start bit
		break;
	case 2:	// 1.5 stop bit
		tx_char_size += 2.5;	// 1.5 stop + 1 start bit
		break;
	case 3:	// 2 stop bits
		tx_char_size += 3.0;	// 2.0 stop + 1 start bit
		break;
	}

	if((scc_ptr->reg[4] & 1) && !sync_mode) {
		// parity enabled
		tx_char_size += 1.0;
	}

	if(scc_ptr->reg[14] & 0x10) {
		/* loopback mode, make it go faster...*/
		rx_char_size = 1.0;
		tx_char_size = 1.0;
	}

	rx_char_size = tx_char_size;	/* HACK */

	baud = (int)(DCYCS_1_MHZ / tx_dcycs);
	max_diff = 5000000;
	pos = 0;
	baud_entries = sizeof(g_baud_table)/sizeof(g_baud_table[0]);
	for(i = 0; i < baud_entries; i++) {
		diff = abs((int)(g_baud_table[i] - baud));
		if(diff < max_diff) {
			pos = i;
			max_diff = diff;
		}
	}

	scc_ptr->baud_rate = g_baud_table[pos];

	scc_ptr->br_dcycs = br_dcycs;
	scc_ptr->tx_dcycs = tx_dcycs * tx_char_size;
	scc_ptr->rx_dcycs = rx_dcycs * rx_char_size;

	switch (scc_ptr->state) {
	case 1: /* socket */
		scc_socket_change_params(port);
		break;
	case 2: /* real serial ports */
#ifdef MAC
		scc_serial_mac_change_params(port);
#endif
#ifdef _WIN32
		scc_serial_win_change_params(port);
#endif
		break;
	case 3: /* localtalk */
		if (g_appletalk_turbo)
		{
			// If the user has selected AppleTalk "turbo" mode, increase the baud
			// rate to be as fast as possible, limited primarily by the ability of
			// the emulated GS to handle data.
			scc_ptr->baud_rate = 0;
			scc_ptr->br_dcycs = 1;
			scc_ptr->tx_dcycs = 1;
			scc_ptr->rx_dcycs = 1;
		}
		break;
	case 4: /* Imagewriter */
		scc_ptr->baud_rate = 230400;
		scc_ptr->tx_dcycs = tx_dcycs * 1.2; //Somehow this speeds up serial transfer without overrunning the buffer
		scc_ptr->rx_dcycs = rx_dcycs * 1.2;
		break;
	}

}

void
scc_port_init(int port)
{
	int	state;
	state = 0;
	switch (g_serial_type[port]) {
	case 0:
		break;
	case 1:
		#ifdef MAC
		state = scc_serial_mac_init(port);
		#endif
		#ifdef _WIN32
		state = scc_serial_win_init(port);
		#endif
		break;
	case 2:
		state = scc_imagewriter_init(port);
		break;
	default:
		break;
	}

	if(state <= 0) {
		scc_socket_init(port);
	}
}

void
scc_try_to_empty_writebuf(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	state;

	scc_ptr = &(scc_stat[port]);
	state = scc_ptr->state;
	if(scc_ptr->write_called_this_vbl) {
		return;
	}

	scc_ptr->write_called_this_vbl = 1;

	switch (state) 
	{
	case 2:
#if defined(MAC)
		scc_serial_mac_empty_writebuf(port);
#endif
#if defined(_WIN32)
		scc_serial_win_empty_writebuf(port);
#endif
		break;
	
	case 1:
		scc_socket_empty_writebuf(port, dcycs);
		break;

	case 3:
		// When we're doing LocalTalk, the write buffer gets emptied at the end of the frame and does not use this function.
		break;

	case 4:
		scc_imagewriter_empty_writebuf(port, dcycs);
		break;
	}
}

void
scc_try_fill_readbuf(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	space_used_before_rx, space_left;
	int space_used_after_rx;
	int	state;

	scc_ptr = &(scc_stat[port]);
	state = scc_ptr->state;

	space_used_before_rx = scc_ptr->in_wrptr - scc_ptr->in_rdptr;
	if(space_used_before_rx < 0) {
		space_used_before_rx += SCC_INBUF_SIZE;
	}
	space_left = (7*SCC_INBUF_SIZE/8) - space_used_before_rx;
	if(space_left < 1) {
		/* Buffer is pretty full, don't try to get more */
		return;
	}

#if 0
	if(scc_ptr->read_called_this_vbl) {
		return;
	}
#endif

	scc_ptr->read_called_this_vbl = 1;

	switch (state)
	{
	case 2:
#if defined(MAC)
		scc_serial_mac_fill_readbuf(port, space_left, dcycs);
#endif
#if defined(_WIN32)
		scc_serial_win_fill_readbuf(port, space_left, dcycs);
#endif
		break;
	
	case 1:
		scc_socket_fill_readbuf(port, space_left, dcycs);
		break;

	case 3:
		// LLAP deals with packets, and we only allow one packet in the read buffer at a time.
		// If the buffer is empty, try to fill it with another packet.
		if (g_appletalk_bridging && (space_used_before_rx == 0) && (scc_ptr->rx_queue_depth == 0) && !(scc_ptr->sdlc_eof))
		{
			scc_llap_fill_readbuf(port, space_left, dcycs);
			//scc_maybe_rx_event(port, dcycs);
			scc_ptr->sdlc_eof = 0;
		break;

	case 4:
		scc_imagewriter_fill_readbuf(port, space_left, dcycs);
		break;
		}
		break;
	}

	// Update the LAD (link activity detector), which LocalTalk uses in the CSMA/CA algorithm.
	// The real LAD depends on the line coding and data, but the "get bigger when data on RX line"
	// emulation is good enough since no software depends on the specific value of the LAD counter.
	// Practically, the emulated LLAP interface never has collisions and the LAD, therefore, is not
	// useful, but, for sake of correctness and more realisitic timing, emulate the LAD anyway.
	space_used_after_rx = scc_ptr->in_wrptr - scc_ptr->in_rdptr;
	if(space_used_after_rx < 0) {
		space_used_after_rx += SCC_INBUF_SIZE;
	}
	scc_ptr->lad += space_used_after_rx - space_used_before_rx;
}

void
scc_update(double dcycs)
{
	if (g_appletalk_bridging && (scc_stat[0].state == 3 || scc_stat[1].state == 3))
		scc_llap_update();

	/* called each VBL update */
	scc_stat[0].write_called_this_vbl = 0;
	scc_stat[1].write_called_this_vbl = 0;
	scc_stat[0].read_called_this_vbl = 0;
	scc_stat[1].read_called_this_vbl = 0;

	scc_try_fill_readbuf(0, dcycs);
	scc_try_fill_readbuf(1, dcycs);
	scc_stat[0].read_called_this_vbl = 0;
	scc_stat[1].read_called_this_vbl = 0;

	/* LLAP mode only transfers complete packets.  Retain the data in the
	   transmit and receive buffers until the buffers contain one complete packet */
	if (scc_stat[0].state != 3)
	{
		scc_try_to_empty_writebuf(0, dcycs);
		scc_stat[0].write_called_this_vbl = 0;
	}
	if (scc_stat[1].state != 3)
	{
		scc_try_to_empty_writebuf(1, dcycs);
		scc_stat[1].write_called_this_vbl = 0;
	}
}

void
do_scc_event(int type, double dcycs)
{
	Scc	*scc_ptr;
	int	port;

	port = type & 1;
	type = (type >> 1);

	scc_ptr = &(scc_stat[port]);
	if(type == SCC_BR_EVENT) {
		/* baud rate generator counted down to 0 */
		scc_ptr->br_event_pending = 0;
		scc_set_zerocnt_int(port);
		scc_maybe_br_event(port, dcycs);
	} else if(type == SCC_TX_EVENT) {
		scc_ptr->tx_event_pending = 0;
		scc_ptr->tx_buf_empty = 1;
		scc_handle_tx_event(port, dcycs);
	} else if(type == SCC_RX_EVENT) {
		scc_ptr->rx_event_pending = 0;
		scc_maybe_rx_event(port, dcycs);
	} else {
		halt_printf("do_scc_event: %08x!\n", type);
	}
	return;
}

void
show_scc_state()
{
	Scc	*scc_ptr;
	int	i, j;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(scc_stat[i]);
		printf("SCC port: %d\n", i);
		for(j = 0; j < 16; j += 4) {
			printf("Reg %2d-%2d: %02x %02x %02x %02x\n", j, j+3,
				scc_ptr->reg[j], scc_ptr->reg[j+1],
				scc_ptr->reg[j+2], scc_ptr->reg[j+3]);
		}
		printf("state: %d, accfd: %d, rdwrfd: %d, host:%p, host2:%p\n",
			scc_ptr->state, scc_ptr->accfd, scc_ptr->rdwrfd,
			scc_ptr->host_handle, scc_ptr->host_handle2);
		printf("in_rdptr: %04x, in_wr:%04x, out_rd:%04x, out_wr:%04x\n",
			scc_ptr->in_rdptr, scc_ptr->in_wrptr,
			scc_ptr->out_rdptr, scc_ptr->out_wrptr);
		printf("rx_queue_depth: %d, queue: %02x, %02x, %02x, %02x\n",
			scc_ptr->rx_queue_depth, scc_ptr->rx_queue[0],
			scc_ptr->rx_queue[1], scc_ptr->rx_queue[2],
			scc_ptr->rx_queue[3]);
		printf("want_ints: rx:%d, tx:%d, zc:%d\n",
			scc_ptr->wantint_rx, scc_ptr->wantint_tx,
			scc_ptr->wantint_zerocnt);
		printf("ev_pendings: rx:%d, tx:%d, br:%d\n",
			scc_ptr->rx_event_pending,
			scc_ptr->tx_event_pending,
			scc_ptr->br_event_pending);
		printf("br_dcycs: %f, tx_dcycs: %f, rx_dcycs: %f\n",
			scc_ptr->br_dcycs, scc_ptr->tx_dcycs,
			scc_ptr->rx_dcycs);
		printf("char_size: %d, baud_rate: %d, mode: %d\n",
			scc_ptr->char_size, scc_ptr->baud_rate,
			scc_ptr->mode);
		printf("modem_dial_mode:%d, telnet_mode:%d iac:%d, "
			"modem_cmd_len:%d\n", scc_ptr->modem_dial_or_acc_mode,
			scc_ptr->telnet_mode, scc_ptr->telnet_iac,
			scc_ptr->modem_cmd_len);
		printf("telnet_loc_modes:%08x %08x, telnet_rem_motes:"
			"%08x %08x\n", scc_ptr->telnet_local_mode[0],
			scc_ptr->telnet_local_mode[1],
			scc_ptr->telnet_remote_mode[0],
			scc_ptr->telnet_remote_mode[1]);
		printf("modem_mode:%08x plus_mode: %d, out_char_dcycs: %f\n",
			scc_ptr->modem_mode, scc_ptr->modem_plus_mode,
			scc_ptr->out_char_dcycs);
	}

}

#define LEN_SCC_LOG	5000
STRUCT(Scc_log) {
	int	regnum;
	word32	val;
	double	dcycs;
};

Scc_log	g_scc_log[LEN_SCC_LOG];
int	g_scc_log_pos = 0;

#define SCC_REGNUM(wr,port,reg) ((wr << 8) + (port << 4) + reg)

void
scc_log(int regnum, word32 val, double dcycs)
{
	int	pos;

	pos = g_scc_log_pos;
	g_scc_log[pos].regnum = regnum;
	g_scc_log[pos].val = val;
	g_scc_log[pos].dcycs = dcycs;
	pos++;
	if(pos >= LEN_SCC_LOG) {
		pos = 0;
	}
	g_scc_log_pos = pos;
}

void
show_scc_log(void)
{
	double	dcycs;
	int	regnum;
	int	pos;
	int	i;
	char* name;

	pos = g_scc_log_pos;
	dcycs = g_cur_dcycs;
	printf("SCC log pos: %d, cur dcycs:%f\n", pos, dcycs);
	for(i = 0; i < LEN_SCC_LOG; i++) {
		pos--;
		if(pos < 0) {
			pos = LEN_SCC_LOG - 1;
		}
		regnum = g_scc_log[pos].regnum;
		if (regnum >> 8)
			name = wr_names[regnum & 0xf];
		else
			name = rr_names[regnum & 0xf];

		printf("%d:%d:\tport:%d wr:%d reg: %d (%s)\t\tval:%02x \tat t:%f\n",
			i, pos, (regnum >> 4) & 0xf, (regnum >> 8),
			(regnum & 0xf),
			name, 
			g_scc_log[pos].val,
			g_scc_log[pos].dcycs /*- dcycs*/);
	}
}

word16 scc_read_lad(int port)
{
	// The IIgs provides a "LocalTalk link activity detector (LAD)" through repurposing the 
	// MegaII mouse interface.  Per the IIgs schematic, the MegaII mouse inputs connect via
	// the MSEX and MSEY lines to the RX lines of the SCC between the SCC and the line drivers.
	// So, if there's activity on the RX lines, the mouse counters increment.  The firmware
	// uses this for the "carrier sense" part of the CSMA/CA algorithm.  Typical firmware usage
	// is to (1) reset the mouse counter, (2) wait a bit, and (3) take action if some activity
	// is present.  The firmware does not appear to use the specific value of the LAD counter;
	// rather, the firmware only considers "zero" and "not zero".
	//
	// Apple engineers invented the term LAD, and there are references to it in Gus.

	if (port != 0 && port != 1)
	{
		halt_printf("Invalid SCC port.\n");
		return 0;
	}

	Scc* scc_ptr = &(scc_stat[port]);
	if (g_c041_val & C041_EN_MOUSE)
	{
		unsigned int temp = scc_ptr->lad;
		scc_ptr->lad = 0;
		return temp;
	}
	else
		return 0;
}

word32
scc_read_reg(int port, double dcycs)
{
	Scc	*scc_ptr;
	word32	ret;
	int	regnum;

	scc_ptr = &(scc_stat[port]);
	scc_ptr->mode = 0;
	regnum = scc_ptr->reg_ptr;

	/* port 0 is channel A, port 1 is channel B */
	switch(regnum) {
	case 0:
	case 4:
		ret = 0x20;
		if (scc_ptr->eom)
			ret |= 0x40;
		if(scc_ptr->dcd) {
			ret |= 0x08;
		}
		ret |= 0x8;	/* HACK HACK */
		if(scc_ptr->rx_queue_depth) {
			ret |= 0x01;
		}
		if(scc_ptr->tx_buf_empty) {
			ret |= 0x04;
		}
		if(scc_ptr->br_is_zero) {
			ret |= 0x02;
		}
		//printf("Read scc[%d] stat: %f : %02x\n", port, dcycs, ret);
		break;
	case 1:
	case 5:
		/* HACK: residue codes not right */
		ret = 0x07;	/* all sent */
		if (scc_ptr->state == 3 && scc_ptr->sdlc_eof)
			ret |= 0x80;
		break;
	case 2:
	case 6:
		if(port == 0) {
			ret = scc_ptr->reg[2];
		} else {

			halt_printf("Read of RR2B...stopping\n");
			ret = 0;
#if 0
			ret = scc_stat[0].reg[2];
			wr9 = scc_stat[0].reg[9];
			for(i = 0; i < 8; i++) {
				if(ZZZ){};
			}
			if(wr9 & 0x10) {
				/* wr9 status high */
				
			}
#endif
		}
		break;
	case 3:
	case 7:
		if(port == 0) {
			// The interrupt pending register only exists in channel A.
			ret = (scc_stat[0].irq_pending << 3) | scc_stat[1].irq_pending;
		} else {
			ret = 0;
		}
		break;
	case 8:
		ret = scc_read_data(port, dcycs);
		break;
	case 9:
	case 13:
		ret = scc_ptr->reg[13];
		break;
	case 10:
	case 14:
		ret = 0;
		break;
	case 11:
	case 15:
		ret = scc_ptr->reg[15];
		break;
	case 12:
		ret = scc_ptr->reg[12];
		break;
	default:
		halt_printf("Tried reading c03%x with regnum: %d!\n", 8+port,
			regnum);
		ret = 0;
	}

	scc_ptr->reg_ptr = 0;
	scc_printf("Read c03%x, rr%d, ret: %02x\n", 8+port, regnum, ret);
	//if(regnum != 0 && regnum != 3) {
		scc_log(SCC_REGNUM(0,port,regnum), ret, dcycs);
	//}

	return ret;
}

void
scc_write_reg(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	word32	old_val;
	word32	changed_bits;
	word32	irq_mask;
	int	regnum;
	int	mode;
	int	tmp1;

	scc_ptr = &(scc_stat[port]);
	regnum = scc_ptr->reg_ptr & 0xf;
	mode = scc_ptr->mode;

	// The SCC has more internal registers than memory locations mapped into the CPU's address space.
	// To access alternate registers, the CPU writes a register selection code to WR0.  The next write
	// goes to the selected register.  WR0 also contains several command and reset codes, and it is 
	// possible to write command, reset, and register selection in a single WR0 write.
	if(mode == 0) {
		// WR0 is selected, and this write goes to WR0.
		// Extract the register selection code, which determines the next register access in conjunction with the "point high" command.
		scc_ptr->reg_ptr = val & 0x07;
		if (((val >> 3) & 0x07) == 0x01)
			scc_ptr->reg_ptr |= 0x08;
			
		// But, this write goes to WR0.
		regnum = 0;
		if (scc_ptr->reg_ptr)
			scc_ptr->mode = 1;
	} else {
		// Some other register is selected, but the next access goes to register 0.
		scc_ptr->reg_ptr = 0;
		scc_ptr->mode = 0;
	}

	if ((regnum != 0) || // accesses to registers other than WR0
		((regnum == 0) && (val & 0xf8)) || // accesses to WR0 only for selecting a register
		((regnum == 0) && ((val & 0x38) == 0x80)) // access to WR0 with a point high register selection
	   )
		// To keep the log shorter and easier to read, omit register selection code write to WR0.  Log everything else.
		scc_log(SCC_REGNUM(1,port,regnum), val, dcycs);

	changed_bits = (scc_ptr->reg[regnum] ^ val) & 0xff;

	/* Set reg reg */
	switch(regnum) {
	case 0: /* wr0 */
		tmp1 = (val >> 3) & 0x7;
		switch(tmp1) {
		case 0x0: /* null code */
			break;
		case 0x1: /* point high */
			break;
		case 0x2:	/* reset ext/status ints */
			/* should clear other ext ints */
			scc_clr_zerocnt_int(port);
			break;
		case 0x3:   /* send abort (sdlc) */
			halt_printf("Wr c03%x to wr0 of %02x, bad cmd cd:%x!\n", 
				8+port, val, tmp1);
			scc_ptr->eom = 1;
			break;
		case 0x4:	/* enable int on next rx char */
			scc_ptr->did_int_rx_first = 0;
			break;
		case 0x5:	/* reset tx int pending */
			scc_clr_tx_int(port);
			break;
		case 0x6:	/* reset rr1 bits */
			// Per Section 5.2.1 of the SCC User's Manual, issuing an Error Reset when
			// a special condition exists (e.g. EOF) while using "interrupt on first RX"
			// mode causes loss of the the data with the special condition from the receive
			// FIFO.  In some cases, the GS relies on this behavior to clear the final CRC
			// byte from the RX FIFO.
			//
			// Based on experimentation, checking for an active first RX interrupt is incorrect.
			// System 5 fails to operate correctly with this check.  Anyway, skipping this check
			// seems to correct operation, but more investigation is necessary.
			if ((scc_ptr->sdlc_eof == 1) /*&& (scc_ptr->did_int_rx_first == 1)*/)
			{
				// Remove and discard one byte (the one causing the current special condition) from the RX FIFO.
				int depth = scc_ptr->rx_queue_depth;
				if (depth != 0) {
					for (int i = 1; i < depth; i++) {
						scc_ptr->rx_queue[i - 1] = scc_ptr->rx_queue[i];
					}
					scc_ptr->rx_queue_depth = depth - 1;
					scc_maybe_rx_event(port, dcycs);
					scc_maybe_rx_int(port, dcycs);
				}
			}

			// Reset emulated error bits.  Note that we don't emulate all the bits.
			scc_ptr->sdlc_eof = 0;
			break;
		case 0x7:	/* reset highest pri int pending */
			irq_mask = g_irq_pending;
			if(port == 0) {
				/* Move SCC0 ints into SCC1 positions */
				irq_mask = irq_mask >> 3;
			}
			if(irq_mask & IRQ_PENDING_SCC1_RX) {
				scc_clr_rx_int(port);
				scc_ptr->irq_pending &= ~IRQ_PENDING_SCC1_RX;
			} else if(irq_mask & IRQ_PENDING_SCC1_TX) {
				scc_clr_tx_int(port);
				scc_ptr->irq_pending &= ~IRQ_PENDING_SCC1_TX;
			} else if(irq_mask & IRQ_PENDING_SCC1_ZEROCNT) {
				scc_clr_zerocnt_int(port);
				scc_ptr->irq_pending &= ~IRQ_PENDING_SCC1_ZEROCNT;
			}
			break;
		default:
			halt_printf("Wr c03%x to wr0 of %02x, bad cmd cd:%x!\n",
				8+port, val, tmp1);
		}
		tmp1 = (val >> 6) & 0x3;
		switch(tmp1) {
		case 0x0:	/* null code */
			break;
		case 0x1:	/* reset rx crc */
			// Do nothing.  Emulated packets never have CRC errors.
			break;
		case 0x2:	/* reset tx crc */
			// Do nothing.  Emulated packets never have CRC errors.
			break;
		case 0x3:	/* reset tx underrun/eom latch */
			/* if no extern status pending, or being reset now */
			/*  and tx disabled, ext int with tx underrun */
			/* ah, just do nothing */
			//if (!(scc_ptr->reg[5] & 0x08))
				// First, this command has no effect unless the transmitter is disabled.
			//scc_ptr->eom = 0;
			break;
		}
		return;
	case 1: /* wr1 */
		/* proterm sets this == 0x10, which is int on all rx */
		scc_ptr->reg[regnum] = val;
		return;
	case 2: /* wr2 */
		/* All values do nothing, let 'em all through! */
		scc_ptr->reg[regnum] = val;
		return;
	case 3: /* wr3 */
		if((scc_ptr->state != 3) && ((val & 0x0e) != 0x0)) {
			halt_printf("Wr c03%x to wr3 of %02x!\n", 8+port, val);
		}
		old_val = scc_ptr->reg[regnum];
		scc_ptr->reg[regnum] = val;

		if (!(old_val & 0x01) && (val & 0x01))
		{
			// If the receiver transitions from disabled to enabled, try to pull data into the FIFO.
			scc_try_fill_readbuf(port, dcycs);
			scc_maybe_rx_event(port, dcycs);
		}
		
		return;
	case 4: /* wr4 */
		if((val & 0x30) != 0x00 && (val & 0x30) != 0x20) {
			halt_printf("Wr c03%x to wr4 of %02x!\n", 8+port, val);
		}

		if (((val >> 4) & 0x3) == 0x02 /* SDLC */ &&
			((val >> 2) & 0x3) == 0x00 /* enable sync modes */)
		{
			if (g_appletalk_bridging)
			{
				// SDLC mode enabled.  Redirect such data to the LocalTalk driver.
				scc_ptr->state = 3;
				scc_llap_init();
				printf("Enabled LocalTalk on port %d.\n", port);
			}
			else
				printf("Attempted to enable LocalTalk on port %d but bridging is disabled.\n", port);
		}

		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 5: /* wr5 */
		if((val & 0x10) != 0x0) {
			halt_printf("Wr c03%x to wr5 of %02x!\n", 8+port, val);
		}

		// Since we don't emulate the SDLC frame, ignore the CRC polynomial type (bit 2).
		// Since the emulated link never has CRC errors, silently accept Tx CRC enable.
		if (g_appletalk_bridging && scc_ptr->state == 3)
		{
			if ((scc_ptr->reg[regnum] & 0x08) && !(val & 0x08))
			{
				// When the TX enable changes from enabled to disabled, the GS is about to finish a frame.
				// The GS will wait a bit longer for the hardware to finish sending the abort sequence, but
				// this is of little concern since we don't have a "real line".
				scc_llap_empty_writebuf(port, dcycs);
				//scc_ptr->eom = 1;
			}
		}

		scc_ptr->reg[regnum] = val;
		if(changed_bits & 0x60) {
			scc_regen_clocks(port);
		}
		return;
	case 6: /* wr6 */
		if (scc_ptr->state == 3) {
			// In SDLC mode (state 3), WR6 contains the node ID for hardware address filtering.
			printf("Trying LocalTalk node ID %d.\n", val);
			scc_llap_set_node(val);
		}
		else if(val != 0) {
			halt_printf("Wr c03%x to wr6 of %02x!\n", 8+port, val);
		}

		scc_ptr->reg[regnum] = val;
		return;
	case 7: /* wr7 */
		if (((scc_ptr->state == 3) && (val != 0x7e)) || (scc_ptr->state != 3))
			// SDLC requires a sync character of 0x7e, per the SDLC spec.
			halt_printf("Wr c03%x to wr7 of %02x!\n", 8+port, val);

		scc_ptr->reg[regnum] = val;
		return;
	case 8: /* wr8 */
		scc_write_data(port, val, dcycs);
		return;
	case 9: /* wr9 */
		if((val & 0xc0)) {
			if(val & 0x80) {
				scc_reset_port(0);
			}
			if(val & 0x40) {
				scc_reset_port(1);
			}
			if((val & 0xc0) == 0xc0) {
				scc_hard_reset_port(0);
				scc_hard_reset_port(1);
			}
		}
		if((val & 0x35) != 0x00) {
			printf("Write c03%x to wr9 of %02x!\n", 8+port, val);
			halt_printf("val & 0x35: %02x\n", (val & 0x35));
		}
		old_val = scc_stat[0].reg[9];
		scc_stat[0].reg[regnum] = val;
		scc_evaluate_ints(0);
		scc_evaluate_ints(1);
		return;
	case 10: /* wr10 */
		if(((val & 0xff) != 0x00) &&
		   ((val & 0xe0) != 0xe0 && scc_ptr->state == 3) /* Allow FM0 */) {
			printf("Wr c03%x to wr10 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		return;
	case 11: /* wr11 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 12: /* wr12 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 13: /* wr13 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 14: /* wr14 */
		old_val = scc_ptr->reg[regnum];
		val = val + (old_val & (~0xff));
		switch((val >> 5) & 0x7) {
		case 0x0:
			// Null command.
		case 0x1:
			// Enter search mode command.
		case 0x2:
			// Reset clock missing command
		case 0x3:
			// Disable PLL command.
			break;
		
		case 0x4:	/* DPLL source is BR gen */
			val |= SCC_R14_DPLL_SOURCE_BRG;
			break;

		case 0x6:
			// Set FM mode.
			//
			// LocalTalk uses this mode.  
			// Ignore this command because we don't emulate line conding.
			if (scc_ptr->state != 3)
				halt_printf("Wr c03%x to wr14 of %02x, FM mode!\n",
				8+port, val);
			val |= SCC_R14_FM_MODE;
			break;

		case 0x5:
			// Set source = /RTxC.
		case 0x7:
			// Set NRZI mode.
		default:
			halt_printf("Wr c03%x to wr14 of %02x, bad dpll cd!\n",
				8+port, val);
		}
		if((val & 0x0c) != 0x0) {
			halt_printf("Wr c03%x to wr14 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		scc_maybe_br_event(port, dcycs);
		return;
	case 15: /* wr15 */
		/* ignore all accesses since IIgs self test messes with it */
		if((val & 0xff) != 0x0) {
			scc_printf("Write c03%x to wr15 of %02x!\n", 8+port,
				val);
		}
		if((scc_stat[0].reg[9] & 0x8) && (val != 0)) {
			printf("Write wr15:%02x and master int en = 1!\n",val);
			/* set_halt(1); */
		}
		scc_ptr->reg[regnum] = val;
		scc_maybe_br_event(port, dcycs);
		scc_evaluate_ints(port);
		return;
	default:
		halt_printf("Wr c03%x to wr%d of %02x!\n", 8+port, regnum, val);
		return;
	}
}

void
scc_maybe_br_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	br_dcycs;

	scc_ptr = &(scc_stat[port]);

	if(((scc_ptr->reg[14] & 0x01) == 0) || scc_ptr->br_event_pending) {
		return;
	}
	/* also, if ext ints not enabled, don't do baud rate ints */
	if((scc_ptr->reg[15] & 0x02) == 0) {
		return;
	}

	br_dcycs = scc_ptr->br_dcycs;
	if(br_dcycs < 1.0) {
		halt_printf("br_dcycs: %f!\n", br_dcycs);
	}

	scc_ptr->br_event_pending = 1;
	add_event_scc(dcycs + br_dcycs, SCC_MAKE_EVENT(port, SCC_BR_EVENT));
}

void
scc_evaluate_ints(int port)
{
	Scc	*scc_ptr;
	word32	irq_add_mask, irq_remove_mask;
	int	mie;

	scc_ptr = &(scc_stat[port]);
	mie = scc_stat[0].reg[9] & 0x8;			/* Master int en */

	// The master interrupt enable (MIE) gates assertion of the interrupt line.
	// Even if the MIE is disabled, the interrupt pending bits still reflect 
	// what interrupt would occur if MIE was enabled.  Software could poll the 
	// pending bits, and AppleTalk does exactly this to detect the start of 
	// a packet.  So, we must always calculate the pending interrupts.
	irq_add_mask = 0;
	irq_remove_mask = 0;
	if(scc_ptr->wantint_rx) {
		irq_add_mask |= IRQ_PENDING_SCC1_RX;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_RX;
	}
	if(scc_ptr->wantint_tx) {
		irq_add_mask |= IRQ_PENDING_SCC1_TX;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_TX;
	}
	if(scc_ptr->wantint_zerocnt) {
		irq_add_mask |= IRQ_PENDING_SCC1_ZEROCNT;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_ZEROCNT;
	}
	scc_stat[port].irq_pending &= ~irq_remove_mask;
	scc_stat[port].irq_pending |= irq_add_mask;


	if(!mie) {
		/* There can be no interrupts if MIE=0 */
		remove_irq(IRQ_PENDING_SCC1_RX | IRQ_PENDING_SCC1_TX |
						IRQ_PENDING_SCC1_ZEROCNT |
			IRQ_PENDING_SCC0_RX | IRQ_PENDING_SCC0_TX |
						IRQ_PENDING_SCC0_ZEROCNT);
		return;
	}
	if(port == 0) {
		/* Port 1 is in bits 0-2 and port 0 is in bits 3-5 */
		irq_add_mask = irq_add_mask << 3;
		irq_remove_mask = irq_remove_mask << 3;
	}
	if(irq_add_mask) {
		add_irq(irq_add_mask);
	}
	if(irq_remove_mask) {
		remove_irq(irq_remove_mask);
	}
}


void
scc_maybe_rx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	rx_dcycs;
	int	in_rdptr, in_wrptr;
	int	depth;

	scc_ptr = &(scc_stat[port]);

	if(scc_ptr->rx_event_pending) {
		/* one pending already, wait for the event to arrive */
		return;
	}

	if (!(scc_ptr->reg[3] & 0x01)) {
		// If the receiver is disabled, don't transfer data into the RX FIFO.
		return;
	}

	in_rdptr = scc_ptr->in_rdptr;
	in_wrptr = scc_ptr->in_wrptr;
	depth = scc_ptr->rx_queue_depth;
	if((in_rdptr == in_wrptr) || (depth >= 3)) {
		/* no more chars or no more space, just get out */
		return;
	}

	if(depth < 0) {
		depth = 0;
	}

	/* pull char from in_rdptr into queue */
	scc_ptr->rx_queue[depth] = scc_ptr->in_buf[in_rdptr];
	scc_ptr->in_rdptr = (in_rdptr + 1) & (SCC_INBUF_SIZE - 1);
	scc_ptr->rx_queue_depth = depth + 1;
	scc_maybe_rx_int(port, dcycs);
	rx_dcycs = scc_ptr->rx_dcycs;
	scc_ptr->rx_event_pending = 1;
	add_event_scc(dcycs + rx_dcycs, SCC_MAKE_EVENT(port, SCC_RX_EVENT));
}

void
scc_maybe_rx_int(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	depth;
	int	rx_int_mode;

	scc_ptr = &(scc_stat[port]);

	depth = scc_ptr->rx_queue_depth;
	if(depth <= 0) {
		/* no more chars, just get out */
		scc_clr_rx_int(port);
		return;
	}
	rx_int_mode = (scc_ptr->reg[1] >> 3) & 0x3;
	switch (rx_int_mode)
	{
	case 0:
		break;
	case 1: /* Rx Int On First Characters or Special Condition */
		// Based on experimentation, there's a delay in SDLC mode before the RX on first interrupt goes active.
		// Most likely, this delay is due to the address matching requiring complete reception of the destination address field.
		if (!scc_ptr->did_int_rx_first && ((scc_ptr->state != 3) || ((scc_ptr->state == 3) && (depth == 2))))
		{
			scc_ptr->did_int_rx_first = 1;
			scc_ptr->wantint_rx = 1;
		}
		break;
	case 2: /* Int On All Rx Characters or Special Condition */
		scc_ptr->wantint_rx = 1;
		break;
	case 3:
		halt_printf("Unsupported SCC RX interrupt mode 3 (Rx Int On Special Condition Only).");
		break;
	}
	scc_evaluate_ints(port);
}

void
scc_clr_rx_int(int port)
{
	scc_stat[port].wantint_rx = 0;
	scc_evaluate_ints(port);
}

void
scc_handle_tx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	tx_int_mode;

	scc_ptr = &(scc_stat[port]);

	/* nothing pending, see if ints on */
	tx_int_mode = (scc_ptr->reg[1] & 0x2);
	if(tx_int_mode) {
		scc_ptr->wantint_tx = 1;
	}
	scc_evaluate_ints(port);
}

void
scc_maybe_tx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	tx_dcycs;

	scc_ptr = &(scc_stat[port]);

	if(scc_ptr->tx_event_pending) {
		/* one pending already, tx_buf is full */
		scc_ptr->tx_buf_empty = 0;
	} else {
		/* nothing pending, see ints on */
		scc_evaluate_ints(port);
		tx_dcycs = scc_ptr->tx_dcycs;
		scc_ptr->tx_event_pending = 1;
		add_event_scc(dcycs + tx_dcycs,
				SCC_MAKE_EVENT(port, SCC_TX_EVENT));
	}
}

void
scc_clr_tx_int(int port)
{
	scc_stat[port].wantint_tx = 0;
	scc_evaluate_ints(port);
}

void
scc_set_zerocnt_int(int port)
{
	Scc	*scc_ptr;

	scc_ptr = &(scc_stat[port]);

	if(scc_ptr->reg[15] & 0x2) {
		scc_ptr->wantint_zerocnt = 1;
	}
	scc_evaluate_ints(port);
}

void
scc_clr_zerocnt_int(int port)
{
	scc_stat[port].wantint_zerocnt = 0;
	scc_evaluate_ints(port);
}

void
scc_add_to_readbuf(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	in_wrptr;
	int	in_wrptr_next;
	int	in_rdptr;

	scc_ptr = &(scc_stat[port]);

	in_wrptr = scc_ptr->in_wrptr;
	in_rdptr = scc_ptr->in_rdptr;
	in_wrptr_next = (in_wrptr + 1) & (SCC_INBUF_SIZE - 1);
	if(in_wrptr_next != in_rdptr) {
		scc_ptr->in_buf[in_wrptr] = val;
		scc_ptr->in_wrptr = in_wrptr_next;
		scc_printf("scc in port[%d] add char 0x%02x, %d,%d != %d\n",
				scc_ptr->port, val,
				in_wrptr, in_wrptr_next, in_rdptr);
		g_scc_overflow = 0;
	} else {
		if(g_scc_overflow == 0) {
			g_code_yellow++;
			printf("scc inbuf overflow port %d\n", port);
		}
		g_scc_overflow = 1;
	}

	scc_maybe_rx_event(port, dcycs);
}

void
scc_add_to_readbufv(int port, double dcycs, const char *fmt, ...)
{
	va_list	ap;
	char	*bufptr;
	int	len, c;
	int	i;

	va_start(ap, fmt);
	bufptr = (char*)malloc(4096);	// OG cast added
	bufptr[0] = 0;
	vsnprintf(bufptr, 4090, fmt, ap);
	len = strlen(bufptr);
	for(i = 0; i < len; i++) {
		c = bufptr[i];
		if(c == 0x0a) {
			scc_add_to_readbuf(port, 0x0d, dcycs);
		}
		scc_add_to_readbuf(port, c, dcycs);
	}
	va_end(ap);
}

void
scc_transmit(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	out_wrptr;
	int	out_rdptr;

	scc_ptr = &(scc_stat[port]);

	/* See if port initialized, if not, do so now */
	if(scc_ptr->state == 0) {
		scc_port_init(port);
	}
	if(scc_ptr->state < 0) {
		/* No working serial port, just toss it and go */
		return;
	}

	if(!scc_ptr->tx_buf_empty) {
		/* toss character! */
		printf("Tossing char\n");
		return;
	}

	out_wrptr = scc_ptr->out_wrptr;
	out_rdptr = scc_ptr->out_rdptr;
	if(scc_ptr->tx_dcycs < 1.0) {
		if(out_wrptr != out_rdptr) {
			/* do just one char, then get out */
			printf("tx_dcycs < 1\n");
			return;
		}
	}
	if(g_serial_out_masking && 
	   (scc_ptr->state != 3 /* never mask LLAP data */)) {
		val = val & 0x7f;
	}

	scc_add_to_writebuf(port, val, dcycs);
}

void
scc_add_to_writebuf(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	out_wrptr;
	int	out_wrptr_next;
	int	out_rdptr;

	scc_ptr = &(scc_stat[port]);

	/* See if port initialized, if not, do so now */
	if(scc_ptr->state == 0) {
		scc_port_init(port);
	}
	if(scc_ptr->state < 0) {
		/* No working serial port, just toss it and go */
		return;
	}

	out_wrptr = scc_ptr->out_wrptr;
	out_rdptr = scc_ptr->out_rdptr;

	out_wrptr_next = (out_wrptr + 1) & (SCC_OUTBUF_SIZE - 1);
	if(out_wrptr_next != out_rdptr) {
		scc_ptr->out_buf[out_wrptr] = val;
		scc_ptr->out_wrptr = out_wrptr_next;
		scc_printf("scc wrbuf port %d had char 0x%02x added\n",
			scc_ptr->port, val);
		g_scc_overflow = 0;
	} else {
		if(g_scc_overflow == 0) {
			g_code_yellow++;
			printf("scc outbuf overflow port %d\n", port);
		}
		g_scc_overflow = 1;
	}
}

word32
scc_read_data(int port, double dcycs)
{
	Scc	*scc_ptr;
	word32	ret;
	int	depth;
	int	i;

	scc_ptr = &(scc_stat[port]);

	scc_try_fill_readbuf(port, dcycs);

	depth = scc_ptr->rx_queue_depth;

	ret = 0;
	if(depth != 0) {
		ret = scc_ptr->rx_queue[0];
		for(i = 1; i < depth; i++) {
			scc_ptr->rx_queue[i-1] = scc_ptr->rx_queue[i];
		}
		scc_ptr->rx_queue_depth = depth - 1;
		scc_maybe_rx_event(port, dcycs);
		scc_maybe_rx_int(port, dcycs);

		int buffered_rx = scc_ptr->in_wrptr - scc_ptr->in_rdptr;
		if(buffered_rx < 0) {
			buffered_rx += SCC_INBUF_SIZE;
		}
		
		int bytes_left = buffered_rx + scc_ptr->rx_queue_depth;
		if (scc_ptr->state == 3 /* SDLC mode */ && bytes_left == 1)
		{
			// Flag an end of frame.
			scc_ptr->sdlc_eof = 1;
		}

		//printf("SCC read %04x: ret %02x, depth:%d, buffered: %d\n", 0xc03b - port, ret, scc_ptr->rx_queue_depth, buffered_rx);
	}
	
	scc_printf("SCC read %04x: ret %02x, depth:%d\n", 0xc03b-port, ret, depth);
	scc_log(SCC_REGNUM(0,port,8), ret, dcycs);

	return ret;
}


void
scc_write_data(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;

	scc_printf("SCC write %04x: %02x\n", 0xc03b-port, val);
	scc_log(SCC_REGNUM(1,port,8), val, dcycs);

	scc_ptr = &(scc_stat[port]);
	if(scc_ptr->reg[14] & 0x10) {
		/* local loopback! */
		scc_add_to_readbuf(port, val, dcycs);
	} else {
		scc_transmit(port, val, dcycs);
	}
	if (scc_ptr->state != 3) {
		// If we're doing LLAP, empty the writebuf at the end of the packet.
		// Otherwise, empty as soon as possible.
		scc_try_to_empty_writebuf(port, dcycs);
	}

	scc_maybe_tx_event(port, dcycs);
}