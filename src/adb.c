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

/* adb_mode bit 3 and bit 2 (faster repeats for arrows and space/del) not done*/

#include "adb.h"

int g_fullscreen = 0;

extern int Verbose;
extern word32 g_vbl_count;
extern int g_num_lines_prev_superhires640;
extern int g_num_lines_prev_superhires;
extern int g_rom_version;
extern int g_fast_disk_emul;
extern int g_limit_speed;
extern int g_irq_pending;
extern int g_swap_paddles;
extern int g_invert_paddles;
extern int g_joystick_type;
extern int g_a2vid_palette;
extern int g_config_control_panel;
extern word32 g_cfg_vbl_count;
extern double g_cur_dcycs;

extern byte *g_slow_memory_ptr;
extern byte *g_memory_ptr;
extern word32 g_mem_size_total;

enum {
	ADB_IDLE = 0,
	ADB_IN_CMD,
	ADB_SENDING_DATA,
};

#define ADB_C027_MOUSE_DATA	0x80
#define ADB_C027_MOUSE_INT	0x40
#define ADB_C027_DATA_VALID	0x20
#define ADB_C027_DATA_INT	0x10
#define ADB_C027_KBD_VALID	0x08
#define ADB_C027_KBD_INT	0x04
#define ADB_C027_MOUSE_COORD	0x02
#define ADB_C027_CMD_FULL	0x01

#define ADB_C027_NEG_MASK	( ~ (				\
	 	ADB_C027_MOUSE_DATA | ADB_C027_DATA_VALID |	\
		ADB_C027_KBD_VALID | ADB_C027_MOUSE_COORD |	\
		ADB_C027_CMD_FULL))


int halt_on_all_c027 = 0;

word32	g_adb_repeat_delay = 45;
word32	g_adb_repeat_rate = 3;
word32	g_adb_repeat_info = 0x23;
word32	g_adb_char_set = 0x0;
word32	g_adb_layout_lang = 0x0;

word32	g_adb_interrupt_byte = 0;
int	g_adb_state = ADB_IDLE;

word32	g_adb_cmd = (word32)-1;
int	g_adb_cmd_len = 0;
int	g_adb_cmd_so_far = 0;
word32	g_adb_cmd_data[16];

#define MAX_ADB_DATA_PEND	16

word32	g_adb_data[MAX_ADB_DATA_PEND];
int	g_adb_data_pending = 0;

word32	g_c027_val = 0;
word32	g_c025_val = 0;

byte	adb_memory[256];

word32 g_adb_mode = 0;		/* mode set via set_modes, clear_modes */

int g_warp_pointer = 0;
int g_hide_pointer = 0;
int g_unhide_pointer = 0;

int g_mouse_a2_x = 0;
int g_mouse_a2_y = 0;
int g_mouse_a2_button = 0;
int g_mouse_fifo_pos = 0;
int g_mouse_raw_x = 0;
int g_mouse_raw_y = 0;

#define ADB_MOUSE_FIFO		8

STRUCT(Mouse_fifo) {
	double	dcycs;
	int	x;
	int	y;
	int	buttons;
};

Mouse_fifo g_mouse_fifo[ADB_MOUSE_FIFO] = { { 0, 0, 0, 0 } };

int g_mouse_warp_x = 0;
int g_mouse_warp_y = 0;

int	g_adb_mouse_valid_data = 0;
int	g_adb_mouse_coord = 0;

#define MAX_KBD_BUF		8

int	g_key_down = 0;
int	g_hard_key_down = 0;
int	g_a2code_down = 0;
int	g_kbd_read_no_update = 0;
int	g_kbd_chars_buffered = 0;
int	g_kbd_buf[MAX_KBD_BUF];
word32	g_adb_repeat_vbl = 0;

int	g_kbd_dev_addr = 2;		/* ADB physical kbd addr */
int	g_mouse_dev_addr = 3;		/* ADB physical mouse addr */

int	g_kbd_ctl_addr = 2;		/* ADB microcontroller's kbd addr */
int	g_mouse_ctl_addr = 3;		/* ADB ucontroller's mouse addr*/
			/* above are ucontroller's VIEW of where mouse/kbd */
			/*  are...if they are moved, mouse/keyboard funcs */
			/*  should stop (c025, c000, c024, etc). */

word32	g_virtual_key_up[4];	/* bitmask of all possible 128 a2codes */
				/* indicates which keys are up=1 by bit */

int	g_keypad_key_is_down[10] = { 0 };/* List from 0-9 of which keypad */
					/*  keys are currently pressed */


#define SHIFT_DOWN	( (g_c025_val & 0x01) )
#define CTRL_DOWN	( (g_c025_val & 0x02) )
#define CAPS_LOCK_DOWN	( (g_c025_val & 0x04) )
#define OPTION_DOWN	( (g_c025_val & 0x40) )
#define CMD_DOWN	( (g_c025_val & 0x80) )


#define MAX_ADB_KBD_REG3	16

int g_kbd_reg0_pos = 0;
int g_kbd_reg0_data[MAX_ADB_KBD_REG3];
int g_kbd_reg3_16bit = 0x602;			/* also set in adb_reset()! */


int	g_adb_init = 0;

void
adb_init()
{
	int	keycode;
	int	i;

	if(g_adb_init) {
		halt_printf("g_adb_init = %d!\n", g_adb_init);
	}
	g_adb_init = 1;

	for(i = 0; i < 128; i++) {
		keycode = a2_key_to_ascii[i][0];
		if(keycode != i) {
			printf("ADB keycode lost/skipped: i=%x: keycode=%x\n",
				i, keycode);
			my_exit(1);
		}
	}

	g_c025_val = 0;

	for(i = 0; i < 4; i++) {
		g_virtual_key_up[i] = -1;
	}

	for(i = 0; i < 10; i++) {
		g_keypad_key_is_down[i] = 0;
	}

	adb_reset();
}


void
adb_reset()
{

	g_c027_val = 0;

	g_key_down = 0;

	g_kbd_dev_addr = 2;
	g_mouse_dev_addr = 3;

	g_kbd_ctl_addr = 2;
	g_mouse_ctl_addr = 3;

	adb_clear_data_int();
	adb_clear_mouse_int();
	adb_clear_kbd_srq();

	g_adb_data_pending = 0;
	g_adb_interrupt_byte = 0;
	g_adb_state = ADB_IDLE;
	g_adb_mouse_coord = 0;
	g_adb_mouse_valid_data = 0;

	g_kbd_reg0_pos = 0;
	g_kbd_reg3_16bit = 0x602;

}


#define LEN_ADB_LOG	16
STRUCT(Adb_log) {
	word32	addr;
	int	val;
	int	state;
};

Adb_log g_adb_log[LEN_ADB_LOG];
int	g_adb_log_pos = 0;

void
adb_log(word32 addr, int val)
{
	int	pos;

	pos = g_adb_log_pos;
	g_adb_log[pos].addr = addr;
	g_adb_log[pos].val = val;
	g_adb_log[pos].state = g_adb_state;
	pos++;
	if(pos >= LEN_ADB_LOG) {
		pos = 0;
	}
	g_adb_log_pos = pos;
}

void
show_adb_log(void)
{
	int	pos;
	int	i;

	pos = g_adb_log_pos;
	printf("ADB log pos: %d\n", pos);
	for(i = 0; i < LEN_ADB_LOG; i++) {
		pos--;
		if(pos < 0) {
			pos = LEN_ADB_LOG - 1;
		}
		printf("%d:%d:  addr:%04x = %02x, st:%d\n", i, pos,
			g_adb_log[pos].addr, g_adb_log[pos].val,
			g_adb_log[pos].state);
	}
	printf("kbd: dev: %x, ctl: %x; mouse: dev: %x, ctl: %x\n",
		g_kbd_dev_addr, g_kbd_ctl_addr,
		g_mouse_dev_addr, g_mouse_ctl_addr);
	printf("g_adb_state: %d, g_adb_interrupt_byte: %02x\n",
		g_adb_state, g_adb_interrupt_byte);
}

void
adb_error(void)
{
	halt_printf("Adb Error\n");

	show_adb_log();
}



void
adb_add_kbd_srq()
{
	if(g_kbd_reg3_16bit & 0x200) {
		/* generate SRQ */
		g_adb_interrupt_byte |= 0x08;
		add_irq(IRQ_PENDING_ADB_KBD_SRQ);
	} else {
		printf("Got keycode but no kbd SRQ!\n");
	}
}

void
adb_clear_kbd_srq()
{
	remove_irq(IRQ_PENDING_ADB_KBD_SRQ);

	/* kbd SRQ's are the only ones to handle now, so just clean it out */
	g_adb_interrupt_byte &= (~(0x08));
}

void
adb_add_data_int()
{
	if(g_c027_val & ADB_C027_DATA_INT) {
		add_irq(IRQ_PENDING_ADB_DATA);
	}
}

void
adb_add_mouse_int()
{
	if(g_c027_val & ADB_C027_MOUSE_INT) {
		add_irq(IRQ_PENDING_ADB_MOUSE);
	}
}

void
adb_clear_data_int()
{
	remove_irq(IRQ_PENDING_ADB_DATA);
}

void
adb_clear_mouse_int()
{
	remove_irq(IRQ_PENDING_ADB_MOUSE);
}


void
adb_send_bytes(int num_bytes, word32 val0, word32 val1, word32 val2)
{
	word32	val;
	int	shift_amount;
	int	i;

	if((num_bytes >= 12) || (num_bytes >= MAX_ADB_DATA_PEND))  {
		halt_printf("adb_send_bytes: %d is too many!\n", num_bytes);
	}

	g_adb_state = ADB_SENDING_DATA;
	g_adb_data_pending = num_bytes;
	adb_add_data_int();

	for(i = 0; i < num_bytes; i++) {
		if(i < 4) {
			val = val0;
		} else if(i < 8) {
			val = val1;
		} else {
			val = val2;
		}

		shift_amount = 8*(3 - i);
		g_adb_data[i] = (val >> shift_amount) & 0xff;
		adb_printf("adb_send_bytes[%d] = %02x\n", i, g_adb_data[i]);
	}
}


void
adb_send_1byte(word32 val)
{

	if(g_adb_data_pending != 0) {
		halt_printf("g_adb_data_pending: %d\n", g_adb_data_pending);
	}

	adb_send_bytes(1, val << 24, 0, 0);
}



void
adb_response_packet(int num_bytes, word32 val)
{

	if(g_adb_data_pending != 0) {
		halt_printf("adb_response_packet, but pending: %d\n",
			g_adb_data_pending);
	}

	g_adb_state = ADB_IDLE;
	g_adb_data_pending = num_bytes;
	g_adb_data[0] = val & 0xff;
	g_adb_data[1] = (val >> 8) & 0xff;
	g_adb_data[2] = (val >> 16) & 0xff;
	g_adb_data[3] = (val >> 24) & 0xff;
	if(num_bytes) {
		g_adb_interrupt_byte |= 0x80 + num_bytes - 1;
	} else {
		g_adb_interrupt_byte |= 0x80;
	}

	adb_printf("adb_response packet: %d: %08x\n",
		num_bytes, val);

	adb_add_data_int();
}


void
adb_kbd_reg0_data(int a2code, int is_up)
{
	if(g_kbd_reg0_pos >= MAX_ADB_KBD_REG3) {
		/* too many keys, toss */
		halt_printf("Had to toss key: %02x, %d\n", a2code, is_up);
		return;
	}

	g_kbd_reg0_data[g_kbd_reg0_pos] = a2code + (is_up << 7);

	adb_printf("g_kbd_reg0_data[%d] = %02x\n", g_kbd_reg0_pos,
		g_kbd_reg0_data[g_kbd_reg0_pos]);

	g_kbd_reg0_pos++;

	adb_add_kbd_srq();
}

void
adb_kbd_talk_reg0()
{
	word32	val0, val1;
	word32	reg;
	int	num_bytes;
	int	num;
	int	i;

	num = 0;
	val0 = g_kbd_reg0_data[0];
	val1 = g_kbd_reg0_data[1];

	num_bytes = 0;
	if(g_kbd_reg0_pos > 0) {
		num_bytes = 2;
		num = 1;
		if((val0 & 0x7f) == 0x7f) {
			/* reset */
			val1 = val0;
		} else if(g_kbd_reg0_pos > 1) {
			num = 2;
			if((val1 & 0x7f) == 0x7f) {
				/* If first byte some other key, don't */
				/*  put RESET next! */
				num = 1;
				val1 = 0xff;
			}
		} else {
			val1 = 0xff;
		}
	}

	if(num) {
		for(i = num; i < g_kbd_reg0_pos; i++) {
			g_kbd_reg0_data[i-1] = g_kbd_reg0_data[i];
		}
		g_kbd_reg0_pos -= num;
	}

	reg = (val0 << 8) + val1;

	adb_printf("adb_kbd_talk0: %04x\n", reg);

	adb_response_packet(num_bytes, reg);
	if(g_kbd_reg0_pos == 0) {
		adb_clear_kbd_srq();
	}
}

void
adb_set_config(word32 val0, word32 val1, word32 val2)
{
	int	new_mouse;
	int	new_kbd;
	int	tmp1;

	new_mouse = val0 >> 4;
	new_kbd = val0  & 0xf;
	if(new_mouse != g_mouse_ctl_addr) {
		printf("ADB config: mouse from %x to %x!\n",
			g_mouse_ctl_addr, new_mouse);
		adb_error();
		g_mouse_ctl_addr = new_mouse;
	}
	if(new_kbd != g_kbd_ctl_addr) {
		printf("ADB config: kbd from %x to %x!\n",
			g_kbd_ctl_addr, new_kbd);
		adb_error();
		g_kbd_ctl_addr = new_kbd;
	}


	tmp1 = val2 >> 4;
	if(tmp1 == 4) {
		g_adb_repeat_delay = 0;
	} else if(tmp1 < 4) {
		g_adb_repeat_delay = (tmp1 + 1) * 15;
	} else {
		halt_printf("Bad ADB repeat delay: %02x\n", tmp1);
	}

	tmp1 = val2 & 0xf;
	if(g_rom_version >= 3) {
		tmp1 = 9 - tmp1;
	}

	switch(tmp1) {
	case 0:
		g_adb_repeat_rate = 1;
		break;
	case 1:
		g_adb_repeat_rate = 2;
		break;
	case 2:
		g_adb_repeat_rate = 3;
		break;
	case 3:
		g_adb_repeat_rate = 3;
		break;
	case 4:
		g_adb_repeat_rate = 4;
		break;
	case 5:
		g_adb_repeat_rate = 5;
		break;
	case 6:
		g_adb_repeat_rate = 7;
		break;
	case 7:
		g_adb_repeat_rate = 15;
		break;
	case 8:
		/* I don't know what this should be, ROM 03 uses it */
		g_adb_repeat_rate = 30;
		break;
	case 9:
		/* I don't know what this should be, ROM 03 uses it */
		g_adb_repeat_rate = 60;
		break;
	default:
		halt_printf("Bad repeat rate: %02x\n", tmp1);
	}

}

void
adb_set_new_mode(word32 val)
{
	if(val & 0x03) {
		printf("Disabling keyboard/mouse:%02x!\n", val);
	}

	if(val & 0xa2) {
		halt_printf("ADB set mode: %02x!\n", val);
		adb_error();
	}

	g_adb_mode = val;
}


int
adb_read_c026()
{
	word32	ret;
	int	i;

	ret = 0;
	switch(g_adb_state) {
	case ADB_IDLE:
		ret = g_adb_interrupt_byte;
		g_adb_interrupt_byte = 0;
		if(g_irq_pending & IRQ_PENDING_ADB_KBD_SRQ) {
			g_adb_interrupt_byte |= 0x08;
		}
		if(g_adb_data_pending == 0) {
			if(ret & 0x80) {
				halt_printf("read_c026: ret:%02x, pend:%d\n",
					ret, g_adb_data_pending);
			}
			adb_clear_data_int();
		}
		if(g_adb_data_pending) {
			if(g_adb_state != ADB_IN_CMD) {
				g_adb_state = ADB_SENDING_DATA;
			}
		}
		break;
	case ADB_IN_CMD:
		ret = 0;
		break;
	case ADB_SENDING_DATA:
		ret = g_adb_data[0];
		for(i = 1; i < g_adb_data_pending; i++) {
			g_adb_data[i-1] = g_adb_data[i];
		}
		g_adb_data_pending--;
		if(g_adb_data_pending <= 0) {
			g_adb_data_pending = 0;
			g_adb_state = ADB_IDLE;
			adb_clear_data_int();
		}
		break;
	default:
		halt_printf("Bad ADB state: %d!\n", g_adb_state);
		adb_clear_data_int();
		break;
	}

	adb_printf("Reading c026.  Returning %02x, st: %02x, pend: %d\n",
		ret, g_adb_state, g_adb_data_pending);

	adb_log(0xc026, ret);
	return (ret & 0xff);
}


void
adb_write_c026(int val)
{
	word32	tmp;
	int	dev;

	adb_printf("Writing c026 with %02x\n", val);
	adb_log(0x1c026, val);


	switch(g_adb_state) {
	case ADB_IDLE:
		g_adb_cmd = val;
		g_adb_cmd_so_far = 0;
		g_adb_cmd_len = 0;

		dev = val & 0xf;
		switch(val) {
		case 0x01:	/* Abort */
			adb_printf("Performing adb abort\n");
			/* adb_abort() */
			break;
		case 0x03:	/* Flush keyboard buffer */
			adb_printf("Flushing adb keyboard buffer\n");
			/* Do nothing */
			break;
		case 0x04:	/* Set modes */
			adb_printf("ADB set modes\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 1;
			break;
		case 0x05:	/* Clear modes */
			adb_printf("ADB clear modes\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 1;
			break;
		case 0x06:	/* Set config */
			adb_printf("ADB set config\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 3;
			break;
		case 0x07:	/* Sync */
			adb_printf("Performing sync cmd!\n");
			g_adb_state = ADB_IN_CMD;
			if(g_rom_version == 1) {
				g_adb_cmd_len = 4;
			} else {
				g_adb_cmd_len = 8;
			}
			break;
		case 0x08:	/* Write mem */
			adb_printf("Starting write_mem cmd\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 2;
			break;
		case 0x09:	/* Read mem */
			adb_printf("Performing read_mem cmd!\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 2;
			break;
		case 0x0a:	/* Read modes byte */
			printf("Performing read_modes cmd!\n");
			/* set_halt(1); */
			adb_send_1byte(g_adb_mode);
			break;
		case 0x0b:	/* Read config bytes */
			printf("Performing read_configs cmd!\n");
			tmp = (g_mouse_ctl_addr << 20) +
				(g_kbd_ctl_addr << 16) +
				(g_adb_char_set << 12) +
				(g_adb_layout_lang << 8) +
				(g_adb_repeat_info << 0);
			tmp = (0x82U << 24) + tmp;
			adb_send_bytes(4, tmp, 0, 0);
			break;
		case 0x0d:	/* Get Version */
			adb_printf("Performing get_version cmd!\n");
			val = 0;
			if(g_rom_version == 1) {
				/* ROM 01 = revision 5 */
				val = 5;
			} else {
				/* ROM 03 checks for rev >= 6 */
				val = 6;
			}
			adb_send_1byte(val);
			break;
		case 0x0e:	/* Read avail char sets */
			adb_printf("Performing read avail char sets cmd!\n");
			adb_send_bytes(2,	/* just 2 bytes */
				0x08000000,	/* number of ch sets=0x8 */
				0, 0);
			/* set_halt(1); */
			break;
		case 0x0f:	/* Read avail kbd layouts */
			adb_printf("Performing read avail kbd layouts cmd!\n");
			adb_send_bytes(0x2,	/* number of kbd layouts=0xa */
				0x0a000000, 0, 0);
			/* set_halt(1); */
			break;
		case 0x10:	/* Reset */
			printf("ADB reset, cmd 0x10\n");
			do_reset();
			break;
		case 0x11:	/* Send ADB keycodes */
			adb_printf("Sending ADB keycodes\n");
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 1;
			break;
		case 0x12:	/* ADB cmd 12: ROM 03 only! */
			if(g_rom_version >= 3) {
				g_adb_state = ADB_IN_CMD;
				g_adb_cmd_len = 2;
			} else {
				printf("ADB cmd 12, but not ROM 3!\n");
				adb_error();
			}
			break;
		case 0x13:	/* ADB cmd 13: ROM 03 only! */
			if(g_rom_version >= 3) {
				g_adb_state = ADB_IN_CMD;
				g_adb_cmd_len = 2;
			} else {
				printf("ADB cmd 13, but not ROM 3!\n");
				adb_error();
			}
			break;
		case 0x73:	/* Disable SRQ device 3: mouse */
			adb_printf("Disabling Mouse SRQ's (device 3)\n");
			/* HACK HACK...should deal with SRQs on mouse */
			break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			/* Listen dev x reg 3 */
			adb_printf("Sending data to dev %x reg 3\n", dev);
			g_adb_state = ADB_IN_CMD;
			g_adb_cmd_len = 2;
			break;
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			/* Talk dev x reg 0 */
			adb_printf("Performing talk dev %x reg 0\n", dev);
			if(dev == g_kbd_dev_addr) {
				adb_kbd_talk_reg0();
			} else {
				printf("Unknown talk dev %x reg 0!\n", dev);
				/* send no data, on SRQ, system polls devs */
				/*  so we don't want to send anything */
				adb_error();
			}
			break;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
			/* Talk dev x reg 3 */
			adb_printf("Performing talk dev %x reg 3\n", dev);
			if(dev == g_kbd_dev_addr) {
				adb_response_packet(2, g_kbd_reg3_16bit);
			} else {
				printf("Performing talk dev %x reg 3!!\n", dev);
				adb_error();
			}
			break;
		default:
			halt_printf("ADB ucontroller cmd %02x unknown!\n", val);
			/* The Gog's says ACS Demo 2 has a bug and writes to */
			/*  c026 */
			break;
		}
		break;
	case ADB_IN_CMD:
		adb_printf("Setting byte %d of cmd %02x to %02x\n",
			g_adb_cmd_so_far, g_adb_cmd, val);

		g_adb_cmd_data[g_adb_cmd_so_far] = val;
		g_adb_cmd_so_far++;
		if(g_adb_cmd_so_far >= g_adb_cmd_len) {
			adb_printf("Finished cmd %02x\n", g_adb_cmd);
			do_adb_cmd();
		}

		break;
	default:
		printf("adb_state: %02x is unknown!  Setting it to ADB_IDLE\n",
			g_adb_state);
		g_adb_state = ADB_IDLE;
		adb_error();
		halt_on_all_c027 = 1;
		break;
	}
	return;
}

void
do_adb_cmd()
{
	int	dev;
	int	new_kbd;
	int	addr;
	int	val;

	dev = g_adb_cmd & 0xf;

	g_adb_state = ADB_IDLE;

	switch(g_adb_cmd) {
	case 0x04:	/* Set modes */
		adb_printf("Performing ADB set mode: OR'ing in %02x\n",
			g_adb_cmd_data[0]);

		val = g_adb_cmd_data[0] | g_adb_mode;
		adb_set_new_mode(val);

		break;
	case 0x05:	/* clear modes */
		adb_printf("Performing ADB clear mode: AND'ing in ~%02x\n",
			g_adb_cmd_data[0]);

		val = g_adb_cmd_data[0];
		val = g_adb_mode & (~val);
		adb_set_new_mode(val);
		break;
	case 0x06:	/* Set config */
		adb_printf("Set ADB config to %02x %02x %02x\n",
			g_adb_cmd_data[0], g_adb_cmd_data[1],g_adb_cmd_data[2]);

		adb_set_config(g_adb_cmd_data[0], g_adb_cmd_data[1],
			g_adb_cmd_data[2]);

		break;
	case 0x07:	/* SYNC */
		adb_printf("Performing ADB SYNC\n");
		adb_printf("data: %02x %02x %02x %02x\n",
			g_adb_cmd_data[0], g_adb_cmd_data[1], g_adb_cmd_data[2],
			g_adb_cmd_data[3]);

		adb_set_new_mode(g_adb_cmd_data[0]);
		adb_set_config(g_adb_cmd_data[1], g_adb_cmd_data[2],
				g_adb_cmd_data[3]);

		if(g_rom_version >= 3) {
			adb_printf("  and cmd12:%02x %02x cmd13:%02x %02x\n",
				g_adb_cmd_data[4], g_adb_cmd_data[5],
				g_adb_cmd_data[6], g_adb_cmd_data[7]);
		}
		break;
	case 0x08:	/* Write mem */
		addr = g_adb_cmd_data[0];
		val = g_adb_cmd_data[1];
		write_adb_ram(addr, val);
		break;
	case 0x09:	/* Read mem */
		addr = (g_adb_cmd_data[1] << 8) + g_adb_cmd_data[0];
		adb_printf("Performing mem read to addr %04x\n", addr);
		adb_send_1byte(read_adb_ram(addr));
		break;
	case 0x11:	/* Send ADB keycodes */
		val = g_adb_cmd_data[0];
		adb_printf("Performing send ADB keycodes: %02x\n", val);
		adb_virtual_key_update(val & 0x7f, val >> 7);
		break;
	case 0x12:	/* ADB cmd12 */
		adb_printf("Performing ADB cmd 12\n");
		adb_printf("data: %02x %02x\n", g_adb_cmd_data[0],
							g_adb_cmd_data[1]);
		break;
	case 0x13:	/* ADB cmd13 */
		adb_printf("Performing ADB cmd 13\n");
		adb_printf("data: %02x %02x\n", g_adb_cmd_data[0],
							g_adb_cmd_data[1]);
		break;
	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb:
	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		/* Listen dev x reg 3 */
		if(dev == g_kbd_dev_addr) {
			if(g_adb_cmd_data[1] == 0xfe) {
				/* change keyboard addr? */
				new_kbd = g_adb_cmd_data[0] & 0xf;
				if(new_kbd != dev) {
					printf("Moving kbd to dev %x!\n",
								new_kbd);
					adb_error();
				}
				g_kbd_dev_addr = new_kbd;
			} else if(g_adb_cmd_data[1] != 1) {
				/* see what new device handler id is */
				printf("KBD listen to dev %x reg 3: 1:%02x\n",
					dev, g_adb_cmd_data[1]);
				adb_error();
			}
			if(g_adb_cmd_data[0] != (word32)g_kbd_dev_addr) {
				/* see if app is trying to change addr */
				printf("KBD listen to dev %x reg 3: 0:%02x!\n",
					dev, g_adb_cmd_data[0]);
				adb_error();
			}
			g_kbd_reg3_16bit = ((g_adb_cmd_data[0] & 0xf) << 12) +
				(g_kbd_reg3_16bit & 0x0fff);
		} else if(dev == g_mouse_dev_addr) {
			if(g_adb_cmd_data[0] != (word32)dev) {
				/* see if app is trying to change mouse addr */
				printf("MOUS listen to dev %x reg3: 0:%02x!\n",
					dev, g_adb_cmd_data[0]);
				adb_error();
			}
			if(g_adb_cmd_data[1] != 1 && g_adb_cmd_data[1] != 2) {
				/* see what new device handler id is */
				printf("MOUS listen to dev %x reg 3: 1:%02x\n",
					dev, g_adb_cmd_data[1]);
				adb_error();
			}
		} else {
			printf("Listen cmd to dev %x reg3????\n", dev);
			printf("data0: %02x, data1: %02x ????\n",
				g_adb_cmd_data[0], g_adb_cmd_data[1]);
			adb_error();
		}
		break;
	default:
		printf("Doing adb_cmd %02x: UNKNOWN!\n", g_adb_cmd);
		break;
	}
}


int
adb_read_c027()
{
	word32	ret;

	if(halt_on_all_c027) {
		halt_printf("halting on all c027 reads!\n");
	}

	if(g_c027_val & (~ADB_C027_NEG_MASK)) {
		halt_printf("read_c027: g_c027_val: %02x\n", g_c027_val);
	}

	ret = (g_c027_val & ADB_C027_NEG_MASK);

	if(g_adb_mouse_valid_data) {
		ret |= ADB_C027_MOUSE_DATA;
	}

	if(g_adb_interrupt_byte != 0) {
		ret |= ADB_C027_DATA_VALID;
	} else if(g_adb_data_pending > 0) {
		if((g_adb_state != ADB_IN_CMD)) {
			ret |= ADB_C027_DATA_VALID;
		}
	}

	if(g_adb_mouse_coord) {
		ret |= ADB_C027_MOUSE_COORD;
	}

#if 0
	adb_printf("Read c027: %02x, int_byte: %02x, d_pend: %d\n",
		ret, g_adb_interrupt_byte, g_adb_data_pending);
#endif

#if 0
	adb_log(0xc027, ret);
#endif
	return ret;
}

void
adb_write_c027(int val)
{
	word32	old_val;
	word32	new_int;
	word32	old_int;

	adb_printf("Writing c027 with %02x\n", val);
	adb_log(0x1c027, val);


	old_val = g_c027_val;

	g_c027_val = (val & ADB_C027_NEG_MASK);
	new_int = g_c027_val & ADB_C027_MOUSE_INT;
	old_int = old_val & ADB_C027_MOUSE_INT;
	if(!new_int && old_int) {
		adb_clear_mouse_int();
	}

	new_int = g_c027_val & ADB_C027_DATA_INT;
	old_int = old_val & ADB_C027_DATA_INT;
	if(!new_int && old_int) {
		/* ints were on, now off */
		adb_clear_data_int();
	}

	if(g_c027_val & ADB_C027_KBD_INT) {
		halt_printf("Can't support kbd interrupts!\n");
	}

	return;
}

int
read_adb_ram(word32 addr)
{
	int val;

	adb_printf("Reading adb ram addr: %02x\n", addr);

	if(addr >= 0x100) {
		if(addr >= 0x1000 && addr < 0x2000) {
			/* ROM self-test checksum */
			if(addr == 0x1400) {
				val = 0x72;
			} else if(addr == 0x1401) {
				val = 0xf7;
			} else {
				val = 0;
			}
		} else {
			printf("adb ram addr out of range: %04x!\n", addr);
			val = 0;
		}
	} else {
		val = adb_memory[addr];
		if((addr == 0xb) && (g_rom_version == 1)) {
			// read special key state byte for Out of This World
			val = (g_c025_val >> 1) & 0x43;
			val |= (g_c025_val << 2) & 0x4;
			val |= (g_c025_val >> 2) & 0x10;
		}
		if((addr == 0xc) && (g_rom_version >= 3)) {
			// read special key state byte for Out of This World
			val = g_c025_val & 0xc7;
			printf("val is %02x\n", val);
		}
	}

	adb_printf("adb_ram returning %02x\n", val);
	return val;
}

void
write_adb_ram(word32 addr, int val)
{

	adb_printf("Writing adb_ram addr: %02x: %02x\n", addr, val);

	if(addr >= 0x100) {
		printf("write adb_ram addr: %02x: %02x!\n", addr, val);
		adb_error();
	} else {
		adb_memory[addr] = val;
	}
}

int
adb_get_keypad_xy(int get_y)
{
	int	x, y;
	int	key;
	int	num_keys;
	int	i, j;

	key = 1;
	num_keys = 0;
	x = 0;
	y = 0;
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			if(g_keypad_key_is_down[key]) {
				num_keys++;	
				x = x + (j - 1)*32768;
				y = y + (1 - i)*32768;
			}
			key++;
		}
	}
	if(num_keys == 0) {
		num_keys = 1;
	}

	adb_printf("get_xy=%d, num_keys: %d, x:%d, y:%d\n", get_y,
							num_keys, x, y);

	if(get_y) {
		return y / num_keys;
	} else {
		return x / num_keys;
	}
}

int
update_mouse(int x, int y, int button_states, int buttons_valid)
{
	double	dcycs;
	int	button1_changed;
	int	mouse_moved;
	int	unhide;
	int	pos;
	int	i;

	dcycs = g_cur_dcycs;

	g_mouse_raw_x = x;
	g_mouse_raw_y = y;

	unhide = 0;
	if(x < 0) {
		x = 0;
		unhide = 1;
	}
	if(x >= 640) {
		x = 639;
		unhide = 1;
	}
	if(y < 0) {
		y = 0;
		unhide = 1;
	}
	if(y >= 400) {
		y = 399;
		unhide = 1;
	}


	g_unhide_pointer = unhide && !g_warp_pointer;

	if(!g_warp_pointer) {
		if(g_hide_pointer && g_unhide_pointer) {
			/* cursor has left a2 window, show it */
			g_hide_pointer = 0;
			x_hide_pointer(0);
		}
		if((g_num_lines_prev_superhires == 200) &&
				(g_num_lines_prev_superhires640 == 0) &&
				((g_slow_memory_ptr[0x19d00] & 0x80) == 0)) {
			// In 320-mode superhires, cut mouse range in half
			x = x >> 1;
		}
		y = y >> 1;
	}

	mouse_compress_fifo(dcycs);

#if 0
	printf("Update Mouse called with buttons:%d x,y:%d,%d, fifo:%d,%d, "
		" a2: %d,%d\n", buttons_valid, x, y,
		g_mouse_fifo[0].x, g_mouse_fifo[0].y,
		g_mouse_a2_x, g_mouse_a2_y);
#endif

	if((buttons_valid < 0) && g_warp_pointer) {
		/* Warping the pointer causes it to jump here...this is not */
		/*  real motion, just update info and get out */
		g_mouse_a2_x += (x - g_mouse_fifo[0].x);
		g_mouse_a2_y += (y - g_mouse_fifo[0].y);
		g_mouse_fifo[0].x = x;
		g_mouse_fifo[0].y = y;
		return 0;
	}

#if 0
	printf("...real move, warp: %d, %d, new x: %d, %d, a2:%d,%d\n",
		g_mouse_warp_x, g_mouse_warp_y, g_mouse_fifo[0].x,
		g_mouse_fifo[0].y, g_mouse_a2_x, g_mouse_a2_y);
#endif

	mouse_moved = (g_mouse_fifo[0].x != x) || (g_mouse_fifo[0].y != y);

	g_mouse_a2_x += g_mouse_warp_x;
	g_mouse_a2_y += g_mouse_warp_y;
	g_mouse_fifo[0].x = x;
	g_mouse_fifo[0].y = y;
	g_mouse_fifo[0].dcycs = dcycs;
	g_mouse_warp_x = 0;
	g_mouse_warp_y = 0;

	button1_changed = (buttons_valid & 1) &&
			((button_states & 1) != (g_mouse_fifo[0].buttons & 1));

	if((button_states & 4) && !(g_mouse_fifo[0].buttons & 4) &&
							(buttons_valid & 4)) {
		/* right button pressed */
		adb_increment_speed();
	}
	if((button_states & 2) && !(g_mouse_fifo[0].buttons & 2) &&
							(buttons_valid & 2)) {
		/* middle button pressed */
		halt2_printf("Middle button pressed\n");
	}

	pos = g_mouse_fifo_pos;
	if((pos < (ADB_MOUSE_FIFO - 2)) && button1_changed) {
		/* copy delta to overflow, set overflow */
		/* overflow ensures the mouse button state is precise at */
		/*  button up/down times.  Using a mouse event list where */
		/*  deltas accumulate until a button change would work, too */
		for(i = pos; i >= 0; i--) {
			g_mouse_fifo[i + 1] = g_mouse_fifo[i];	/* copy struct*/
		}
		g_mouse_fifo_pos = pos + 1;
	}

	g_mouse_fifo[0].buttons = (button_states & buttons_valid) |
				(g_mouse_fifo[0].buttons & ~buttons_valid);

	if(mouse_moved || button1_changed) {
		if( (g_mouse_ctl_addr == g_mouse_dev_addr) &&
						((g_adb_mode & 0x2) == 0)) {
			g_adb_mouse_valid_data = 1;
			adb_add_mouse_int();
		}
	}

	return mouse_moved;
}

int
mouse_read_c024(double dcycs)
{
	word32	ret;
	word32	tool_start;
	int	em_active;
	int	target_x, target_y;
	int	delta_x, delta_y;
	int	a2_x, a2_y;
	int	mouse_button;
	int	clamped;
	int	pos;

	if(((g_adb_mode & 0x2) != 0) || (g_mouse_dev_addr != g_mouse_ctl_addr)){
		/* mouse is off, return 0, or mouse is not autopoll */
		g_adb_mouse_valid_data = 0;
		adb_clear_mouse_int();
		return 0;
	}

	mouse_compress_fifo(dcycs);

	pos = g_mouse_fifo_pos;
	target_x = g_mouse_fifo[pos].x;
	target_y = g_mouse_fifo[pos].y;
	mouse_button = (g_mouse_fifo[pos].buttons & 1);
	delta_x = target_x - g_mouse_a2_x;
	delta_y = target_y - g_mouse_a2_y;

	clamped = 0;
	if(delta_x > 0x3f) {
		delta_x = 0x3f;
		clamped = 1;
	} else if(delta_x < -0x3f) {
		delta_x = -0x3f;
		clamped = 1;
	}
	if(delta_y > 0x3f) {
		delta_y = 0x3f;
		clamped = 1;
	} else if(delta_y < -0x3f) {
		delta_y = -0x3f;
		clamped = 1;
	}

	if(pos > 0) {
		/* peek into next entry's button info if we are not clamped */
		/*  and we're returning the y-coord */
		if(!clamped && g_adb_mouse_coord) {
			mouse_button = g_mouse_fifo[pos - 1].buttons & 1;
		}
	}

	if(g_adb_mouse_coord) {
		/* y coord */
		delta_x = 0;	/* clear unneeded x delta */
	} else {
		delta_y = 0;	/* clear unneeded y delta */
	}


	adb_printf(" pre a2_x:%02x,%02x,%02x,%02x\n",
		g_slow_memory_ptr[0x100e9], g_slow_memory_ptr[0x100ea],
		g_slow_memory_ptr[0x100eb], g_slow_memory_ptr[0x100ec]);
	adb_printf(" pre a2_x:%02x,%02x,%02x,%02x\n",
		g_slow_memory_ptr[0x10190], g_slow_memory_ptr[0x10192],
		g_slow_memory_ptr[0x10191], g_slow_memory_ptr[0x10193]);

	/* Update event manager internal state */
	tool_start = (g_slow_memory_ptr[0x103ca] << 16) +
			(g_slow_memory_ptr[0x103c9] << 8) +
			g_slow_memory_ptr[0x103c8];

	em_active = 0;
	if((tool_start >= 0x20000) && (tool_start < (g_mem_size_total - 28)) ) {
		/* seems to be valid ptr to addr of mem space for tools */
		/* see if event manager appears to be active */
		em_active = g_memory_ptr[tool_start + 6*4] +
				(g_memory_ptr[tool_start + 6*4 + 1] << 8);
		if(g_warp_pointer) {
			em_active = 0;
		}
	}

	a2_x = g_mouse_a2_x;
	a2_y = g_mouse_a2_y;

	if(em_active) {
		if((!g_hide_pointer) && (g_num_lines_prev_superhires == 200) &&
				!g_unhide_pointer) {
			/* if super-hires and forcing tracking, then hide */
			g_hide_pointer = 1;
			x_hide_pointer(1);
		}
		if(g_adb_mouse_coord == 0) {
			/* update x coord values */
			g_slow_memory_ptr[0x47c] = a2_x & 0xff;
			g_slow_memory_ptr[0x57c] = a2_x >> 8;
			g_memory_ptr[0x47c] = a2_x & 0xff;
			g_memory_ptr[0x57c] = a2_x >> 8;

			g_slow_memory_ptr[0x10190] = a2_x & 0xff;
			g_slow_memory_ptr[0x10192] = a2_x >> 8;
		} else {
			g_slow_memory_ptr[0x4fc] = a2_y & 0xff;
			g_slow_memory_ptr[0x5fc] = a2_y >> 8;
			g_memory_ptr[0x4fc] = a2_y & 0xff;
			g_memory_ptr[0x5fc] = a2_y >> 8;

			g_slow_memory_ptr[0x10191] = a2_y & 0xff;
			g_slow_memory_ptr[0x10193] = a2_y >> 8;
		}
	} else {
		if(g_hide_pointer && !g_warp_pointer) {
			g_hide_pointer = 0;
			x_hide_pointer(0);
		}
	}

	ret = ((!mouse_button) << 7) + ((delta_x | delta_y) & 0x7f);
	if(g_adb_mouse_coord) {
		g_mouse_a2_button = mouse_button;	/* y coord has button*/
	} else {
		ret |= 0x80;	/* mouse button not down on x coord rd */
	}

	a2_x += delta_x;
	a2_y += delta_y;
	g_mouse_a2_x = a2_x;
	g_mouse_a2_y = a2_y;
	if(g_mouse_fifo_pos) {
		if((target_x == a2_x) && (target_y == a2_y) &&
					(g_mouse_a2_button == mouse_button)) {
			g_mouse_fifo_pos--;
		}
	}


	adb_printf("Read c024, mouse is_y:%d, %02x, vbl:%08x, dcyc:%f, em:%d\n",
		g_adb_mouse_coord, ret, g_vbl_count, dcycs, em_active);
	adb_printf("...mouse targ_x:%d,%d delta_x,y:%d,%d fifo:%d, a2:%d,%d\n",
		target_x, target_y, delta_x, delta_y, g_mouse_fifo_pos,
		a2_x, a2_y);
	adb_printf("   post a2_x:%02x,%02x,%02x,%02x\n",
		g_slow_memory_ptr[0x10190], g_slow_memory_ptr[0x10192],
		g_slow_memory_ptr[0x10191], g_slow_memory_ptr[0x10193]);

	if((g_mouse_fifo_pos == 0) && (g_mouse_fifo[0].x == a2_x) &&
			(g_mouse_fifo[0].y == a2_y) &&
			((g_mouse_fifo[0].buttons & 1) == g_mouse_a2_button)) {
		g_adb_mouse_valid_data = 0;
		adb_clear_mouse_int();
	}

	g_adb_mouse_coord = !g_adb_mouse_coord;
	return ret;
}

void
mouse_compress_fifo(double dcycs)
{
	int	pos;

	/* The mouse fifo exists so that fast button changes don't get lost */
	/*  if the emulator lags behind the mouse events */
	/* But the FIFO means really old mouse events are saved if */
	/*  the emulated code isn't looking at the mouse registers */
	/* This routine compresses all mouse events > 0.5 seconds old */

	for(pos = g_mouse_fifo_pos; pos >= 1; pos--) {
		if(g_mouse_fifo[pos].dcycs < (dcycs - 500*1000.0)) {
			/* Remove this entry */
			adb_printf("Old mouse FIFO pos %d removed\n", pos);
			g_mouse_fifo_pos = pos - 1;
			continue;
		}
		/* Else, stop searching the FIFO */
		break;
	}
}

void
adb_key_event(int a2code, int is_up)
{
	word32	special;
	word32	vbl_count;
	int	key;
	int	hard_key;
	int	pos;
	int	tmp_ascii;
	int	ascii;

	if(is_up) {
		adb_printf("adb_key_event, key:%02x, is up, g_key_down: %02x\n",
			a2code, g_key_down);
	}

	if(a2code < 0 || a2code > 0x7f) {
		halt_printf("add_key_event: a2code: %04x!\n", a2code);
		return;
	}

	if(!is_up && a2code == 0x35) {
		/* ESC pressed, see if ctrl & cmd key down */
		if(CTRL_DOWN && CMD_DOWN) {
			/* Desk mgr int */
			printf("Desk mgr int!\n");

			g_adb_interrupt_byte |= 0x20;
			adb_add_data_int();
		}
	}

	/* convert key to ascii, if possible */
	hard_key = 0;
	if(a2_key_to_ascii[a2code][1] & 0xef00) {
		/* special key */
	} else {
		/* we have ascii */
		hard_key = 1;
	}

	pos = 1;
	ascii = a2_key_to_ascii[a2code][1];
	if(CAPS_LOCK_DOWN && (ascii >= 'a' && ascii <= 'z')) {
		pos = 2;
		if(SHIFT_DOWN && (g_adb_mode & 0x40)) {
			/* xor shift mode--capslock and shift == lowercase */
			pos = 1;
		}
	} else if(SHIFT_DOWN) {
		pos = 2;
	}

	ascii = a2_key_to_ascii[a2code][pos];
	if(CTRL_DOWN) {
		tmp_ascii = a2_key_to_ascii[a2code][3];
		if(tmp_ascii >= 0) {
			ascii = tmp_ascii;
		}
	}
	key = (ascii & 0x7f) + 0x80;

	special = (ascii >> 8) & 0xff;
	if(ascii < 0) {
		printf("ascii1: %d, a2code: %02x, pos: %d\n", ascii,a2code,pos);
		ascii = 0;
		special = 0;
	}


	if(!is_up) {
		if(hard_key) {
			g_kbd_buf[g_kbd_chars_buffered] = key;
			g_kbd_chars_buffered++;
			if(g_kbd_chars_buffered >= MAX_KBD_BUF) {
				g_kbd_chars_buffered = MAX_KBD_BUF - 1;
			}
			g_key_down = 1;
			g_a2code_down = a2code;

			/* first key down, set up autorepeat */
			vbl_count = g_vbl_count;
			if(g_config_control_panel) {
				vbl_count = g_cfg_vbl_count;
			}
			g_adb_repeat_vbl = vbl_count + g_adb_repeat_delay;
			if(g_adb_repeat_delay == 0) {
				g_key_down = 0;
			}
			g_hard_key_down = 1;
		}

		g_c025_val = g_c025_val | special;
		adb_printf("new c025_or: %02x\n", g_c025_val);
	} else {
		if(hard_key && (a2code == g_a2code_down)) {
			g_hard_key_down = 0;
			/* Turn off repeat */
			g_key_down = 0;
		}

		g_c025_val = g_c025_val & (~ special);
		adb_printf("new c025_and: %02x\n", g_c025_val);
	}

	if(g_key_down) {
		g_c025_val = g_c025_val & (~0x20);
	} else {
		/* If no hard key down, set update mod latch */
		g_c025_val = g_c025_val | 0x20;
	}

}

word32
adb_read_c000()
{
	word32	vbl_count;

	if( ((g_kbd_buf[0] & 0x80) == 0) && (g_key_down == 0)) {
		/* nothing happening, just get out */
		return g_kbd_buf[0];
	}
	if(g_kbd_buf[0] & 0x80) {
		/* got one */
		if((g_kbd_read_no_update++ > 5) && (g_kbd_chars_buffered > 1)) {
			/* read 5 times, keys pending, let's move it along */
			printf("Read %02x %d times, tossing\n", g_kbd_buf[0],
					g_kbd_read_no_update);
			adb_access_c010();
		}
	} else {
		vbl_count = g_vbl_count;
		if(g_config_control_panel) {
			vbl_count = g_cfg_vbl_count;
		}
		if(g_key_down && vbl_count >= g_adb_repeat_vbl) {
			/* repeat the g_key_down */
			g_c025_val |= 0x8;
			adb_key_event(g_a2code_down, 0);
			g_adb_repeat_vbl = vbl_count + g_adb_repeat_rate;
		}
	}

	return g_kbd_buf[0];
}

word32
adb_access_c010()
{
	int	tmp;
	int	i;

	g_kbd_read_no_update = 0;

	tmp = g_kbd_buf[0] & 0x7f;
	g_kbd_buf[0] = tmp;

	tmp = tmp | (g_hard_key_down << 7);
	if(g_kbd_chars_buffered) {
		for(i = 1; i < g_kbd_chars_buffered; i++) {
			g_kbd_buf[i - 1] = g_kbd_buf[i];
		}
		g_kbd_chars_buffered--;
	}

	g_c025_val = g_c025_val & (~ (0x08));

	return tmp;
}

word32
adb_read_c025()
{
	return	g_c025_val;
}

int
adb_is_cmd_key_down()
{
	return	CMD_DOWN;
}

int
adb_is_option_key_down()
{
	return	OPTION_DOWN;
}

void
adb_increment_speed()
{
	const char *str;

	g_limit_speed++;
	if(g_limit_speed > 3) {
		g_limit_speed = 0;
	}

	str = "";
	switch(g_limit_speed) {
	case 0:
		str = "...as fast as possible!";
		break;
	case 1:
		str = "...1.024MHz!";
		break;
	case 2:
		str = "...2.8MHz!";
		break;
	case 3:
		str = "...8.0MHz!";
		break;
	}
	printf("Toggling g_limit_speed to %d%s\n", g_limit_speed, str);
}

void
adb_physical_key_update(int a2code, int is_up)
{
	int	autopoll;
	int	special;
	int	ascii_and_type;
	int	ascii;

	/* this routine called by xdriver to pass raw codes--handle */
	/*  ucontroller and ADB bus protocol issues here */
	/* if autopoll on, pass it on through to c025,c000 regs */
	/*  else only put it in kbd reg 3, and pull SRQ if needed */

	adb_printf("adb_phys_key_update: %02x, %d\n", a2code, is_up);

	adb_printf("Handle a2code: %02x, is_up: %d\n", a2code, is_up);

	if(a2code < 0 || a2code > 0x7f) {
		halt_printf("a2code: %04x!\n", a2code);
		return;
	}

	/* Remap 0x7b-0x7e to 0x3b-0x3e (arrow keys on new mac keyboards) */
	if(a2code >= 0x7b && a2code <= 0x7e) {
		a2code = a2code - 0x40;
	}

	/* Now check for special keys (function keys, etc) */
	ascii_and_type = a2_key_to_ascii[a2code][1];
	special = 0;
	if((ascii_and_type & 0xf000) == 0x8000) {
		/* special function key */
		special = ascii_and_type & 0xff;
		switch(special) {
		case 0x01: /* F1 - remap to cmd */
			a2code = 0x37;
			special = 0;
			break;
		case 0x02: /* F2 - remap to option */
			a2code = 0x3a;
			special = 0;
			break;
		case 0x03: /* F3 - remap to escape for OS/2 */
			a2code = 0x35;
			special = 0;
			break;
		case 0x0c: /* F12 - remap to reset */
			a2code = 0x7f;
			special = 0;
			break;
		default:
			break;
		}
	}

	/* Only process reset requests here */
	if(is_up == 0 && a2code == 0x7f && CTRL_DOWN) {
		/* Reset pressed! */
		printf("Reset pressed since CTRL_DOWN: %d\n", CTRL_DOWN);
		do_reset();
		return;
	}

	if(special && !is_up) {
		switch(special) {
		case 0x04: /* F4 - Emulator config panel */
			cfg_toggle_config_panel();
			break;
		case 0x06: /* F6 - emulator speed */
			if(SHIFT_DOWN) {
				halt2_printf("Shift-F6 pressed\n");
			} else {
				adb_increment_speed();
			}
			break;
		case 0x07: /* F7 - fast disk emul */
			g_fast_disk_emul = !g_fast_disk_emul;
			printf("g_fast_disk_emul is now %d\n",
							g_fast_disk_emul);
			break;
		case 0x08: /* F8 - warp pointer */
			g_warp_pointer = !g_warp_pointer;
			if(g_hide_pointer != g_warp_pointer) {
				g_hide_pointer = g_warp_pointer;
				x_hide_pointer(g_hide_pointer);
			}
			break;
		case 0x09: /* F9 - swap paddles */
			if(SHIFT_DOWN) {
				g_swap_paddles = !g_swap_paddles;
				printf("Swap paddles is now: %d\n",
							g_swap_paddles);
			} else {
				g_invert_paddles = !g_invert_paddles;
				printf("Invert paddles is now: %d\n",
							g_invert_paddles);
			}
			break;
		case 0x0a: /* F10 - change a2vid paletter */
			change_a2vid_palette((g_a2vid_palette + 1) & 0xf);
			break;
		case 0x0b: /* F11 - full screen */
			g_fullscreen = !g_fullscreen;
			x_full_screen(g_fullscreen);
			break;
		}

		return;
	}
	/* Handle Keypad Joystick here partly...if keypad key pressed */
	/*  while in Keypad Joystick mode, do not pass it on as a key press */
	if((ascii_and_type & 0xff00) == 0x1000) {
		/* Keep track of keypad number keys being up or down even */
		/*  if joystick mode isn't keypad.  This avoid funny cases */
		/*  if joystick mode is changed while a key is pressed */
		ascii = ascii_and_type & 0xff;
		if(ascii > 0x30 && ascii <= 0x39) {
			g_keypad_key_is_down[ascii - 0x30] = !is_up;
		}
		if(g_joystick_type == 0) {
			/* If Joystick type is keypad, then do not let these */
			/*  keypress pass on further, except for cmd/opt */
			if(ascii == 0x30) {
				/* remap '0' to cmd */
				a2code = 0x37;
			} else if(ascii == 0x2e || ascii == 0x2c) {
				/* remap '.' and ',' to option */
				a2code = 0x3a;
			} else {
				/* Just ignore it in this mode */
				return;
			}
		}
	}

	autopoll = 1;
	if(g_adb_mode & 1) {
		/* autopoll is explicitly off */
		autopoll = 0;
	}
	if(g_kbd_dev_addr != g_kbd_ctl_addr) {
		/* autopoll is off because ucontroller doesn't know kbd moved */
		autopoll = 0;
	}
	if(g_config_control_panel) {
		/* always do autopoll */
		autopoll = 1;
	}


	if(is_up) {
		if(!autopoll) {
			/* no auto keys, generate SRQ! */
			adb_kbd_reg0_data(a2code, is_up);
		} else {
			adb_virtual_key_update(a2code, is_up);
		}
	} else {
		if(!autopoll) {
			/* no auto keys, generate SRQ! */
			adb_kbd_reg0_data(a2code, is_up);
		} else {
			/* was up, now down */
			adb_virtual_key_update(a2code, is_up);
		}
	}
}

void
adb_virtual_key_update(int a2code, int is_up)
{
	int	i;
	int	bitpos;
	word32	mask;

	adb_printf("Virtual handle a2code: %02x, is_up: %d\n", a2code, is_up);

	if(a2code < 0 || a2code > 0x7f) {
		halt_printf("a2code: %04x!\n", a2code);
		return;
	}

	i = (a2code >> 5) & 3;
	bitpos = a2code & 0x1f;
	mask = (1 << bitpos);

	if(is_up) {
		if(g_virtual_key_up[i] & mask) {
			/* already up, do nothing */
		} else {
			g_virtual_key_up[i] |= mask;
			adb_key_event(a2code, is_up);
		}
	} else {
		if(g_virtual_key_up[i] & mask) {
			g_virtual_key_up[i] &= (~mask);
			adb_key_event(a2code, is_up);
		}
	}
}

void
adb_all_keys_up()
{
	word32	mask;
	int	i, j;

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 32; j++) {
			mask = 1 << j;
			if((g_virtual_key_up[i] & mask) == 0) {
				/* create key-up event */
				adb_physical_key_update(i*32 + j, 1);
			}
		}
	}
}

void
adb_kbd_repeat_off()
{
	g_key_down = 0;
}
