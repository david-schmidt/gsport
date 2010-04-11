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

	.data
	.export rcsid_engine_s_s,data
rcsid_engine_s_s
	.stringz "@(#)$KmKId: engine_s.s,v 1.155 2003-11-25 22:06:48-05 kentd Exp $"

	.code

	.level	1.1

#define INCLUDE_RCSID_S
#include "defs.h"
#undef INCLUDE_RCSID_S

#define ASM


#if 0
# define LOG_PC
# define ACCURATE_SLOW_MEM
# define DEBUG_TOOLBOX
#endif


/*
#define COUNT_GET_CALLS
*/

#if 0
# define CHECK_SIZE_CONSISTENCY
#endif


#define STACK_ENGINE_SIZE			512

#define STACK_SAVE_CMP_INDEX_REG		-64
#define STACK_GET_MEM_B0_DIRECT_SAVELINK	-68
#define STACK_SAVE_ARG0				-72
#define STACK_SAVE_INSTR			-76
#define STACK_SRC_BANK				-80
#define STACK_INST_TAB_PTR_DONT_USE_THIS	-84
#if 0
#define STACK_BP_ARG0_SAVE			-88
#define STACK_BP_ARG1_SAVE			-92
#define STACK_BP_ARG2_SAVE			-96
#define STACK_BP_ARG3_SAVE			-100
#define STACK_BP_RP_SAVE			-104
#define STACK_BP_SCRATCH4_SAVE			-108
#endif

#define STACK_GET_MEMORY_SAVE_LINK		-112
#define STACK_SET_MEMORY_SAVE_LINK		-116

#define STACK_MEMORY16_SAVE1			-120
#define STACK_MEMORY16_SAVE2			-124
#define STACK_MEMORY16_SAVE3			-128

#define STACK_SAVE_CYCLES_WORD2			-132	/* Cycles = dword */
#define STACK_SAVE_CYCLES			-136

#define STACK_SET_MEMORY24_SAVE1		-140
#define STACK_SET_MEMORY24_SAVE2		-144
#define STACK_SET_MEMORY24_SAVE3		-148

#define STACK_GET_MEMORY24_SAVE1		-152
#define STACK_GET_MEMORY24_SAVE2		-156
#define STACK_GET_MEMORY24_SAVE3		-160

/* #define STACK_SAVE_INIT_CYCLES		-164 */
#define STACK_SAVE_DECIMAL16_A			-168
#define STACK_SAVE_DECIMAL16_B			-172
#define STACK_SAVE_INSTR_TMP1			-176

#define STACK_SAVE_DISPATCH_LINK		-180
#define STACK_SAVE_OP_LINK			-184
#define STACK_GET_MEMORY16_ADDR_LATCH		-188
#define STACK_GET_MEMORY24_ADDR_LATCH		-192

#define STACK_GET_MEM_B0_DIRECT_ARG0		-200
#define STACK_GET_MEM_B0_DIRECT_RET0		-204
#define STACK_SAVE_PUSH16_LINK			-208
#define STACK_SAVE_PUSH16_ARG1			-212

#define STACK_SAVE_PULL16_LINK			-216
#define STACK_SAVE_PULL16_RET0			-220
#define STACK_SAVE_PULL24_LINK			-224
#define STACK_SAVE_PULL24_RET0			-228

#define STACK_SAVE_COP_ARG0			-232
#define STACK_SAVE_TMP_INST0			-236
#define STACK_SAVE_TMP_INST			-240
#define STACK_SAVE_TMP_INST1			-244
#define STACK_SAVE_DISP_PIECES_LINK		-248
#define STACK_SAVE_DISPATCH_SCRATCH1		-252

#if 0
#define STACK_BP_SCRATCH2_SAVE			-256
#define STACK_BP_SCRATCH3_SAVE			-260
#endif



#define	CYCLES_PLUS_1		fadd,dbl fr_plus_1,fcycles,fcycles
#define	CYCLES_PLUS_2		fadd,dbl fr_plus_2,fcycles,fcycles
#define	CYCLES_PLUS_3		fadd,dbl fr_plus_3,fcycles,fcycles
#define	CYCLES_PLUS_5		fadd,dbl fr_plus_3,fcycles,fcycles ! \
				fadd,dbl fr_plus_2,fcycles,fcycles
				
#define	CYCLES_MINUS_1		fsub,dbl fcycles,fr_plus_1,fcycles
#define	CYCLES_MINUS_2		fsub,dbl fcycles,fr_plus_2,fcycles

#define CYCLES_FINISH		fadd,dbl fcycles_stop,fr_plus_1,fcycles

#define FCYCLES_ROUND_1		fadd,dbl fcycles,fr_plus_x_m1,ftmp1
#define FCYCLES_ROUND_2		fcnvfxt,dbl,dbl ftmp1,ftmp1
#define FCYCLES_ROUND_3		fcnvxf,dbl,dbl ftmp1,fcycles

/* HACK: INC_KPC* and DEC_KPC2 should avoid overflow into kbank! */
#define INC_KPC_1		addi	1,kpc,kpc
#define INC_KPC_2		addi	2,kpc,kpc
#define INC_KPC_3		addi	3,kpc,kpc
#define INC_KPC_4		addi	4,kpc,kpc
#define DEC_KPC2		addi	-2,kpc,kpc

#define get_mem_b0_8	get_memory_asm
#define get_mem_b0_16	get_memory16_asm
#define get_mem_b0_24	get_memory24_asm

#define get_mem_long_8	get_memory_asm
#define get_mem_long_16	get_memory16_asm
#define get_mem_long_24	get_memory24_asm

#define set_mem_long_8	set_memory_asm
#define set_mem_long_16	set_memory16_asm

#define set_mem_b0_8	set_memory_asm
#define set_mem_b0_16	set_memory16_asm
#define set_mem_b0_24	set_memory24_asm

	.code
	.import halt_sim,data
	.import g_fcycles_stop,data
	.import g_irq_pending,data
	.import g_wait_pending,data
	.import g_rom_version,data
	.import g_num_brk,data
	.import g_num_cop,data
	.import g_testing,data

	.import log_pc,code
	.import toolbox_debug_c,code

	.import	get_memory_io,code

	.import	set_memory_io,code
	.import	set_memory16_pieces,code
	.import	set_memory24_pieces,code

#define INCLUDE_RCSID_S
#include "op_routs.h"
#undef INCLUDE_RCSID_S

	.import do_break,code
	.import do_cop,code
	.import	page_info_rd_wr,data
	.import	get_memory_calls,data
	.import	slow_mem_changed,data
	.import	g_cur_dcycs,data
	.import	g_last_vbl_dcycs,data
	.import	g_slow_memory_ptr,data
	.import	g_memory_ptr,data
	.import	g_dummy_memory1_ptr,data
	.import	g_rom_fc_ff_ptr,data
	.import	g_rom_cards_ptr,data

	.export	fixed_memory_ptrs_init,code
fixed_memory_ptrs_init
	LDC(slow_memory,arg0)
	LDC(g_slow_memory_ptr,arg1)
	stw	arg0,(arg1)

	LDC(dummy_memory1,arg0)
	LDC(g_dummy_memory1_ptr,arg1)
	stw	arg0,(arg1)

	LDC(rom_fc_ff,arg0)
	LDC(g_rom_fc_ff_ptr,arg1)
	stw	arg0,(arg1)

	LDC(rom_cards,arg0)
	LDC(g_rom_cards_ptr,arg1)
	stw	arg0,(arg1)

	bv	0(link)
	nop

	.export get_itimer,code
get_itimer
	bv	0(link)
	mfctl	%cr16,ret0

	.export enter_asm,data
enter_asm
	stwm	page_info_ptr,STACK_ENGINE_SIZE(sp)
	stw	link,-STACK_ENGINE_SIZE+4(sp)
	ldo	-STACK_ENGINE_SIZE+16(sp),scratch2
	stw	addr_latch,-STACK_ENGINE_SIZE+8(sp)
	fstds,ma fcycles,8(scratch2)
	fstds,ma fr_plus_1,8(scratch2)
	fcpy,dbl 0,fcycles
	fstds,ma fr_plus_2,8(scratch2)
	fstds,ma fr_plus_3,8(scratch2)
	fcpy,dbl 0,fr_plus_1
	fstds,ma fr_plus_x_m1,8(scratch2)
	fstds,ma fcycles_stop,8(scratch2)
	fcpy,dbl 0,fr_plus_2
	fstds,ma fcycles_last_dcycs,8(scratch2)
	ldil	l%g_cur_dcycs,scratch2

	ldil	l%g_last_vbl_dcycs,scratch3
	fcpy,dbl 0,fr_plus_3
	ldo	r%g_cur_dcycs(scratch2),scratch2
	fcpy,dbl 0,fr_plus_x_m1
	ldo	r%g_last_vbl_dcycs(scratch3),scratch3
	fldds	0(scratch2),ftmp1
	ldil	l%page_info_rd_wr,page_info_ptr
	fldds	0(scratch3),fcycles_last_dcycs
	fcpy,dbl 0,fcycles_stop
	ldo	r%page_info_rd_wr(page_info_ptr),page_info_ptr
	bv	0(scratch1)
	fsub,dbl ftmp1,fcycles_last_dcycs,fcycles


	.export leave_asm,data
leave_asm
	ldw	-STACK_ENGINE_SIZE+4(sp),link
	ldo	-STACK_ENGINE_SIZE+16(sp),scratch2
	ldw	-STACK_ENGINE_SIZE+8(sp),addr_latch
	fldds,ma 8(scratch2),fcycles
	fldds,ma 8(scratch2),fr_plus_1
	fldds,ma 8(scratch2),fr_plus_2
	fldds,ma 8(scratch2),fr_plus_3
	fldds,ma 8(scratch2),fr_plus_x_m1
	fldds,ma 8(scratch2),fcycles_stop
	fldds,ma 8(scratch2),fcycles_last_dcycs

	bv	(link)
	ldwm	-STACK_ENGINE_SIZE(sp),page_info_ptr


	.align	8
	.export get_memory_c
get_memory_c
; arg0 = addr
; arg1 = cycles
	bl	enter_asm,scratch1
	nop
	bl	get_memory_asm,link
	nop
	b	leave_asm
	nop

	.export get_memory16_c
get_memory16_c
; arg0 = addr
; arg1 = cycles
	bl	enter_asm,scratch1
	nop
	bl	get_memory16_asm,link
	nop
	b	leave_asm
	nop

	.export get_memory24_c
get_memory24_c
; arg0 = addr
; arg1 = cycles
	bl	enter_asm,scratch1
	nop
	bl	get_memory24_asm,link
	nop
	b	leave_asm
	nop

#define GET_MEM8(upper16,lower8,ret0)		\
	extru	arg0,23,16,arg3			! \
	CYCLES_PLUS_1				! \
	ldwx,s	arg3(page_info_ptr),scratch3	! \
	copy	arg0,addr_latch			! \
	copy	scratch3,scratch2		! \
	dep	arg0,31,8,scratch3		! \
	extru,=	scratch2,BANK_IO_BIT,1,0	! \
	bl,n	get_memory_iocheck_stub_asm,link ! \
	ldb	(scratch3),ret0

	.align	32

	.export get_memory_asm
get_memory_asm
; arg0 = addr
	extru	arg0,23,16,arg3
	copy	arg0,addr_latch

	ldwx,s	arg3(page_info_ptr),scratch2
	CYCLES_PLUS_1
	bb,<,n	scratch2,BANK_IO_BIT,get_memory_iocheck_stub_asm
	dep	arg0,31,8,scratch2
	bv	0(link)
	ldb	(scratch2),ret0


	.align	8

	.export	get_memory16_asm
get_memory16_asm
; arg0 = addr
	ldi	0xff,scratch3
	extru	arg0,23,16,arg3

	and	scratch3,arg0,scratch4
	ldwx,s	arg3(page_info_ptr),scratch2
	copy	arg0,addr_latch
	comb,=	scratch4,scratch3,get_memory16_pieces_stub_asm
	and	scratch2,scratch3,scratch3
	comb,<>	0,scratch3,get_memory16_pieces_stub_asm
	dep	arg0,31,8,scratch2
	ldb	(scratch2),ret0
	CYCLES_PLUS_2
	ldb	1(scratch2),scratch1
	bv	0(link)
	dep	scratch1,23,8,ret0

	.align	8

	.export	get_memory24_asm
get_memory24_asm
; arg0 = addr
	ldi	0xfe,scratch3
	extru	arg0,23,16,arg3

	and	scratch3,arg0,scratch4
	ldwx,s	arg3(page_info_ptr),scratch2
	copy	arg0,addr_latch
	comb,=	scratch4,scratch3,get_memory24_pieces_stub_asm
	extru	scratch2,31,8,scratch3
	comb,<>	0,scratch3,get_memory24_pieces_stub_asm
	dep	arg0,31,8,scratch2
	ldb	(scratch2),ret0
	ldb	1(scratch2),scratch1
	CYCLES_PLUS_3
	ldb	2(scratch2),scratch2
	dep	scratch1,23,8,ret0
	bv	0(link)
	dep	scratch2,15,8,ret0


	.align	0x20
	.export get_memory_iocheck_stub_asm,code
get_memory_iocheck_stub_asm
	extru,=	scratch2,BANK_BREAK_BIT,1,0
	bl	check_breakpoints_asm,scratch4
	stw	link,STACK_GET_MEMORY_SAVE_LINK(sp)
	bb,<	scratch2,BANK_IO2_BIT,get_memory_io_stub_asm
	dep	arg0,31,8,scratch2
	bv	0(link)
	ldb	(scratch2),ret0

	.export get_memory_io_stub_asm
get_memory_io_stub_asm
	FCYCLES_ROUND_1
	ldo	STACK_SAVE_CYCLES(sp),arg1
	FCYCLES_ROUND_2
	FCYCLES_ROUND_3
	bl	get_memory_io,link
	fstds	fcycles,(arg1)

	ldw	STACK_GET_MEMORY_SAVE_LINK(sp),link
	ldo	STACK_SAVE_CYCLES(sp),arg1
	bv	(link)
	fldds	(arg1),fcycles



	.export get_memory16_pieces_stub_asm,code
get_memory16_pieces_stub_asm
	stw	addr_latch,STACK_GET_MEMORY16_ADDR_LATCH(sp)
	addi	1,arg0,scratch1
	stw	link,STACK_MEMORY16_SAVE2(sp)
	bl	get_memory_asm,link
	stw	scratch1,STACK_MEMORY16_SAVE1(sp)

	stw	ret0,STACK_MEMORY16_SAVE3(sp)
	bl	get_memory_asm,link
	ldw	STACK_MEMORY16_SAVE1(sp),arg0

	ldw	STACK_MEMORY16_SAVE2(sp),link
	copy	ret0,scratch1
	ldw	STACK_MEMORY16_SAVE3(sp),ret0
	ldw	STACK_GET_MEMORY16_ADDR_LATCH(sp),addr_latch
	bv	(link)
	dep	scratch1,23,8,ret0


	.export get_memory24_pieces_stub_asm,code
get_memory24_pieces_stub_asm
	stw	addr_latch,STACK_GET_MEMORY16_ADDR_LATCH(sp)
	addi	1,arg0,scratch1
	stw	link,STACK_GET_MEMORY24_SAVE2(sp)
	bl	get_memory_asm,link
	stw	scratch1,STACK_GET_MEMORY24_SAVE1(sp)

	stw	ret0,STACK_GET_MEMORY24_SAVE3(sp)
	bl	get_memory_asm,link
	ldw	STACK_GET_MEMORY24_SAVE1(sp),arg0

	ldw	STACK_GET_MEMORY24_SAVE1(sp),arg0
	stb	ret0,STACK_GET_MEMORY24_SAVE3+2(sp)
	bl	get_memory_asm,link
	addi	1,arg0,arg0
	
	ldw	STACK_GET_MEMORY24_SAVE2(sp),link
	copy	ret0,scratch1
	ldw	STACK_GET_MEMORY24_SAVE3(sp),ret0
	ldw	STACK_GET_MEMORY16_ADDR_LATCH(sp),addr_latch
	bv	(link)
	dep	scratch1,15,8,ret0




; C callable routine to wrap around set_memory_asm
	.export set_memory_c
set_memory_c
;arg0 = addr
;arg1 = val
;arg2 = cycles
	bl	enter_asm,scratch1
	nop
	bl	set_memory_asm,link
	nop
	b	leave_asm
	nop


	.export set_memory16_c
set_memory16_c
;arg0 = addr
;arg1 = val
;arg2 = cycles
	bl	enter_asm,scratch1
	nop
	bl	set_memory16_asm,link
	nop
	b	leave_asm
	nop

	.export set_memory24_c
set_memory24_c
;arg0 = addr
;arg1 = val
;arg2 = cycles
	bl	enter_asm,scratch1
	nop
	bl	set_memory24_asm,link
	nop
	b	leave_asm
	nop


	.align	32

	.export set_memory_asm
set_memory_asm
; arg0 = addr
; arg1 = val
	extru	arg0,23,16,arg3
	addil	l%PAGE_INFO_WR_OFFSET,arg3
	CYCLES_PLUS_1
	ldwx,s	r1(page_info_ptr),scratch2
	ldi	0xff,scratch3
	and	scratch2,scratch3,scratch3
	dep	arg0,31,8,scratch2
	comib,<>,n 0,scratch3,set_memory_special_case
set_memory_cont_asm
	bv	0(link)
	stb	arg1,(scratch2)


	.export set_memory_special_case
set_memory_special_case
	extru,=	scratch3,BANK_BREAK_BIT,1,0
	bl	check_breakpoints_asm,scratch4
	extru	arg1,31,8,arg1

set_memory_special_case2
	bb,<	scratch3,BANK_IO2_BIT,set_memory_io_stub_asm
	ldil	l%slow_memory,scratch4
	bb,<	scratch3,BANK_SHADOW_BIT,set_memory_shadow1_asm
	extru	arg0,31,16,arg3
	bb,<	scratch3,BANK_SHADOW2_BIT,set_memory_shadow2_asm
	nop
	bb,<	scratch3,BANK_BREAK_BIT,set_memory_cont_asm
	nop
	break


set_memory_shadow1_asm
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_1
#endif
	add	arg3,scratch4,scratch4
	extru	arg3,31-SHIFT_PER_CHANGE,5,scratch1
	ldb	r%slow_memory(scratch4),arg2
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_2
#endif
	mtctl	scratch1,cr11
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_3
#endif
	comclr,<> arg2,arg1,0
	bv	0(link)
	stb	arg1,(scratch2)
	zvdepi	1,1,arg2
	extru	arg3,31-CHANGE_SHIFT,16-CHANGE_SHIFT,scratch2
	ldil	l%slow_mem_changed,scratch1
	sh2add	scratch2,scratch1,scratch1
	ldw	r%slow_mem_changed(scratch1),scratch3
	stb	arg1,r%slow_memory(scratch4)
	or	arg2,scratch3,scratch3
	bv	0(link)
	stw	scratch3,r%slow_mem_changed(scratch1)

set_memory_shadow2_asm
	depi	1,15,1,arg3
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_1
#endif
	add	arg3,scratch4,scratch4
	extru	arg3,31-SHIFT_PER_CHANGE,5,scratch1
	ldb	r%slow_memory(scratch4),arg2
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_2
#endif
	mtctl	scratch1,cr11
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_3
#endif
	comclr,<> arg2,arg1,0
	bv	0(link)
	stb	arg1,(scratch2)
	zvdepi	1,1,arg2
	extru	arg3,31-CHANGE_SHIFT,16-CHANGE_SHIFT,scratch2
	ldil	l%slow_mem_changed,scratch1
	sh2add	scratch2,scratch1,scratch1
	ldw	r%slow_mem_changed(scratch1),scratch3
	stb	arg1,r%slow_memory(scratch4)
	or	arg2,scratch3,scratch3
	bv	0(link)
	stw	scratch3,r%slow_mem_changed(scratch1)


set_memory_io_stub_asm
	FCYCLES_ROUND_1
	ldo	STACK_SAVE_CYCLES(sp),arg2
	FCYCLES_ROUND_2
	stw	link,STACK_SET_MEMORY_SAVE_LINK(sp)
	FCYCLES_ROUND_3
	bl	set_memory_io,link
	fstds	fcycles,(arg2)

	ldw	STACK_SET_MEMORY_SAVE_LINK(sp),link
	ldo	STACK_SAVE_CYCLES(sp),arg2
	bv	(link)
	fldds	(arg2),fcycles

	.align	8
	.export set_memory16_asm
set_memory16_asm
; arg0 = addr
; arg1 = val
	extru	arg0,23,16,arg3

	addil	l%PAGE_INFO_WR_OFFSET,arg3
	extrs	arg0,31,8,scratch4
	ldwx,s	r1(page_info_ptr),scratch2
	ldi	0xff,scratch3
	and	scratch3,scratch2,scratch3
	dep	arg0,31,8,scratch2
	comib,=,n -1,scratch4,set_memory16_pieces_stub_asm
	comib,<>,n 0,scratch3,set_memory16_special_case
set_memory16_cont_asm
	stb	arg1,0(scratch2)
	CYCLES_PLUS_2
	extru	arg1,23,8,arg3
	bv	0(link)
	stb	arg3,1(scratch2)


	.align	8
set_memory16_shadow1_asm
	CYCLES_PLUS_2
	copy	arg1,arg2
	extru	arg1,23,8,arg1
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_1
#endif
	add	arg3,scratch4,scratch4
	dep	arg2,23,8,arg1
	extru	arg3,31-SHIFT_PER_CHANGE,5,scratch1
	ldh	r%slow_memory(scratch4),arg2
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_2
#endif
	mtctl	scratch1,cr11
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_3
#endif
	comclr,<> arg2,arg1,0		;return if arg2 == arg1
	bv	0(link)
	sth	arg1,(scratch2)
	zvdepi	1,1,arg2
	extru	arg3,31-CHANGE_SHIFT,16-CHANGE_SHIFT,scratch2
	ldil	l%slow_mem_changed,scratch1
	sh2add	scratch2,scratch1,scratch1
	ldw	r%slow_mem_changed(scratch1),scratch3
	sth	arg1,r%slow_memory(scratch4)
	or	arg2,scratch3,scratch3
	bv	0(link)
	stw	scratch3,r%slow_mem_changed(scratch1)

	.align	8
set_memory16_shadow2_asm
	CYCLES_PLUS_2
	copy	arg1,arg2
	extru	arg1,23,8,arg1
	depi	1,15,1,arg3
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_1
#endif
	dep	arg2,23,8,arg1
	add	arg3,scratch4,scratch4
	extru	arg3,31-SHIFT_PER_CHANGE,5,scratch1
	ldh	r%slow_memory(scratch4),arg2
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_2
#endif
	mtctl	scratch1,cr11
#ifdef ACCURATE_SLOW_MEM
	FCYCLES_ROUND_3
#endif
	comclr,<> arg2,arg1,0
	bv	0(link)
	sth	arg1,(scratch2)
	zvdepi	1,1,arg2
	extru	arg3,31-CHANGE_SHIFT,16-CHANGE_SHIFT,scratch2
	ldil	l%slow_mem_changed,scratch1
	sh2add	scratch2,scratch1,scratch1
	ldw	r%slow_mem_changed(scratch1),scratch3
	sth	arg1,r%slow_memory(scratch4)
	or	arg2,scratch3,scratch3
	bv	0(link)
	stw	scratch3,r%slow_mem_changed(scratch1)


	.align	8
set_memory16_special_case
	extru,=	scratch3,BANK_BREAK_BIT,1,0
	bl	check_breakpoints_asm,scratch4
	extru	arg1,31,16,arg1

set_memory16_special_case2
	bb,<	scratch3,BANK_IO2_BIT,set_memory16_pieces_stub_asm
	ldil	l%slow_memory,scratch4

; if not halfword aligned, go through pieces_stub_asm
	bb,<,n	arg0,31,set_memory16_pieces_stub_asm
	bb,<	scratch3,BANK_SHADOW2_BIT,set_memory16_shadow2_asm
	extru	arg0,31,16,arg3
	bb,<	scratch3,BANK_SHADOW_BIT,set_memory16_shadow1_asm
	nop
	bb,<	scratch3,BANK_BREAK_BIT,set_memory16_cont_asm
	nop
	break

	.align	8
set_memory16_pieces_stub_asm
	addi	1,arg0,scratch1
	stw	link,STACK_MEMORY16_SAVE3(sp)
	extru	arg1,23,8,scratch2
	stw	scratch1,STACK_MEMORY16_SAVE1(sp)
	bl	set_memory_asm,link
	stw	scratch2,STACK_MEMORY16_SAVE2(sp)

	ldw	STACK_MEMORY16_SAVE1(sp),arg0
	ldw	STACK_MEMORY16_SAVE2(sp),arg1
	b	set_memory_asm
	ldw	STACK_MEMORY16_SAVE3(sp),link


	.align	8
	.export set_memory24_asm
set_memory24_asm
; arg0 = addr
; arg1 = val
	extru	arg0,23,16,arg3

	addil	l%PAGE_INFO_WR_OFFSET,arg3
	extrs	arg0,30,7,scratch4
	ldwx,s	r1(page_info_ptr),scratch2
	ldi	0xff,scratch3
	and	scratch3,scratch2,scratch3
	dep	arg0,31,8,scratch2
	comib,=,n -1,scratch4,set_memory24_pieces_stub_asm
	comib,<>,n 0,scratch3,set_memory24_pieces_stub_asm
	stb	arg1,0(scratch2)
	extru	arg1,23,8,arg3
	CYCLES_PLUS_3
	stb	arg3,1(scratch2)
	extru	arg1,15,8,arg3
	bv	0(link)
	stb	arg3,2(scratch2)

set_memory24_pieces_stub_asm
	addi	1,arg0,scratch1
	stw	link,STACK_SET_MEMORY24_SAVE3(sp)
	extru	arg1,23,16,scratch2
	stw	scratch1,STACK_SET_MEMORY24_SAVE1(sp)
	bl	set_memory_asm,link
	stw	scratch2,STACK_SET_MEMORY24_SAVE2(sp)

	ldw	STACK_SET_MEMORY24_SAVE1(sp),arg0
	bl	set_memory_asm,link
	ldw	STACK_SET_MEMORY24_SAVE2(sp),arg1

	ldw	STACK_SET_MEMORY24_SAVE1(sp),arg0
	ldw	STACK_SET_MEMORY24_SAVE3(sp),link
	addi	1,arg0,arg0
	b	set_memory_asm
	ldb	STACK_SET_MEMORY24_SAVE2+2(sp),arg1





	.import g_num_breakpoints,data
	.import g_breakpts,data

	.align	8
	.export check_breakpoints_asm,code
check_breakpoints_asm
; can't use arg0-arg3.  don't use scratch2,scratch3
; scratch4: return link
;
	ldil	l%g_num_breakpoints,scratch1
	ldil	l%g_breakpts,ret0
	ldw	r%g_num_breakpoints(scratch1),r1
	ldo	r%g_breakpts(ret0),ret0
	addi,>=	-1,r1,r1
	bv,n	0(scratch4)
	ldwx,s	r1(ret0),scratch1
check_breakpoints_loop_asm
	comb,=,n scratch1,arg0,check_breakpoints_hit
	addib,>=,n -1,r1,check_breakpoints_loop_asm
	ldwx,s	r1(ret0),scratch1

	bv	0(scratch4)
	nop

	.export check_breakpoints_hit,code
check_breakpoints_hit
	LDC(halt_sim,scratch1)
	ldw	(scratch1),r1
	ldil	l%g_fcycles_stop,ret0
	depi	1,31,1,r1
	stw	0,r%g_fcycles_stop(ret0)
	stw	0,r%g_fcycles_stop+4(ret0)
	bv	0(scratch4)
	stw	r1,(scratch1)
	nop
	nop
	nop



	.align	8
	.export set_mem_yreg
set_mem_yreg
; arg0 = addr to write
	extru,=	psr,27,1,0		;null branch if 16 bit
	b	set_memory_asm
	copy	yreg,arg1
;if get here, 16 bit yreg
	b,n	set_memory16_asm
	nop


	.align	8
	.export set_mem_xreg
set_mem_xreg
; arg0 = addr to write
	extru,=	psr,27,1,0		;null branch if 16 bit
	b	set_memory_asm
	copy	xreg,arg1
;if get here, 16 bit xreg
	b,n	set_memory16_asm
	nop



	.export get_memory_outofrange,code
get_memory_outofrange
	break


get_mem_b0_16_stub
	b	get_mem_b0_16
	nop

	.align	8
get_mem_b0_direct_page_16
; get 2 bytes for direct-page fetch.
; arg0 = addr;
; if emul and dl = 0, then stick dh in 
; into high bytes.
; if emul, grab + 1 byte from dh page also.
; if not emul, just call get_mem_b0
	ldi	0xff,scratch2
	extru,<> psr,23,1,0		;null next if emul bit set
	b	get_mem_b0_16
	extru	direct,23,8,scratch1
	and	arg0,scratch2,scratch3
	extru,<> direct,31,8,0		;null if direct not page aligned
	dep	scratch1,23,24,arg0	;..done only if direct is page aligned
	comb,<>	scratch3,scratch2,get_mem_b0_16_stub
	stw	link,STACK_GET_MEM_B0_DIRECT_SAVELINK(sp)
; we're at 0x??ff, so next byte needs to come from 0x??00.
	bl	get_mem_b0_8,link
	stw	arg0,STACK_GET_MEM_B0_DIRECT_ARG0(sp)
; now, get next byte
	ldw	STACK_GET_MEM_B0_DIRECT_ARG0(sp),arg0
	extru	direct,23,8,scratch1
	stw	ret0,STACK_GET_MEM_B0_DIRECT_RET0(sp)
	addi	1,arg0,arg0
	extru,<> direct,31,8,0		;null if direct not page aligned
	dep	scratch1,23,24,arg0	;..done only if direct is page aligned
	bl	get_mem_b0_8,link
	nop

; and return
	copy	ret0,scratch2
	ldw	STACK_GET_MEM_B0_DIRECT_SAVELINK(sp),scratch1
	ldb	STACK_GET_MEM_B0_DIRECT_RET0+3(sp),ret0
	bv	(scratch1)
	dep	scratch2,23,8,ret0



push_8
	copy	arg0,arg1
	copy	stack,arg0
	addi	-1,stack,stack
	extru,=	psr,23,1,0		;emul mode?
	depi	1,23,24,stack
	b	set_mem_b0_8
	extru	stack,31,16,stack

pull_8
	addi	1,stack,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	extru	stack,31,16,stack
	b	get_mem_b0_8
	copy	stack,arg0

push_16
	copy	arg0,arg1
	bb,>=	psr,23,push_16_native
	extru	stack,30,7,scratch1

; push_16_emul
	addi	-2,stack,stack
	comib,=	0,scratch1,push_16_emul_page
	addi	1,stack,arg0		;we know we are not at end of page
	b	set_mem_b0_16
	depi	1,23,24,stack


push_16_emul_page
	stw	link,STACK_SAVE_PUSH16_LINK(sp)
	addi	1,arg0,arg0
	stw	arg1,STACK_SAVE_PUSH16_ARG1(sp)
	depi	1,23,24,arg0
	bl	set_mem_b0_8,link
	extru	arg1,23,8,arg1
; and do next push
	addi	1,stack,arg0
	depi	1,23,24,stack
	ldw	STACK_SAVE_PUSH16_LINK(sp),link
	ldb	STACK_SAVE_PUSH16_ARG1+3(sp),arg1
	b	set_mem_b0_8
	depi	1,23,24,arg0

push_16_native
; here, we're a native push_16
	addi	-2,stack,stack
	comib,=	0,scratch1,push_16_nat_page
	addi	1,stack,arg0		;we know we are not at end of page
	b	set_mem_b0_16
	extru	stack,31,16,stack


push_16_nat_page
	stw	link,STACK_SAVE_PUSH16_LINK(sp)
	addi	1,arg0,arg0
	stw	arg1,STACK_SAVE_PUSH16_ARG1(sp)
	extru	arg0,31,16,arg0
	bl	set_mem_b0_8,link
	extru	arg1,23,8,arg1
; and do next push
	addi	1,stack,arg0
	extru	stack,31,16,stack
	ldw	STACK_SAVE_PUSH16_LINK(sp),link
	ldb	STACK_SAVE_PUSH16_ARG1+3(sp),arg1
	b	set_mem_b0_8
	extru	arg0,31,16,arg0

push_16_unsafe
	copy	arg0,arg1
	addi	-1,stack,arg0
	addi	-2,stack,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	extru	arg0,31,16,arg0
	b	set_mem_b0_16
	extru	stack,31,16,stack

push_24_unsafe
	copy	arg0,arg1
	addi	-2,stack,arg0
	addi	-3,stack,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	extru	arg0,31,16,arg0
	b	set_mem_b0_24
	extru	stack,31,16,stack

pull_16_unsafe
	addi	1,stack,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	extru	stack,31,16,arg0
	addi	1,stack,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	b	get_mem_b0_16
	extru	stack,31,16,stack

	.align	8
pull_16
	extrs	stack,29,6,scratch1
	bb,<	psr,23,pull_16_emul
	addi	1,stack,arg0
	comib,=	-1,scratch1,pull_16_nat_page
	addi	2,stack,stack
; if we get here, native & not near page cross
	extru	arg0,31,16,arg0
	b	get_mem_b0_16
	extru	stack,31,16,stack

pull_16_emul
	comib,=	-1,scratch1,pull_16_emul_page
	addi	2,stack,stack
; if get here, emul & not near page cross
	b	get_mem_b0_16
	depi	1,23,24,stack

pull_16_nat_page
	stw	link,STACK_SAVE_PULL16_LINK(sp)
	bl	get_mem_b0_8,link
	extru	arg0,31,16,arg0
; got first byte
	stw	ret0,STACK_SAVE_PULL16_RET0(sp)
	extru	stack,31,16,stack
	bl	get_mem_b0_8,link
	copy	stack,arg0
; got second byte
	ldw	STACK_SAVE_PULL16_LINK(sp),link
	copy	ret0,scratch1
	ldb	STACK_SAVE_PULL16_RET0+3(sp),ret0
	bv	0(link)
	dep	scratch1,23,8,ret0

pull_16_emul_page
	stw	link,STACK_SAVE_PULL16_LINK(sp)
	bl	get_mem_b0_8,link
	depi	1,23,24,arg0
; got first byte
	stw	ret0,STACK_SAVE_PULL16_RET0(sp)
	depi	1,23,24,stack
	bl	get_mem_b0_8,link
	copy	stack,arg0
; got second byte
	ldw	STACK_SAVE_PULL16_LINK(sp),link
	copy	ret0,scratch1
	ldb	STACK_SAVE_PULL16_RET0+3(sp),ret0
	bv	0(link)
	dep	scratch1,23,8,ret0

	.export pull_24,code
pull_24
	extrs	stack,29,6,scratch1
	bb,<	psr,23,pull_24_emul
	addi	1,stack,arg0
	comib,=	-1,scratch1,pull_24_nat_page
	addi	3,stack,stack
; if we get here, native & not near page cross, go for it
	extru	arg0,31,16,arg0
	b	get_mem_b0_24
	extru	stack,31,16,stack

pull_24_emul
	depi	1,23,24,arg0
	comib,=	-1,scratch1,pull_24_emul_page
	addi	3,stack,stack
; if we get here, emul & not near page cross
	b	get_mem_b0_24
	depi	1,23,24,stack

pull_24_nat_page
	stw	link,STACK_SAVE_PULL24_LINK(sp)
	bl	get_mem_b0_8,link
	extru	arg0,31,16,arg0
; got first byte
	stw	ret0,STACK_SAVE_PULL24_RET0(sp)
	addi	-1,stack,arg0
	extru	stack,31,16,stack
	bl	get_mem_b0_8,link
	extru	arg0,31,16,arg0
; got second byte
	stb	ret0,STACK_SAVE_PULL24_RET0+2(sp)
	bl	get_mem_b0_8,link
	copy	stack,arg0
; got all bytes
	ldw	STACK_SAVE_PULL24_LINK(sp),link
	copy	ret0,scratch1
	ldw	STACK_SAVE_PULL24_RET0(sp),ret0
	bv	(link)
	dep	scratch1,15,8,ret0

pull_24_emul_page
	stw	link,STACK_SAVE_PULL24_LINK(sp)
	bl	get_mem_b0_8,link
	nop
; got first byte
	addi	-1,stack,arg0
	stw	ret0,STACK_SAVE_PULL24_RET0(sp)
	depi	1,23,24,stack
	bl	get_mem_b0_8,link
	depi	1,23,24,arg0
; got second byte
	stb	ret0,STACK_SAVE_PULL24_RET0+2(sp)
	bl	get_mem_b0_8,link
	copy	stack,arg0
; got all bytes
	ldw	STACK_SAVE_PULL24_LINK(sp),link
	copy	ret0,scratch1
	ldw	STACK_SAVE_PULL24_RET0(sp),ret0
	bv	(link)
	dep	scratch1,15,8,ret0

update_system_state_and_change_kbank
; kbank already changed..do nothing

update_system_state
; psr is new psw state
; arg0 is old in bits 31 and 30
	ldi	0x30,scratch1
	extru,=	psr,23,1,0
	depi	3,27,2,psr
	and	psr,scratch1,scratch1
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	dep	arg0,29,2,scratch1
	blr	scratch1,0
	addit,>= -0x3d,scratch1,0
; 0000: no change
	b	update_sys9
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 0001: x from 1->0
	b	update_sys9
	ldi	2,scratch1
	nop ! nop
	nop ! nop ! nop ! nop
; 0010: m from 1->0
	b	resize_acc_to16
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 0011: m,x from 1->0
	b	resize_acc_to16
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 0100: x from 0->1
	depi	0,23,24,yreg
	b	update_sys9
	depi	0,23,24,xreg
	nop
	nop ! nop ! nop ! nop
; 0101: no change
	b	update_sys9
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 0110: x from 0->1, m from 1->0
	depi	0,23,24,yreg
	ldi	0,ret0
	b	resize_acc_to16
	depi	0,23,24,xreg
	nop ! nop ! nop ! nop
; 0111: m from 1->0
	b	resize_acc_to16
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 1000: m from 0->1
	b	resize_acc_to8
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 1001: m from 0->1, x from 1->0
	b	resize_acc_to8
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 1010: no change
	b	update_sys9
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 1011: x from 1->0
	b	update_sys9
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 1100: m,x from 0->1
	depi	0,23,24,yreg
	ldi	0,ret0
	b	resize_acc_to8
	depi	0,23,24,xreg
	nop ! nop ! nop ! nop
; 1101: m from 0->1
	b	resize_acc_to8
	ldi	0,ret0
	nop ! nop
	nop ! nop ! nop ! nop
; 1110: x from 0->1
	depi	0,23,24,yreg
	ldi	0,ret0
	b	update_sys9
	depi	0,23,24,xreg
	nop ! nop ! nop ! nop
; 1111: no change
	b	update_sys9
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 10000
	break


	.export get_yreg_from_mem,code
get_yreg_from_mem
; arg0 = addr to read from, write into yreg
	bb,>=,n	psr,27,get_yreg_from_mem16
	bl	get_mem_b0_8,link
	extru	arg0,31,24,arg0

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,yreg

	.export get_yreg_from_mem16,code
get_yreg_from_mem16
	bl	get_mem_b0_16,link
	extru	arg0,31,24,arg0

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,yreg


	.export get_xreg_from_mem,code
get_xreg_from_mem
; arg0 = addr to read from, write into xreg
	bb,>=,n	psr,27,get_xreg_from_mem16
	bl	get_mem_b0_8,link
	extru	arg0,31,24,arg0

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,xreg

	.export get_xreg_from_mem16,code
get_xreg_from_mem16
	bl	get_mem_b0_16,link
	extru	arg0,31,24,arg0

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,xreg




	.export enter_engine,code
enter_engine
; load up regs with struct vals
	.proc
	.callinfo frame=STACK_ENGINE_SIZE,caller,save_rp,entry_gr=18,entry_fr=19
	.enter

	ldw	ENGINE_FPLUS_PTR(arg0),scratch1		;fplus ptr
	fldds	ENGINE_FCYCLES(arg0),fcycles

	ldil	l%g_fcycles_stop,fcycles_stop_ptr
	ldw	ENGINE_REG_ACC(arg0),acc
	ldo	r%g_fcycles_stop(fcycles_stop_ptr),fcycles_stop_ptr
	fldds	FPLUS_PLUS_1(scratch1),fr_plus_1
	ldo	FPLUS_PLUS_3(scratch1),ret0
	fldds	FPLUS_PLUS_2(scratch1),fr_plus_2
	ldil	l%g_last_vbl_dcycs,ret1
	fldds	FPLUS_PLUS_3-FPLUS_PLUS_3(ret0),fr_plus_3
	ldo	r%g_last_vbl_dcycs(ret1),ret1
	fldds	FPLUS_PLUS_X_M1-FPLUS_PLUS_3(ret0),fr_plus_x_m1
	fldds	0(ret1),fcycles_last_dcycs
	ldil	l%table8,ret0
	ldw	ENGINE_REG_XREG(arg0),xreg
	ldil	l%table16,inst_tab_ptr
	ldw	ENGINE_REG_YREG(arg0),yreg
	ldo	r%table8(ret0),ret0
	ldw	ENGINE_REG_STACK(arg0),stack
	ldo	r%table16(inst_tab_ptr),inst_tab_ptr
	ldw	ENGINE_REG_PSR(arg0),psr
	ldi	0,zero
	ldw	ENGINE_REG_DBANK(arg0),dbank
	ldil	l%page_info_rd_wr,page_info_ptr
	ldw	ENGINE_REG_DIRECT(arg0),direct
	extru,=	psr,26,1,0		;nullify if acc size = 0 == 16bit
	copy	ret0,inst_tab_ptr
	ldw	ENGINE_REG_KPC(arg0),kpc

	ldo	r%page_info_rd_wr(page_info_ptr),page_info_ptr
	extru,<> psr,30,1,0
	ldi	1,zero
	extru	psr,24,1,neg
	stw	arg0,STACK_SAVE_ARG0(sp)
	ldi	0xfd,const_fd
	b	dispatch
	ldi	0,scratch1

	.export resize_acc_to8,code
resize_acc_to8
	ldil	l%table8,inst_tab_ptr
	extru	psr,27,1,scratch1		;size of x
	b	update_sys9
	ldo	r%table8(inst_tab_ptr),inst_tab_ptr

	.export resize_acc_to16,code
resize_acc_to16
	ldil	l%table16,inst_tab_ptr
	extru	psr,27,1,scratch1
	b	update_sys9
	ldo	r%table16(inst_tab_ptr),inst_tab_ptr



dispatch_done_cycles_mismatch
	ldi	-1,ret0
	b	dispatch_done
	nop



	.export dispatch_done
dispatch_done
	bl	refresh_engine_struct,link
	ldw	STACK_SAVE_ARG0(sp),arg0
	.leave
	.procend

refresh_engine_struct
; warning--this routine must not change arg1, arg2, arg3, or ret0
; can only change scratch1

	comiclr,<> 0,zero,scratch1
	ldi	1,scratch1
	dep	neg,24,1,psr
	dep	scratch1,30,1,psr
	stw	acc,ENGINE_REG_ACC(arg0)
	stw	xreg,ENGINE_REG_XREG(arg0)
	stw	yreg,ENGINE_REG_YREG(arg0)
	stw	stack,ENGINE_REG_STACK(arg0)
	stw	dbank,ENGINE_REG_DBANK(arg0)
	stw	direct,ENGINE_REG_DIRECT(arg0)
	stw	psr,ENGINE_REG_PSR(arg0)
	stw	kpc,ENGINE_REG_KPC(arg0)
	bv	0(link)
	fstds	fcycles,ENGINE_FCYCLES(arg0)

	.export check_irqs_pending,code
update_sys9
check_irqs_pending
; if any g_irq_pending, return RET_IRQ
	ldil	l%g_irq_pending,scratch1
	ldw	r%g_irq_pending(scratch1),scratch2
	bb,<,n	psr,29,dispatch
	comib,=	0,scratch2,dispatch
	zdepi	RET_IRQ,3,4,ret0
	b,n	dispatch_done
	nop

	.export clr_halt_act
	.export set_halt_act
clr_halt_act
	LDC(halt_sim,scratch1)
	bv	0(link)
	stw	0,(scratch1)

set_halt_act
	LDC(halt_sim,scratch1)
	ldw	(scratch1),scratch2
	ldil	l%g_fcycles_stop,scratch3
	stw	0,r%g_fcycles_stop(scratch3)
	or	scratch2,arg0,arg0
	stw	0,r%g_fcycles_stop+4(scratch3)
	bv	0(link)
	stw	arg0,(scratch1)


	.align	32
	.export dispatch_fast,code
dispatch_fast
; instr is the instr to fetch
#ifdef LOG_PC
	b	dispatch
	nop
#endif
	fldds	0(fcycles_stop_ptr),fcycles_stop
	extru	kpc,23,16,arg2

	extru	kpc,31,8,scratch4
	ldwx,s	arg2(page_info_ptr),scratch2

	ldwx,s	instr(inst_tab_ptr),link
	fcmp,>,dbl fcycles,fcycles_stop		;C=1 if must stop

	addl	scratch4,scratch2,scratch1
	comclr,>= scratch4,const_fd,0	;stop for pieces if near end of page

	ldi	-1,scratch2
	bb,<,n	scratch2,BANK_IO_BIT,dispatch_instr_io

	ftest				;null next if can cont

	bv	0(link)
	CYCLES_PLUS_2

	b	dispatch_instr_io
	CYCLES_MINUS_2


	.align	32
	.export dispatch,code
dispatch

#ifdef CHECK_SIZE_CONSISTENCY
	nop
	bl	check_size_consist,link
	nop
#endif

#ifdef DEBUG_TOOLBOX
	ldil	l%g_rom_version,scratch1
	ldw	r%g_rom_version(scratch1),scratch1
	ldi	0x00db,scratch1			;ROM 01
	comiclr,> 3,scratch1,0
	ldi	0x00e5,scratch1			;ROM 03
	depi	-2,15,8,scratch1		;set bank to 0xfe
	comb,<>,n scratch1,kpc,no_debug_toolbox
	copy	xreg,arg0
	copy	stack,arg1
	bl	toolbox_debug_c,link
	copy	cycles,arg2

	extru	kpc,23,16,scratch2
no_debug_toolbox
#endif
	fldds	0(fcycles_stop_ptr),fcycles_stop
	extru	kpc,23,16,arg2

	ldi	0xfd,scratch3
	ldwx,s	arg2(page_info_ptr),scratch2

	fcmp,<=,dbl fcycles,fcycles_stop		;C=1 if can cont
	extru	kpc,31,8,scratch4

	ldbx	scratch4(scratch2),instr
	comclr,>= scratch4,scratch3,0	;stop for pieces if near end of page

	ftest				;null next if can cont

	ldi	-1,scratch2
	ldwx,s	instr(inst_tab_ptr),link

	addl	scratch4,scratch2,scratch1
	bb,<,n	scratch2,BANK_IO_BIT,dispatch_instr_io

	; depi	0,31,3,link

#ifndef LOG_PC
	bv	0(link)
	CYCLES_PLUS_2
#else
	CYCLES_PLUS_2

	.import	log_pc_ptr,data
	.import	log_pc_start_ptr,data
	.import	log_pc_end_ptr,data
	.export log_pc_asm
log_pc_asm
; save regs into log_pc_ptr, wrap around to log_pc_start_ptr if
;  log_pc_ptr gets > log_pc_end_ptr
	ldb	1(scratch1),scratch3
	dep	neg,24,1,psr				;set neg
	ldb	2(scratch1),scratch2
	ldil	l%log_pc_ptr,scratch4
	ldb	3(scratch1),ret0
	fsub,dbl fcycles_last_dcycs,fr_plus_2,ftmp1
	dep	scratch2,23,8,scratch3
	ldo	r%log_pc_ptr(scratch4),scratch4
	dep	instr,7,8,scratch3
	ldw	0(scratch4),scratch2
	dep	ret0,15,8,scratch3
	copy	kpc,ret1
	depi	0,30,1,psr				;zero
	comiclr,<> 0,zero,0
	depi	1,30,1,psr				;set zero
	stw	scratch3,LOG_PC_INSTR(scratch2)
	dep	dbank,7,8,ret1
	copy	acc,scratch3
	dep	psr,15,16,scratch3
	fadd,dbl fcycles,ftmp1,ftmp1
	stw	ret1,LOG_PC_DBANK_KPC(scratch2)
	copy	yreg,ret1
	stw	scratch3,LOG_PC_PSR_ACC(scratch2)
	dep	xreg,15,16,ret1
	copy	direct,scratch3
	fstds	ftmp1,LOG_PC_DCYCS(scratch2)
	ldw	rs%log_pc_end_ptr-log_pc_ptr(scratch4),ret0
	dep	stack,15,16,scratch3
	stw	ret1,LOG_PC_XREG_YREG(scratch2)
	addi	LOG_PC_SIZE,scratch2,r31
	stw	scratch3,LOG_PC_STACK_DIRECT(scratch2)

;	comb,>=	r31,ret0,log_pc_oflow
;	nop

	comclr,< r31,ret0,0
; reload log_pc with log_pc_start_ptr
	ldw	rs%log_pc_start_ptr-log_pc_ptr(scratch4),r31

	bv	0(link)
	stw	r31,0(scratch4)

log_pc_oflow
	ldil	l%g_fcycles_stop,scratch3
	ldil	l%halt_sim,ret0
	stw	0,r%g_fcycles_stop(scratch3)
	ldi	2,arg0
	stw	0,r%g_fcycles_stop+4(scratch3)
	stw	arg0,r%halt_sim(ret0)

	ldw	rs%log_pc_start_ptr-log_pc_ptr(scratch4),r31
	bv	0(link)
	stw	r31,0(scratch4)
#endif


	.export dispatch_instr_io,code
dispatch_instr_io
; check if we're here because of timeout or halt required
	fcmp,<=,dbl fcycles,fcycles_stop	;C=1 if we can cont
	ldwx,s	arg2(page_info_ptr),scratch2

	ftest					;do next instr if must stop
	b,n	dispatch_done_clr_ret0

	bb,>=,n	scratch2,BANK_IO_BIT,dispatch_instr_pieces

	ldil	l%0xc700,scratch1
	ldo	r%0xc700(scratch1),scratch1
	addi	0x0a,scratch1,scratch2
	comb,=	scratch1,kpc,dispatch_done
	zdepi	RET_C700,3,4,ret0

	addi	0xd,scratch1,scratch3
	comb,=	scratch2,kpc,dispatch_done
	zdepi	RET_C70A,3,4,ret0

	comb,=	scratch3,kpc,dispatch_done
	zdepi	RET_C70D,3,4,ret0

	.export dispatch_instr_pieces,code
dispatch_instr_pieces
; fetch pc, get size from inst_info_ptr
	bl	get_mem_long_8,link
	copy	kpc,arg0
; ret is instr
	ldwx,s	ret0(inst_tab_ptr),link
	ldil	l%sizes_tab,scratch4
	copy	ret0,instr
	ldo	r%sizes_tab(scratch4),scratch4
	addi	1,kpc,arg0
	ldbx	instr(scratch4),scratch2
#ifdef LOG_PC
; save "real" link so call_log_pc can restore it

	stw	link,STACK_SAVE_DISPATCH_LINK(sp)
	LDC(call_log_pc,link)
	stw	instr,STACK_SAVE_INSTR(sp)
#endif
	stw	link,STACK_SAVE_DISP_PIECES_LINK(sp)

	ldi	0x1bea,ret0
	sh3add	scratch2,0,scratch2
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	blr	scratch2,0
	addit,>= -48,scratch2,0

/* must correct cycle count so all instrs are called with cycls += 2 */
/*  since get_mem will auto-inc cycles by the number of bytes, we */
/*  need to "patch" things here, by adding 1 for 1byte, and subbing */
/*  from 3 and 4 byte instrs */
; 0
	bv	0(link)
	CYCLES_PLUS_1
	nop
	nop
	nop ! nop ! nop ! nop
	nop ! nop ! nop ! nop
	nop ! nop ! nop ! nop
; 1
	bl	get_mem_long_8,link
	nop
	ldw	STACK_SAVE_DISP_PIECES_LINK(sp),link
	dep	ret0,15,8,ret0
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	bv	0(link)
	stw	ret0,0(scratch1)
	nop ! nop
	nop ! nop ! nop ! nop
	nop ! nop ! nop ! nop
; 2
	bl	get_mem_long_16,link
	CYCLES_MINUS_1
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	ldw	STACK_SAVE_DISP_PIECES_LINK(sp),link
	dep	ret0,15,8,ret0
	bv	0(link)
	stw	ret0,0(scratch1)
	nop
	nop ! nop ! nop ! nop
	nop ! nop ! nop ! nop
; 3
	bl	get_mem_long_24,link
	CYCLES_MINUS_2
	shd	ret0,ret0,16,scratch2
	ldw	STACK_SAVE_DISP_PIECES_LINK(sp),link
	extru	ret0,23,8,ret0
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	dep	ret0,23,8,scratch2
	bv	0(link)
	stw	scratch2,0(scratch1)
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 4 variable acc size
	extru,<> psr,26,1,0
	bl,n	get_mem_long_16,link
	bl,n	get_mem_long_8,link
	CYCLES_MINUS_1
	ldw	STACK_SAVE_DISP_PIECES_LINK(sp),link
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	dep	ret0,15,8,ret0
	bv	0(link)
	stw	ret0,0(scratch1)
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 5 variable x size
	extru,<> psr,27,1,0
	bl,n	get_mem_long_16,link
	bl,n	get_mem_long_8,link
	CYCLES_MINUS_1
	ldw	STACK_SAVE_DISP_PIECES_LINK(sp),link
	ldo	STACK_SAVE_TMP_INST(sp),scratch1
	dep	ret0,15,8,ret0
	bv	0(link)
	stw	ret0,0(scratch1)
	nop ! nop ! nop
	nop ! nop ! nop ! nop
; 6 = evil
	break


#ifdef LOG_PC
	.export call_log_pc,code
call_log_pc
; ret0 = operands
; must get instr = instruction
; and link = correct dispatch loc
	ldw	STACK_SAVE_INSTR(sp),instr
	b	log_pc_asm
	ldw	STACK_SAVE_DISPATCH_LINK(sp),link
#endif

dispatch_done_clr_ret0
	nop			;just in case of bad nullification
	b	dispatch_done
	ldi	0,ret0


#ifdef CHECK_SIZE_CONSISTENCY
	.import	size_fail,code

	.export check_size_consist
check_size_consist
	ldil	l%table16,scratch1
	ldil	l%table8,scratch2
	ldo	r%table16(scratch1),scratch1
	bb,<	psr,26,check_tab_8_bit
	ldo	r%table8(scratch2),scratch2
; else 16
	comb,=	scratch1,inst_tab_ptr,acc_size_ok
	nop

	.export acc_tab_fail1
acc_tab_fail1
	copy	inst_tab_ptr,arg1
	copy	scratch1,arg2
	bl	size_fail,link
	ldi	0x100,arg0
	b,n	dispatch_done_clr_ret0
; 8
	.export check_tab_8_bit
check_tab_8_bit
	comb,=	scratch2,inst_tab_ptr,acc_size_ok
	nop

	.export acc_tab_fail0
acc_tab_fail0
	copy	inst_tab_ptr,arg1
	copy	scratch2,arg2
	bl	size_fail,link
	ldi	0x101,arg0
	b	dispatch_done
	ldi	0,ret0

	.export acc_size_ok
acc_size_ok
	bv	0(link)
	nop
#endif 

	.align	8
adc_binary_8_entry2
	extru	psr,31,1,scratch3
	add	ret0,scratch3,ret0

	dep	ret0,31,8,acc
	extru	ret0,31,8,zero

/*  and calc overflow */
	xor	arg0,ret0,arg2			/* cmp binary add res w/ src1 */
	xor	arg0,scratch2,scratch3		/* cmp signs of two inputs */
	extru	ret0,24,1,neg
	andcm	arg2,scratch3,scratch3		/* and that with ~res. */
	extru	ret0,23,1,scratch4
	extru	scratch3,24,1,scratch3
	dep	scratch4,31,1,psr		/* set carry */
	b	dispatch
	dep	scratch3,25,1,psr		/* set overflow */

	.align	8
	.export	adc_binary_8
adc_binary_8
	extru	ret0,31,8,scratch2
	bb,>=	psr,28,adc_binary_8_entry2
	add	arg0,scratch2,ret0


	ldil	l%dispatch,link
	b	adc_decimal_8
	ldo	r%dispatch(link),link

	.export adc_decimal_8
/*  adds arg0 to scratch2 */
/*  acc8 (in arg0) and ret0 have already been added into ret0.  Ignore that */
adc_decimal_8
	ldi	0xf,scratch1
	extru	psr,31,1,ret0

	and	arg0,scratch1,scratch3
	and	scratch2,scratch1,scratch4

	add	scratch3,scratch4,ret1
	ldi	0xf0,arg3

	add	ret0,ret1,ret0

	and	arg0,arg3,scratch3
	addi	-0xa,ret0,ret1

	and	scratch2,arg3,scratch4
	depi	1,27,4,ret1
	comiclr,> 0xa,ret0,0
	copy	ret1,ret0

	add	scratch3,scratch4,ret1
	add	ret0,ret1,ret0

	extru	ret0,24,1,ret1
	extru	ret0,23,1,arg1
	xor	ret1,arg1,ret1
	dep	ret1,25,1,psr		/* ov=((sum>>2) ^ (sum>>1) & 0x40 */

	comiclr,> 0xa0,ret0,0
	addi	0x60,ret0,ret0

	xor	arg0,scratch2,scratch4
	extru	ret0,31,8,zero

	extru,=	scratch4,24,1,0
	depi	0,25,1,psr			/* no overflow! */


	depi	0,31,1,psr
	comiclr,> 0x100,ret0,0
	addi	1,psr,psr

	extru	ret0,24,1,neg
	bv	0(link)
	dep	zero,31,8,acc



	.align	8
	.export	sbc_binary_8,code
sbc_binary_8
	extru	ret0,31,8,scratch2
	bb,>=	psr,28,adc_binary_8_entry2
	add	arg0,scratch2,ret0

	ldil	l%dispatch,link
	b	sbc_decimal_8
	ldo	r%dispatch(link),link


/* else decimal */
	.export sbc_decimal_8,code
sbc_decimal_8
/*  do arg0 - scratch2 = acc */
	ldi	0xf,scratch1
	extru	psr,31,1,ret0

	and	scratch2,scratch1,scratch3
	and	arg0,scratch1,scratch4

	add	scratch3,ret0,ret0

	add	ret0,scratch4,ret0
	ldi	0xf0,arg3

	addi	-0x6,ret0,ret1
	and	scratch2,arg3,scratch3

	and	ret1,scratch1,ret1		/* sum2 = (sum - 0x6) & 0xf */
	and	arg0,arg3,scratch4
	comiclr,<= 0x10,ret0,0
	copy	ret1,ret0			/* sum = sum2 */

	add	scratch3,scratch4,ret1
	ldi	0xff,arg2
	add	ret0,ret1,ret0

	extru	ret0,24,1,ret1
	addi	0xa0,ret0,scratch3
	extru	ret0,23,1,arg3
	and	scratch3,arg2,scratch3		/* (sum = sum + 0xa0) & 0xff */
	xor	ret1,arg3,ret1

	dep	ret1,25,1,psr			/* overflow = ((sum >> 2) ^ */
						/* 	(sum >> 1)) & 0x40 */

	depi	0,31,1,psr
	comiclr,<= 0x100,ret0,arg3
	or,TR	scratch3,0,ret0
	addi	1,psr,psr

	and	ret0,arg2,zero
	extru	ret0,24,1,neg

	xor	arg0,scratch2,ret1

	extru,= ret1,24,1,0
	depi	0,25,1,psr			/* clear overflow */

	bv	0(link)
	dep	ret0,31,8,acc



	.align	8
	.export adc_binary_16
adc_binary_16
	extru	ret0,31,16,scratch2
	bb,<	psr,28,adc_decimal_16
	add	arg0,scratch2,ret0

adc_binary_16_entry2
	extru	psr,31,1,scratch1
	add	ret0,scratch1,ret0

	dep	ret0,31,16,acc
	extru	ret0,31,16,zero

/*  and calc overflow */
	xor	arg0,ret0,arg2			/* cmp binary add res w/ src1 */
	xor	arg0,scratch2,scratch3
	extru	ret0,16,1,neg
	andcm	arg2,scratch3,scratch3		/* and that with ~res. */
	extru	ret0,15,1,scratch4
	extru	scratch3,16,1,scratch3
	dep	scratch4,31,1,psr		/* set carry */
	b	dispatch
	dep	scratch3,25,1,psr		/* set overflow */


 	.export	adc_decimal_16
adc_decimal_16
/*  must save arg0, scratch2 */
	stw	arg0,STACK_SAVE_DECIMAL16_A(sp)
	extru	arg0,31,8,arg0
	stw	scratch2,STACK_SAVE_DECIMAL16_B(sp)
	bl	adc_decimal_8,link
	extru	scratch2,31,8,scratch2

	ldb	STACK_SAVE_DECIMAL16_A+2(sp),arg0
	ldb	STACK_SAVE_DECIMAL16_B+2(sp),scratch2
	bl	adc_decimal_8,link
	stw	acc,STACK_SAVE_DECIMAL16_A(sp)

	ldw	STACK_SAVE_DECIMAL16_A(sp),scratch1
	zdep	acc,23,8,acc
	dep	scratch1,31,8,acc
	b	dispatch
	copy	acc,zero


	.align	8
	.export	sbc_binary_16,code
sbc_binary_16
	extru	ret0,31,16,scratch2
	bb,>=	psr,28,adc_binary_16_entry2
	add	arg0,scratch2,ret0

/* else decimal */
	.export sbc_decimal_16,code
sbc_decimal_16
	stw	arg0,STACK_SAVE_DECIMAL16_A(sp)
	extru	arg0,31,8,arg0
	stw	scratch2,STACK_SAVE_DECIMAL16_B(sp)
	bl	sbc_decimal_8,link
	extru	scratch2,31,8,scratch2

	ldb	STACK_SAVE_DECIMAL16_A+2(sp),arg0
	ldb	STACK_SAVE_DECIMAL16_B+2(sp),scratch2
	bl	sbc_decimal_8,link
	stw	acc,STACK_SAVE_DECIMAL16_A(sp)

	ldw	STACK_SAVE_DECIMAL16_A(sp),scratch1
	zdep	acc,23,8,acc
	dep	scratch1,31,8,acc
	b	dispatch
	copy	acc,zero




#define ACC8
	.code
#define INCLUDE_RCSID_S
#include "defs_instr.h"
#include "8inst_s.h" 
#undef INCLUDE_RCSID_S
	.code
#undef SYM
#undef ACC8

	.code
#include "defs_instr.h"
#include "16inst_s.h"
	.code
#undef SYM

	.export inst00_8
	.export inst01_8
	.export inst02_8
	.export inst03_8
	.export inst04_8
	.export inst05_8
	.export inst06_8
	.export inst07_8
	.export inst08_8
	.export inst09_8
	.export inst0a_8
	.export inst0b_8
	.export inst0c_8
	.export inst0d_8
	.export inst0e_8
	.export inst0f_8

	.export inst10_8
	.export inst11_8
	.export inst12_8
	.export inst13_8
	.export inst14_8
	.export inst15_8
	.export inst16_8
	.export inst17_8
	.export inst18_8
	.export inst19_8
	.export inst1a_8
	.export inst1b_8
	.export inst1c_8
	.export inst1d_8
	.export inst1e_8
	.export inst1f_8

	.export inst20_8
	.export inst21_8
	.export inst22_8
	.export inst23_8
	.export inst24_8
	.export inst25_8
	.export inst26_8
	.export inst27_8
	.export inst28_8
	.export inst29_8
	.export inst2a_8
	.export inst2b_8
	.export inst2c_8
	.export inst2d_8
	.export inst2e_8
	.export inst2f_8

	.export inst30_8
	.export inst31_8
	.export inst32_8
	.export inst33_8
	.export inst34_8
	.export inst35_8
	.export inst36_8
	.export inst37_8
	.export inst38_8
	.export inst39_8
	.export inst3a_8
	.export inst3b_8
	.export inst3c_8
	.export inst3d_8
	.export inst3e_8
	.export inst3f_8

	.export inst40_8
	.export inst41_8
	.export inst42_8
	.export inst43_8
	.export inst44_8
	.export inst45_8
	.export inst46_8
	.export inst47_8
	.export inst48_8
	.export inst49_8
	.export inst4a_8
	.export inst4b_8
	.export inst4c_8
	.export inst4d_8
	.export inst4e_8
	.export inst4f_8

	.export inst50_8
	.export inst51_8
	.export inst52_8
	.export inst53_8
	.export inst54_8
	.export inst55_8
	.export inst56_8
	.export inst57_8
	.export inst58_8
	.export inst59_8
	.export inst5a_8
	.export inst5b_8
	.export inst5c_8
	.export inst5d_8
	.export inst5e_8
	.export inst5f_8

	.export inst60_8
	.export inst61_8
	.export inst62_8
	.export inst63_8
	.export inst64_8
	.export inst65_8
	.export inst66_8
	.export inst67_8
	.export inst68_8
	.export inst69_8
	.export inst6a_8
	.export inst6b_8
	.export inst6c_8
	.export inst6d_8
	.export inst6e_8
	.export inst6f_8

	.export inst70_8
	.export inst71_8
	.export inst72_8
	.export inst73_8
	.export inst74_8
	.export inst75_8
	.export inst76_8
	.export inst77_8
	.export inst78_8
	.export inst79_8
	.export inst7a_8
	.export inst7b_8
	.export inst7c_8
	.export inst7d_8
	.export inst7e_8
	.export inst7f_8

	.export inst80_8
	.export inst81_8
	.export inst82_8
	.export inst83_8
	.export inst84_8
	.export inst85_8
	.export inst86_8
	.export inst87_8
	.export inst88_8
	.export inst89_8
	.export inst8a_8
	.export inst8b_8
	.export inst8c_8
	.export inst8d_8
	.export inst8e_8
	.export inst8f_8
	.export inst90_8
	.export inst91_8
	.export inst92_8
	.export inst93_8
	.export inst94_8
	.export inst95_8
	.export inst96_8
	.export inst97_8
	.export inst98_8
	.export inst99_8
	.export inst9a_8
	.export inst9b_8
	.export inst9c_8
	.export inst9d_8
	.export inst9e_8
	.export inst9f_8
	.export insta0_8
	.export insta1_8
	.export insta2_8
	.export insta3_8
	.export insta4_8
	.export insta5_8
	.export insta6_8
	.export insta7_8
	.export insta8_8
	.export insta9_8
	.export instaa_8
	.export instab_8
	.export instac_8
	.export instad_8
	.export instae_8
	.export instaf_8
	.export instb0_8
	.export instb1_8
	.export instb2_8
	.export instb3_8
	.export instb4_8
	.export instb5_8
	.export instb6_8
	.export instb7_8
	.export instb8_8
	.export instb9_8
	.export instba_8
	.export instbb_8
	.export instbc_8
	.export instbd_8
	.export instbe_8
	.export instbf_8
	.export instc0_8
	.export instc1_8
	.export instc2_8
	.export instc3_8
	.export instc4_8
	.export instc5_8
	.export instc6_8
	.export instc7_8
	.export instc8_8
	.export instc9_8
	.export instca_8
	.export instcb_8
	.export instcc_8
	.export instcd_8
	.export instce_8
	.export instcf_8
	.export instd0_8
	.export instd1_8
	.export instd2_8
	.export instd3_8
	.export instd4_8
	.export instd5_8
	.export instd6_8
	.export instd7_8
	.export instd8_8
	.export instd9_8
	.export instda_8
	.export instdb_8
	.export instdc_8
	.export instdd_8
	.export instde_8
	.export instdf_8
	.export inste0_8
	.export inste1_8
	.export inste2_8
	.export inste3_8
	.export inste4_8
	.export inste5_8
	.export inste6_8
	.export inste7_8
	.export inste8_8
	.export inste9_8
	.export instea_8
	.export insteb_8
	.export instec_8
	.export insted_8
	.export instee_8
	.export instef_8
	.export instf0_8
	.export instf1_8
	.export instf2_8
	.export instf3_8
	.export instf4_8
	.export instf5_8
	.export instf6_8
	.export instf7_8
	.export instf8_8
	.export instf9_8
	.export instfa_8
	.export instfb_8
	.export instfc_8
	.export instfd_8
	.export instfe_8
	.export instff_8


	.export inst00_16
	.export inst01_16
	.export inst02_16
	.export inst03_16
	.export inst04_16
	.export inst05_16
	.export inst06_16
	.export inst07_16
	.export inst08_16
	.export inst09_16
	.export inst0a_16
	.export inst0b_16
	.export inst0c_16
	.export inst0d_16
	.export inst0e_16
	.export inst0f_16

	.export inst10_16
	.export inst11_16
	.export inst12_16
	.export inst13_16
	.export inst14_16
	.export inst15_16
	.export inst16_16
	.export inst17_16
	.export inst18_16
	.export inst19_16
	.export inst1a_16
	.export inst1b_16
	.export inst1c_16
	.export inst1d_16
	.export inst1e_16
	.export inst1f_16

	.export inst20_16
	.export inst21_16
	.export inst22_16
	.export inst23_16
	.export inst24_16
	.export inst25_16
	.export inst26_16
	.export inst27_16
	.export inst28_16
	.export inst29_16
	.export inst2a_16
	.export inst2b_16
	.export inst2c_16
	.export inst2d_16
	.export inst2e_16
	.export inst2f_16

	.export inst30_16
	.export inst31_16
	.export inst32_16
	.export inst33_16
	.export inst34_16
	.export inst35_16
	.export inst36_16
	.export inst37_16
	.export inst38_16
	.export inst39_16
	.export inst3a_16
	.export inst3b_16
	.export inst3c_16
	.export inst3d_16
	.export inst3e_16
	.export inst3f_16

	.export inst40_16
	.export inst41_16
	.export inst42_16
	.export inst43_16
	.export inst44_16
	.export inst45_16
	.export inst46_16
	.export inst47_16
	.export inst48_16
	.export inst49_16
	.export inst4a_16
	.export inst4b_16
	.export inst4c_16
	.export inst4d_16
	.export inst4e_16
	.export inst4f_16

	.export inst50_16
	.export inst51_16
	.export inst52_16
	.export inst53_16
	.export inst54_16
	.export inst55_16
	.export inst56_16
	.export inst57_16
	.export inst58_16
	.export inst59_16
	.export inst5a_16
	.export inst5b_16
	.export inst5c_16
	.export inst5d_16
	.export inst5e_16
	.export inst5f_16

	.export inst60_16
	.export inst61_16
	.export inst62_16
	.export inst63_16
	.export inst64_16
	.export inst65_16
	.export inst66_16
	.export inst67_16
	.export inst68_16
	.export inst69_16
	.export inst6a_16
	.export inst6b_16
	.export inst6c_16
	.export inst6d_16
	.export inst6e_16
	.export inst6f_16

	.export inst70_16
	.export inst71_16
	.export inst72_16
	.export inst73_16
	.export inst74_16
	.export inst75_16
	.export inst76_16
	.export inst77_16
	.export inst78_16
	.export inst79_16
	.export inst7a_16
	.export inst7b_16
	.export inst7c_16
	.export inst7d_16
	.export inst7e_16
	.export inst7f_16

	.export inst80_16
	.export inst81_16
	.export inst82_16
	.export inst83_16
	.export inst84_16
	.export inst85_16
	.export inst86_16
	.export inst87_16
	.export inst88_16
	.export inst89_16
	.export inst8a_16
	.export inst8b_16
	.export inst8c_16
	.export inst8d_16
	.export inst8e_16
	.export inst8f_16
	.export inst90_16
	.export inst91_16
	.export inst92_16
	.export inst93_16
	.export inst94_16
	.export inst95_16
	.export inst96_16
	.export inst97_16
	.export inst98_16
	.export inst99_16
	.export inst9a_16
	.export inst9b_16
	.export inst9c_16
	.export inst9d_16
	.export inst9e_16
	.export inst9f_16
	.export insta0_16
	.export insta1_16
	.export insta2_16
	.export insta3_16
	.export insta4_16
	.export insta5_16
	.export insta6_16
	.export insta7_16
	.export insta8_16
	.export insta9_16
	.export instaa_16
	.export instab_16
	.export instac_16
	.export instad_16
	.export instae_16
	.export instaf_16
	.export instb0_16
	.export instb1_16
	.export instb2_16
	.export instb3_16
	.export instb4_16
	.export instb5_16
	.export instb6_16
	.export instb7_16
	.export instb8_16
	.export instb9_16
	.export instba_16
	.export instbb_16
	.export instbc_16
	.export instbd_16
	.export instbe_16
	.export instbf_16
	.export instc0_16
	.export instc1_16
	.export instc2_16
	.export instc3_16
	.export instc4_16
	.export instc5_16
	.export instc6_16
	.export instc7_16
	.export instc8_16
	.export instc9_16
	.export instca_16
	.export instcb_16
	.export instcc_16
	.export instcd_16
	.export instce_16
	.export instcf_16
	.export instd0_16
	.export instd1_16
	.export instd2_16
	.export instd3_16
	.export instd4_16
	.export instd5_16
	.export instd6_16
	.export instd7_16
	.export instd8_16
	.export instd9_16
	.export instda_16
	.export instdb_16
	.export instdc_16
	.export instdd_16
	.export instde_16
	.export instdf_16
	.export inste0_16
	.export inste1_16
	.export inste2_16
	.export inste3_16
	.export inste4_16
	.export inste5_16
	.export inste6_16
	.export inste7_16
	.export inste8_16
	.export inste9_16
	.export instea_16
	.export insteb_16
	.export instec_16
	.export insted_16
	.export instee_16
	.export instef_16
	.export instf0_16
	.export instf1_16
	.export instf2_16
	.export instf3_16
	.export instf4_16
	.export instf5_16
	.export instf6_16
	.export instf7_16
	.export instf8_16
	.export instf9_16
	.export instfa_16
	.export instfb_16
	.export instfc_16
	.export instfd_16
	.export instfe_16
	.export instff_16


	.data
#define INCLUDE_RCSID_S
#include "8size_s.h"
#undef INCLUDE_RCSID_S

	.export table8,data
table8
#include "8size_s.h"

	.export	table16,data
table16
#include "16size_s.h"

	.export	sizes_tab,data
sizes_tab
#include "size_s.h"


	.export g_engine_c_mode,data
g_engine_c_mode
	.word	0

	.bss

	.export slow_memory,data
	.export rom_fc_ff,data
	.export rom_cards,data
	.export dummy_memory1,data
	.align	0x100
slow_memory	.block	128*1024
dummy_memory1	.block	3*1024
rom_fc_ff	.block	256*1024
rom_cards	.block	256*16

