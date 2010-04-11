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

#ifdef ASM
# ifdef INCLUDE_RCSID_S
	.stringz "@(#)$KmKId: instable.h,v 1.104 2004-10-05 20:12:08-04 kentd Exp $"
# endif
#endif

inst00_SYM		/*  brk */
#ifdef ASM
	ldb	1(scratch1),ret0
	ldil	l%g_testing,arg3
	ldil	l%g_num_brk,arg1
	ldw	r%g_testing(arg3),arg3
	INC_KPC_2;
	ldw	r%g_num_brk(arg1),arg2
	comib,<> 0,arg3,brk_testing_SYM
	extru	kpc,31,16,arg0
	addi	1,arg2,arg2
	bb,>=	psr,23,brk_native_SYM
	stw	arg2,r%g_num_brk(arg1)

	bl	push_16,link
	nop

	bl	push_8,link
	extru	psr,31,8,arg0		;B bit already on in PSR

	ldil	l%0xfffe,arg0
	bl	get_mem_long_16,link
	ldo	r%0xfffe(arg0),arg0

	zdep	ret0,31,16,kpc		;set kbank to 0

#if 0
	bl	set_halt_act,link
	ldi	3,arg0
#endif


	ldi	0,dbank			;clear dbank in emul mode
	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off


brk_native_SYM
	stw	arg0,STACK_SAVE_COP_ARG0(sp)
	bl	push_8,link
	extru	kpc,15,8,arg0

	bl	push_16,link
	ldw	STACK_SAVE_COP_ARG0(sp),arg0

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xffe6,arg0
	bl	get_mem_long_16,link
	ldo	r%0xffe6(arg0),arg0

	zdep	ret0,31,16,kpc		;zero kbank in kpc

#if 0
#endif
	bl	set_halt_act,link
	ldi	3,arg0

	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off

brk_testing_SYM
	DEC_KPC2;
	CYCLES_PLUS_2
	b	dispatch_done
	depi	RET_BREAK,3,4,ret0

#else
	GET_1BYTE_ARG;
	if(g_testing) {
		CYCLES_PLUS_2;
		FINISH(RET_BREAK, arg);
	}
	g_num_brk++;
	INC_KPC_2;
	if(psr & 0x100) {
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xfffe, kpc, 0);
		dbank = 0;
	} else {
		PUSH8(kpc >> 16);
		PUSH16(kpc);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xffe6, kpc, 0);
		halt_printf("Halting for native break!\n");
	}
	kpc = kpc & 0xffff;
	psr |= 0x4;
	psr &= ~(0x8);
#endif

inst01_SYM		/*  ORA (Dloc,X) */
/*  called with arg = val to ORA in */
	GET_DLOC_X_IND_RD();
	ORA_INST();

inst02_SYM		/*  COP */
#ifdef ASM
	ldil	l%g_num_cop,arg1
	INC_KPC_2;
	ldw	r%g_num_cop(arg1),arg2
	extru	kpc,31,16,arg0
	addi	1,arg2,arg2
	bb,>=	psr,23,cop_native_SYM
	stw	arg2,r%g_num_cop(arg1)

	bl	push_16,link
	nop

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xfff4,arg0
	bl	get_mem_long_16,link
	ldo	r%0xfff4(arg0),arg0

	ldi	0,dbank			;clear dbank in emul mode
	zdep	ret0,31,16,kpc		;clear kbank

	bl	set_halt_act,link
	ldi	3,arg0

	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off

cop_native_SYM
	stw	arg0,STACK_SAVE_COP_ARG0(sp)
	bl	push_8,link
	extru	kpc,15,8,arg0

	bl	push_16,link
	ldw	STACK_SAVE_COP_ARG0(sp),arg0

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xffe4,arg0
	bl	get_mem_long_16,link
	ldo	r%0xffe4(arg0),arg0

	zdep	ret0,31,16,kpc		;clear kbank
	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off


#else
	g_num_cop++;
	INC_KPC_2;
	if(psr & 0x100) {
		halt_printf("Halting for emul COP at %04x\n", kpc);
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xfff4, kpc, 0);
		dbank = 0;
	} else {
		PUSH8(kpc >> 16);
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xffe4, kpc, 0);
	}
	kpc = kpc & 0xffff;
	psr |= 4;
	psr &= ~(0x8);
#endif

inst03_SYM		/*  ORA Disp8,S */
	GET_DISP8_S_RD();
	ORA_INST();

inst04_SYM		/*  TSB Dloc */
	GET_DLOC_RD();
	TSB_INST(1);

inst05_SYM		/*  ORA Dloc */
	GET_DLOC_RD();
	ORA_INST();

inst06_SYM		/*  ASL Dloc */
	GET_DLOC_RD();
	ASL_INST(1);

inst07_SYM		/*  ORA [Dloc] */
	GET_DLOC_L_IND_RD();
	ORA_INST();

inst08_SYM		/*  PHP */
#ifdef ASM
	dep	neg,24,1,psr
	ldil	l%dispatch,link
	INC_KPC_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	ldo	r%dispatch(link),link
	b	push_8
	extru	psr,31,8,arg0
#else
	INC_KPC_1;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	PUSH8(psr);
#endif

inst09_SYM		/*  ORA #imm */
	GET_IMM_MEM();
	ORA_INST();

inst0a_SYM		/*  ASL a */
#ifdef ASM
# ifdef ACC8
	ldi	0xff,scratch1
	sh1add	acc,0,scratch3
	INC_KPC_1
	extru	scratch3,24,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,23,1,scratch2
	dep	zero,31,8,acc
	b	dispatch
	dep	scratch2,31,1,psr		/* set carry */
# else
	zdepi	-1,31,16,scratch1
	sh1add	acc,0,scratch3
	INC_KPC_1
	extru	scratch3,16,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,15,1,scratch2
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	tmp1 = acc + acc;
# ifdef ACC8
	SET_CARRY8(tmp1);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	SET_CARRY16(tmp1);
	acc = tmp1 & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

inst0b_SYM		/*  PHD */
#ifdef ASM
	ldil	l%dispatch,link
	extru	direct,31,16,arg0
	INC_KPC_1
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	INC_KPC_1;
	PUSH16_UNSAFE(direct);
#endif

inst0c_SYM		/*  TSB abs */
	GET_ABS_RD();
	TSB_INST(0);

inst0d_SYM		/*  ORA abs */
	GET_ABS_RD();
	ORA_INST();

inst0e_SYM		/*  ASL abs */
	GET_ABS_RD();
	ASL_INST(0);

inst0f_SYM		/*  ORA long */
	GET_LONG_RD();
	ORA_INST();


inst10_SYM		/*  BPL disp8 */
#ifdef ASM
	COND_BR1
	comib,<> 0,neg,inst10_2_SYM
	COND_BR2

inst10_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(neg == 0);
#endif

inst11_SYM		/*  ORA (Dloc),y */
	GET_DLOC_IND_Y_RD();
	ORA_INST();

inst12_SYM		/*  ORA (Dloc) */
	GET_DLOC_IND_RD();
	ORA_INST();

inst13_SYM		/*  ORA (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	ORA_INST();

inst14_SYM		/*  TRB Dloc */
	GET_DLOC_RD();
	TRB_INST(1);

inst15_SYM		/*  ORA Dloc,x */
	GET_DLOC_X_RD();
	ORA_INST();

inst16_SYM		/*  ASL Dloc,X */
	GET_DLOC_X_RD();
	ASL_INST(1);

inst17_SYM		/*  ORA [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	ORA_INST();

inst18_SYM		/*  CLC */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,31,1,psr		/* clear carry */
#else
	psr = psr & (~1);
	INC_KPC_1;
#endif

inst19_SYM		/*  ORA abs,y */
	GET_ABS_Y_RD();
	ORA_INST();


inst1a_SYM		/*  INC a */
#ifdef ASM
# ifdef ACC8
	ldi	0xff,scratch2
	addi	1,acc,scratch1
	extru	scratch1,24,1,neg
	INC_KPC_1
	extru	scratch1,31,8,zero
	b	dispatch
	dep	zero,31,8,acc
# else
	zdepi	-1,31,16,scratch2
	addi	1,acc,scratch1
	extru	scratch1,16,1,neg
	INC_KPC_1
	extru	scratch1,31,16,zero
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	acc = (acc & 0xff00) | ((acc + 1) & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	acc = (acc + 1) & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

inst1b_SYM		/*  TCS */
#ifdef ASM
	copy	acc,stack
	extru,=	psr,23,1,0		/* in emulation mode, stack page 1 */
	depi	1,23,24,stack
	INC_KPC_1
	b	dispatch
	nop
#else
	stack = acc;
	INC_KPC_1;
	if(psr & 0x100) {
		stack = (stack & 0xff) + 0x100;
	}
#endif

inst1c_SYM		/*  TRB Abs */
	GET_ABS_RD();
	TRB_INST(0);

inst1d_SYM		/*  ORA Abs,X */
	GET_ABS_X_RD();
	ORA_INST();

inst1e_SYM		/*  ASL Abs,X */
	GET_ABS_X_RD_WR();
	ASL_INST(0);

inst1f_SYM		/*  ORA Long,X */
	GET_LONG_X_RD();
	ORA_INST();


inst20_SYM		/*  JSR abs */
#ifdef ASM
	addi	2,kpc,arg0
	ldb	1(scratch1),scratch2
	CYCLES_PLUS_2
	ldb	2(scratch1),scratch1
	ldil	l%dispatch,link
	extru	arg0,31,16,arg0
	ldo	r%dispatch(link),link
	dep	scratch2,31,8,kpc
	b	push_16
	dep	scratch1,23,8,kpc
#else
	GET_2BYTE_ARG;
	INC_KPC_2;
	PUSH16(kpc);
	kpc = (kpc & 0xff0000) + arg;
	CYCLES_PLUS_2;
#endif

inst21_SYM		/*  AND (Dloc,X) */
/*  called with arg = val to AND in */
	GET_DLOC_X_IND_RD();
	AND_INST();

inst22_SYM		/*  JSL Long */
#ifdef ASM
	INC_KPC_3
	ldb	3(scratch1),scratch2
	copy	kpc,arg0
	ldb	1(scratch1),kpc
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_3
	dep	scratch2,15,8,kpc
	stw	scratch2,STACK_SAVE_INSTR_TMP1(sp)
	bl	push_24_unsafe,link
	dep	scratch1,23,8,kpc

	b	dispatch
	nop
#else
	GET_3BYTE_ARG;
	tmp1 = arg;
	CYCLES_PLUS_3;
	INC_KPC_3;
	PUSH24_UNSAFE(kpc);
	kpc = tmp1 & 0xffffff;
#endif

inst23_SYM		/*  AND Disp8,S */
/*  called with arg = val to AND in */
	GET_DISP8_S_RD();
	AND_INST();

inst24_SYM		/*  BIT Dloc */
	GET_DLOC_RD();
	BIT_INST();

inst25_SYM		/*  AND Dloc */
/*  called with arg = val to AND in */
	GET_DLOC_RD();
	AND_INST();

inst26_SYM		/*  ROL Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	ROL_INST(1);

inst27_SYM		/*  AND [Dloc] */
	GET_DLOC_L_IND_RD();
	AND_INST();

inst28_SYM		/*  PLP */
#ifdef ASM
	bl	pull_8,link
	ldi	0,zero

	extru	psr,27,2,scratch2		/* save old x & m */
	dep	ret0,31,8,psr
	CYCLES_PLUS_1
	INC_KPC_1
	extru,<> ret0,30,1,0
	ldi	1,zero
	copy	scratch2,arg0
	b	update_system_state
	extru	ret0,24,1,neg
#else
	PULL8(tmp1);
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_1;
	psr = (psr & ~0xff) | (tmp1 & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
#endif
	

inst29_SYM		/*  AND #imm */
	GET_IMM_MEM();
	AND_INST();

inst2a_SYM		/*  ROL a */
#ifdef ASM
# ifdef ACC8
	extru	psr,31,1,scratch2
	ldi	0xff,scratch1
	sh1add	acc,scratch2,scratch3
	INC_KPC_1
	extru	scratch3,24,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,23,1,scratch2
	dep	zero,31,8,acc
	b	dispatch
	dep	scratch2,31,1,psr		/* set carry */
# else
	extru	psr,31,1,scratch2
	INC_KPC_1
	sh1add	acc,scratch2,scratch3
	zdepi	-1,31,16,scratch1
	extru	scratch3,16,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,15,1,scratch2
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) << 1) + (psr & 1);
	SET_CARRY8(tmp1);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc << 1) + (psr & 1);
	SET_CARRY16(tmp1);
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

inst2b_SYM		/*  PLD */
#ifdef ASM
	INC_KPC_1
	bl	pull_16_unsafe,link
	CYCLES_PLUS_1
	extru	ret0,31,16,direct
	extru	ret0,16,1,neg
	b	dispatch
	copy	direct,zero
#else
	INC_KPC_1;
	PULL16_UNSAFE(direct);
	CYCLES_PLUS_1;
	SET_NEG_ZERO16(direct);
#endif

inst2c_SYM		/*  BIT abs */
	GET_ABS_RD();
	BIT_INST();

inst2d_SYM		/*  AND abs */
	GET_ABS_RD();
	AND_INST();

inst2e_SYM		/*  ROL abs */
	GET_ABS_RD();
	ROL_INST(0);

inst2f_SYM		/*  AND long */
	GET_LONG_RD();
	AND_INST();


inst30_SYM		/*  BMI disp8 */
#ifdef ASM
	COND_BR1
	comib,= 0,neg,inst30_2_SYM
	COND_BR2

inst30_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(neg);
#endif

inst31_SYM		/*  AND (Dloc),y */
	GET_DLOC_IND_Y_RD();
	AND_INST();

inst32_SYM		/*  AND (Dloc) */
	GET_DLOC_IND_RD();
	AND_INST();

inst33_SYM		/*  AND (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	AND_INST();

inst34_SYM		/*  BIT Dloc,x */
	GET_DLOC_X_RD();
	BIT_INST();

inst35_SYM		/*  AND Dloc,x */
	GET_DLOC_X_RD();
	AND_INST();

inst36_SYM		/*  ROL Dloc,X */
	GET_DLOC_X_RD();
	ROL_INST(1);

inst37_SYM		/*  AND [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	AND_INST();

inst38_SYM		/*  SEC */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,31,1,psr		/* set carry */
#else
	psr = psr | 1;
	INC_KPC_1;
#endif

inst39_SYM		/*  AND abs,y */
	GET_ABS_Y_RD();
	AND_INST();

inst3a_SYM		/*  DEC a */
#ifdef ASM
# ifdef ACC8
	addi	-1,acc,scratch1
	extru	scratch1,24,1,neg
	INC_KPC_1
	extru	scratch1,31,8,zero
	b	dispatch
	dep	zero,31,8,acc
# else
	addi	-1,acc,scratch1
	extru	scratch1,16,1,neg
	INC_KPC_1
	extru	scratch1,31,16,zero
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	acc = (acc & 0xff00) | ((acc - 1) & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	acc = (acc - 1) & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

inst3b_SYM		/*  TSC */
/*  set N,Z according to 16 bit acc */
#ifdef ASM
	copy	stack,acc
	extru	stack,16,1,neg
	INC_KPC_1
	b	dispatch
	extru	acc,31,16,zero
#else
	INC_KPC_1;
	acc = stack;
	SET_NEG_ZERO16(acc);
#endif

inst3c_SYM		/*  BIT Abs,x */
	GET_ABS_X_RD();
	BIT_INST();

inst3d_SYM		/*  AND Abs,X */
	GET_ABS_X_RD();
	AND_INST();

inst3e_SYM		/*  ROL Abs,X */
	GET_ABS_X_RD_WR();
	ROL_INST(0);

inst3f_SYM		/*  AND Long,X */
	GET_LONG_X_RD();
	AND_INST();


inst40_SYM		/*  RTI */
#ifdef ASM
	bb,>=	psr,23,rti_native_SYM
	CYCLES_PLUS_1
/*  emulation */
	bl	pull_24,link
	ldi	0,zero

	extru	psr,27,2,scratch2
	extru	ret0,23,16,scratch3
	copy	scratch2,arg0
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr

	extru	ret0,24,1,neg
	b	update_system_state
	dep	scratch3,31,16,kpc

rti_native_SYM
	bl	pull_8,link
	ldi	0,zero

	copy	ret0,scratch1
	extru	ret0,24,1,neg
	dep	ret0,31,8,scratch1
	bl	pull_24,link
	stw	scratch1,STACK_SAVE_INSTR_TMP1(sp)

	extru	psr,27,2,scratch2
	ldw	STACK_SAVE_INSTR_TMP1(sp),psr
	extru	ret0,31,24,kpc
	extru,<> psr,30,1,0
	ldi	1,zero

	b	update_system_state_and_change_kbank
	copy	scratch2,arg0
#else
	CYCLES_PLUS_1
	if(psr & 0x100) {
		PULL24(tmp1);
		kpc = (kpc & 0xff0000) + ((tmp1 >> 8) & 0xffff);
		tmp2 = psr;
		psr = (psr & ~0xff) + (tmp1 & 0xff);
		neg = (psr >> 7) & 1;
		zero = !(psr & 2);
		UPDATE_PSR(psr, tmp2);
	} else {
		PULL8(tmp1);
		tmp2 = psr;
		psr = (tmp1 & 0xff);
		neg = (psr >> 7) & 1;
		zero = !(psr & 2);
		PULL24(kpc);
		UPDATE_PSR(psr, tmp2);
	}
#endif


inst41_SYM		/*  EOR (Dloc,X) */
/*  called with arg = val to EOR in */
	GET_DLOC_X_IND_RD();
	EOR_INST();

inst42_SYM		/*  WDM */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_5
	CYCLES_PLUS_2
	INC_KPC_2
	b	dispatch_done
	depi	RET_WDM,3,4,ret0
#else
	GET_1BYTE_ARG;
	INC_KPC_2;
	CYCLES_PLUS_5;
	CYCLES_PLUS_2;
	FINISH(RET_WDM, arg & 0xff);
#endif

inst43_SYM		/*  EOR Disp8,S */
/*  called with arg = val to EOR in */
	GET_DISP8_S_RD();
	EOR_INST();

inst44_SYM		/*  MVP */
#ifdef ASM
	ldb	2(scratch1),scratch2		/* src bank */
	bb,<	psr,23,inst44_notnat_SYM
	ldb	1(scratch1),dbank		/* dest bank */
	bb,<	psr,27,inst44_notnat_SYM
	stw	scratch2,STACK_SRC_BANK(sp)

inst44_loop_SYM
	CYCLES_PLUS_1
	ldw	STACK_SRC_BANK(sp),scratch2
	copy	xreg,arg0

	bl	get_mem_long_8,link
	dep	scratch2,15,8,arg0
/*  got byte */
	copy	ret0,arg1
	copy	yreg,arg0
	bl	set_mem_long_8,link
	dep	dbank,15,8,arg0
/*  wrote byte, dec acc */
	CYCLES_PLUS_2
	fldds	0(fcycles_stop_ptr),fcycles_stop
	addi	-1,xreg,xreg
	zdepi	-1,31,16,scratch2
	addi	-1,yreg,yreg
	addi	-1,acc,acc
	fcmp,<,dbl fcycles,fcycles_stop
	and	xreg,scratch2,xreg
	extrs	acc,31,16,scratch1
	and	yreg,scratch2,yreg

	comib,= -1,scratch1,inst44_done_SYM
	and	acc,scratch2,acc

	ftest
	b	inst44_out_of_time_SYM

	CYCLES_PLUS_2
	b	inst44_loop_SYM
	nop

/*  get here if done */
inst44_done_SYM
	INC_KPC_3
	b	dispatch
	nop

inst44_notnat_SYM
	copy	dbank,ret0
	dep	scratch2,23,8,ret0
	CYCLES_PLUS_3
	depi	RET_MVP,3,4,ret0
	b	dispatch_done
	CYCLES_PLUS_2

inst44_out_of_time_SYM
/*  cycle have gone positive, just get out, do not update kpc */
	b,n	dispatch
#else
	GET_2BYTE_ARG;
	/* arg & 0xff = dest bank, arg & 0xff00 = src bank */
	if(psr & 0x110) {
		halt_printf("MVP but not native m or x!\n");
		break;
	}
	CYCLES_MINUS_2
	dbank = arg & 0xff;
	tmp1 = (arg >> 8) & 0xff;
	while(1) {
		CYCLES_PLUS_3;
		GET_MEMORY8((tmp1 << 16) + xreg, arg);
		SET_MEMORY8((dbank << 16) + yreg, arg);
		CYCLES_PLUS_2;
		xreg = (xreg - 1) & 0xffff;
		yreg = (yreg - 1) & 0xffff;
		acc = (acc - 1) & 0xffff;
		if(acc == 0xffff) {
			INC_KPC_3;
			break;
		}
		if(fcycles >= g_fcycles_stop) {
			break;
		}
	}
#endif


inst45_SYM		/*  EOR Dloc */
/*  called with arg = val to EOR in */
	GET_DLOC_RD();
	EOR_INST();

inst46_SYM		/*  LSR Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	LSR_INST(1);

inst47_SYM		/*  EOR [Dloc] */
	GET_DLOC_L_IND_RD();
	EOR_INST();

inst48_SYM		/*  PHA */
#ifdef ASM
# ifdef ACC8
	INC_KPC_1
	ldil	l%dispatch,link
	extru	acc,31,8,arg0
	b	push_8
	ldo	r%dispatch(link),link
# else
	INC_KPC_1
	ldil	l%dispatch,link
	extru	acc,31,16,arg0
	b	push_16
	ldo	r%dispatch(link),link
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	PUSH8(acc);
# else
	PUSH16(acc);
# endif
#endif

inst49_SYM		/*  EOR #imm */
	GET_IMM_MEM();
	EOR_INST();

inst4a_SYM		/*  LSR a */
#ifdef ASM
# ifdef ACC8
	extru	acc,31,1,scratch2
	INC_KPC_1
	extru	acc,30,7,zero
	ldi	0,neg
	dep	scratch2,31,1,psr		/* set carry */
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	acc,31,1,scratch2
	INC_KPC_1
	extru	acc,30,15,zero
	ldi	0,neg
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) >> 1);
	SET_CARRY8(acc << 8);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc >> 1);
	SET_CARRY8((acc << 8));
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

inst4b_SYM		/*  PHK */
#ifdef ASM
	ldil	l%dispatch,link
	extru	kpc,15,8,arg0
	INC_KPC_1
	b	push_8
	ldo	r%dispatch(link),link
#else
	PUSH8(kpc >> 16);
	INC_KPC_1;
#endif

inst4c_SYM		/*  JMP abs */
#ifdef ASM
	ldb	1(scratch1),scratch2
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	dep	scratch2,31,8,kpc
	b	dispatch
	dep	scratch1,23,8,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = (kpc & 0xff0000) + arg;
#endif
	

inst4d_SYM		/*  EOR abs */
	GET_ABS_RD();
	EOR_INST();

inst4e_SYM		/*  LSR abs */
	GET_ABS_RD();
	LSR_INST(0);

inst4f_SYM		/*  EOR long */
	GET_LONG_RD();
	EOR_INST();


inst50_SYM		/*  BVC disp8 */
#ifdef ASM
	COND_BR1
	bb,<	psr,25,inst50_2_SYM
	COND_BR2

inst50_2_SYM
	COND_BR_UNTAKEN

#else
	BRANCH_DISP8((psr & 0x40) == 0);
#endif

inst51_SYM		/*  EOR (Dloc),y */
	GET_DLOC_IND_Y_RD();
	EOR_INST();

inst52_SYM		/*  EOR (Dloc) */
	GET_DLOC_IND_RD();
	EOR_INST();

inst53_SYM		/*  EOR (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	EOR_INST();

inst54_SYM		/*  MVN  */
#ifdef ASM
	ldb	2(scratch1),scratch2		/* src bank */
	bb,<	psr,23,inst54_notnat_SYM
	ldb	1(scratch1),dbank		/* dest bank */
	bb,<	psr,27,inst54_notnat_SYM
	stw	scratch2,STACK_SRC_BANK(sp)

/*  even in 8bit acc mode, use 16-bit accumulator! */

inst54_loop_SYM
	CYCLES_PLUS_1
	ldw	STACK_SRC_BANK(sp),scratch2
	copy	xreg,arg0

	bl	get_mem_long_8,link
	dep	scratch2,15,8,arg0
/*  got byte */
	copy	ret0,arg1
	copy	yreg,arg0
	bl	set_mem_long_8,link
	dep	dbank,15,8,arg0
/*  wrote byte, dec acc */
	CYCLES_PLUS_2
	fldds	0(fcycles_stop_ptr),fcycles_stop
	addi	1,xreg,xreg
	zdepi	-1,31,16,scratch2
	addi	1,yreg,yreg
	addi	-1,acc,acc
	fcmp,<,dbl fcycles,fcycles_stop
	and	xreg,scratch2,xreg
	extrs	acc,31,16,scratch1
	and	yreg,scratch2,yreg

	comib,=	-1,scratch1,inst54_done_SYM
	and	acc,scratch2,acc
	ftest
	b,n	inst54_out_of_time_SYM

	CYCLES_PLUS_2
	b	inst54_loop_SYM
	nop

/*  get here if done */
inst54_done_SYM
	INC_KPC_3
	b	dispatch
	nop

inst54_out_of_time_SYM
/*  cycle have gone positive, just get out, don't update kpc */
	b,n	dispatch

inst54_notnat_SYM
	copy	dbank,ret0
	dep	scratch2,23,8,ret0
	CYCLES_PLUS_3
	depi	RET_MVN,3,4,ret0
	b	dispatch_done
	CYCLES_PLUS_3
#else
	GET_2BYTE_ARG;
	/* arg & 0xff = dest bank, arg & 0xff00 = src bank */
	if(psr & 0x110) {
		halt_printf("MVN but not native m or x!\n");
		break;
	}
	CYCLES_MINUS_2;
	dbank = arg & 0xff;
	tmp1 = (arg >> 8) & 0xff;
	while(1) {
		CYCLES_PLUS_3;
		GET_MEMORY8((tmp1 << 16) + xreg, arg);
		SET_MEMORY8((dbank << 16) + yreg, arg);
		CYCLES_PLUS_2;
		xreg = (xreg + 1) & 0xffff;
		yreg = (yreg + 1) & 0xffff;
		acc = (acc - 1) & 0xffff;
		if(acc == 0xffff) {
			INC_KPC_3;
			break;
		}
		if(fcycles >= g_fcycles_stop) {
			break;
		}
	}
#endif

inst55_SYM		/*  EOR Dloc,x */
	GET_DLOC_X_RD();
	EOR_INST();

inst56_SYM		/*  LSR Dloc,X */
	GET_DLOC_X_RD();
	LSR_INST(1);

inst57_SYM		/*  EOR [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	EOR_INST();

inst58_SYM		/*  CLI */
#ifdef ASM
	INC_KPC_1
	b	check_irqs_pending	/* check for ints pending! */
	depi	0,29,1,psr		/* clear int disable */
#else
	psr = psr & (~4);
	INC_KPC_1;
	if(((psr & 0x4) == 0) && g_irq_pending) {
		FINISH(RET_IRQ, 0);
	}
#endif

inst59_SYM		/*  EOR abs,y */
	GET_ABS_Y_RD();
	EOR_INST();

inst5a_SYM		/*  PHY */
#ifdef ASM
	INC_KPC_1
	ldil	l%dispatch,link
	bb,>=	psr,27,phy_16_SYM
	ldo	r%dispatch(link),link

	b	push_8
	copy	yreg,arg0

phy_16_SYM
	b	push_16
	copy	yreg,arg0
#else
	INC_KPC_1;
	if(psr & 0x10) {
		PUSH8(yreg);
	} else {
		PUSH16(yreg);
	}
#endif

inst5b_SYM		/*  TCD */
#ifdef ASM
	extru	acc,31,16,direct
	INC_KPC_1
	copy	acc,zero
	b	dispatch
	extru	acc,16,1,neg
#else
	INC_KPC_1;
	direct = acc;
	SET_NEG_ZERO16(acc);
#endif

inst5c_SYM		/*  JMP Long */
#ifdef ASM
	ldb	1(scratch1),kpc
	ldb	2(scratch1),scratch2
	CYCLES_PLUS_1
	ldb	3(scratch1),arg0		/* new bank */
	dep	scratch2,23,8,kpc
	b	dispatch
	dep	arg0,15,8,kpc
#else
	GET_3BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = arg;
#endif

inst5d_SYM		/*  EOR Abs,X */
	GET_ABS_X_RD();
	EOR_INST();

inst5e_SYM		/*  LSR Abs,X */
	GET_ABS_X_RD_WR();
	LSR_INST(0);

inst5f_SYM		/*  EOR Long,X */
	GET_LONG_X_RD();
	EOR_INST();


inst60_SYM		/*  RTS */
#ifdef ASM
	bl	pull_16,link
	CYCLES_PLUS_2
/*  ret0 is new kpc-1 */
	addi	1,ret0,ret0
	b	dispatch
	dep	ret0,31,16,kpc
#else
	CYCLES_PLUS_2
	PULL16(tmp1);
	kpc = (kpc & 0xff0000) + ((tmp1 + 1) & 0xffff);
#endif


inst61_SYM		/*  ADC (Dloc,X) */
/*  called with arg = val to ADC in */
	GET_DLOC_X_IND_RD();
	ADC_INST();

inst62_SYM		/*  PER */
#ifdef ASM
	ldb	1(scratch1),ret0
	INC_KPC_3
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_2
	ldil	l%dispatch,link
	dep	scratch1,23,8,ret0
	ldo	r%dispatch(link),link
	add	kpc,ret0,arg0
	b	push_16_unsafe
	extru	arg0,31,16,arg0
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_2;
	INC_KPC_3;
	PUSH16_UNSAFE(kpc + arg);
#endif

inst63_SYM		/*  ADC Disp8,S */
/*  called with arg = val to ADC in */
	GET_DISP8_S_RD();
	ADC_INST();

inst64_SYM		/*  STZ Dloc */
	GET_DLOC_ADDR();
	STZ_INST(1);

inst65_SYM		/*  ADC Dloc */
/*  called with arg = val to ADC in */
	GET_DLOC_RD();
	ADC_INST();

inst66_SYM		/*  ROR Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	ROR_INST(1);

inst67_SYM		/*  ADC [Dloc] */
	GET_DLOC_L_IND_RD();
	ADC_INST();

inst68_SYM		/*  PLA */
#ifdef ASM
# ifdef ACC8
	INC_KPC_1
	bl	pull_8,link
	CYCLES_PLUS_1
	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	dep	ret0,31,8,acc
# else
	INC_KPC_1
	bl	pull_16,link
	CYCLES_PLUS_1

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	extru	ret0,31,16,acc
# endif
#else
	INC_KPC_1;
	CYCLES_PLUS_1;
# ifdef ACC8
	PULL8(tmp1);
	acc = (acc & 0xff00) + tmp1;
	SET_NEG_ZERO8(tmp1);
# else
	PULL16(tmp1);
	acc = tmp1;
	SET_NEG_ZERO16(tmp1);
# endif
#endif


inst69_SYM		/*  ADC #imm */
	GET_IMM_MEM();
	ADC_INST();

inst6a_SYM		/*  ROR a */
#ifdef ASM
# ifdef ACC8
	extru	psr,31,1,neg
	INC_KPC_1
	extru	acc,30,7,zero
	dep	neg,24,1,zero
	dep	acc,31,1,psr			/* set carry */
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	psr,31,1,neg
	INC_KPC_1
	extru	acc,30,15,zero
	dep	neg,16,1,zero
	dep	acc,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) >> 1) + ((psr & 1) << 7);
	SET_CARRY8((acc << 8));
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc >> 1) + ((psr & 1) << 15);
	SET_CARRY16((acc << 16));
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

inst6b_SYM		/*  RTL */
#ifdef ASM
	bl	pull_24,link
	CYCLES_PLUS_1
/*  ret0 is new kpc-1 */
	copy	ret0,kpc
	addi	1,ret0,scratch1
	b	dispatch
	dep	scratch1,31,16,kpc
	
#else
	CYCLES_PLUS_1;
	PULL24(tmp1);
	kpc = (tmp1 & 0xff0000) + ((tmp1 + 1) & 0xffff);
#endif

inst6c_SYM		/*  JMP (abs) */
#ifdef ASM
	ldb	1(scratch1),arg0
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	bl	get_mem_long_16,link
	dep	scratch1,23,8,arg0
/*  ret0 is addr to jump to */
	b	dispatch
	dep	ret0,31,16,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	GET_MEMORY16(arg, tmp1, 1);
	kpc = (kpc & 0xff0000) + tmp1;
#endif

inst6d_SYM		/*  ADC abs */
	GET_ABS_RD();
	ADC_INST();

inst6e_SYM		/*  ROR abs */
	GET_ABS_RD();
	ROR_INST(0);

inst6f_SYM		/*  ADC long */
	GET_LONG_RD();
	ADC_INST();


inst70_SYM		/*  BVS disp8 */
#ifdef ASM
	COND_BR1
	bb,>=	psr,25,inst70_2_SYM
	COND_BR2

inst70_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x40));
#endif

inst71_SYM		/*  ADC (Dloc),y */
	GET_DLOC_IND_Y_RD();
	ADC_INST();

inst72_SYM		/*  ADC (Dloc) */
	GET_DLOC_IND_RD();
	ADC_INST();

inst73_SYM		/*  ADC (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	ADC_INST();

inst74_SYM		/*  STZ Dloc,x */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_X_WR();
	STZ_INST(1);
#else
	GET_1BYTE_ARG;
	GET_DLOC_X_WR();
	STZ_INST(1);
#endif

inst75_SYM		/*  ADC Dloc,x */
	GET_DLOC_X_RD();
	ADC_INST();

inst76_SYM		/*  ROR Dloc,X */
	GET_DLOC_X_RD();
	ROR_INST(1);

inst77_SYM		/*  ADC [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	ADC_INST();

inst78_SYM		/*  SEI */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,29,1,psr		/* set int disable */
#else
	psr = psr | 4;
	INC_KPC_1;
#endif

inst79_SYM		/*  ADC abs,y */
	GET_ABS_Y_RD();
	ADC_INST();

inst7a_SYM		/*  PLY */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,inst7a_16bit_SYM
	nop

	bl	pull_8,link
	CYCLES_PLUS_1

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,yreg

inst7a_16bit_SYM
	bl	pull_16,link
	CYCLES_PLUS_1

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,yreg

#else
	INC_KPC_1;
	CYCLES_PLUS_1
	if(psr & 0x10) {
		PULL8(yreg);
		SET_NEG_ZERO8(yreg);
	} else {
		PULL16(yreg);
		SET_NEG_ZERO16(yreg);
	}
#endif

inst7b_SYM		/*  TDC */
#ifdef ASM
	extru	direct,31,16,zero
	copy	direct,acc
	INC_KPC_1
	b	dispatch
	extru	direct,16,1,neg
#else
	INC_KPC_1;
	acc = direct;
	SET_NEG_ZERO16(direct);
#endif

inst7c_SYM		/*  JMP (Abs,x) */
/*  always access kbank, xreg cannot wrap into next bank */
#ifdef ASM
	ldb	1(scratch1),ret0
	copy	kpc,scratch2
	ldb	2(scratch1),scratch1
	dep	xreg,31,16,scratch2
	CYCLES_PLUS_2
	dep	scratch1,23,8,ret0
	add	ret0,scratch2,arg0
	bl	get_mem_long_16,link
	extru	arg0,31,24,arg0
	b	dispatch
	dep	ret0,31,16,kpc
#else
	GET_2BYTE_ARG;
	arg = (kpc & 0xff0000) + ((xreg + arg) & 0xffff);
	CYCLES_PLUS_2;
	GET_MEMORY16(arg, tmp1, 1);
	kpc = (kpc & 0xff0000) + tmp1;
#endif

inst7d_SYM		/*  ADC Abs,X */
	GET_ABS_X_RD();
	ADC_INST();

inst7e_SYM		/*  ROR Abs,X */
	GET_ABS_X_RD_WR();
	ROR_INST(0);

inst7f_SYM		/*  ADC Long,X */
	GET_LONG_X_RD();
	ADC_INST();


inst80_SYM		/*  BRA */
#ifdef ASM
	COND_BR1
	COND_BR2
#else
	BRANCH_DISP8(1);
#endif


inst81_SYM		/*  STA (Dloc,X) */
	GET_DLOC_X_IND_ADDR();
	STA_INST(0);

inst82_SYM		/*  BRL disp16 */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	addi	3,kpc,scratch2
	dep	scratch1,23,8,ret0
	add	ret0,scratch2,scratch2
	b	dispatch
	dep	scratch2,31,16,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = (kpc & 0xff0000) + ((kpc + 3 + arg) & 0xffff);
#endif

inst83_SYM		/*  STA Disp8,S */
	GET_DISP8_S_ADDR();
	STA_INST(1);

inst84_SYM		/*  STY Dloc */
	GET_DLOC_ADDR();
	STY_INST(1);


inst85_SYM		/*  STA Dloc */
	GET_DLOC_ADDR();
	STA_INST(1);

inst86_SYM		/*  STX Dloc */
	GET_DLOC_ADDR();
	STX_INST(1);


inst87_SYM		/*  STA [Dloc] */
	GET_DLOC_L_IND_ADDR();
	STA_INST(0);

inst88_SYM		/*  DEY */
#ifdef ASM
	INC_KPC_1
	bb,<	psr,27,inst88_8bit_SYM
	addi	-1,yreg,yreg
/*  16 bit */
	extru	yreg,31,16,zero
	extru	yreg,16,1,neg
	b	dispatch
	copy	zero,yreg

inst88_8bit_SYM
	extru	yreg,31,8,zero
	extru	yreg,24,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(yreg - 1, yreg);
#endif

inst89_SYM		/*  BIT #imm */
#ifdef ASM
	GET_IMM_MEM();
# ifdef ACC8
/* Immediate BIT does not set condition flags */
	and	acc,ret0,zero
	b	dispatch
	extru	zero,31,8,zero
# else
	and	acc,ret0,zero
	b	dispatch
	extru	zero,31,16,zero
# endif
#else
	GET_IMM_MEM();
# ifdef ACC8
	zero = (acc & arg) & 0xff;
# else
	zero = (acc & arg) & 0xffff;
# endif
#endif

inst8a_SYM		/*  TXA */
#ifdef ASM
# ifdef ACC8
	extru	xreg,31,8,zero
	INC_KPC_1
	extru	xreg,24,1,neg
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	xreg,31,16,zero
	INC_KPC_1
	extru	xreg,16,1,neg
	b	dispatch
	zdep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	arg = xreg;
	LDA_INST();
#endif

inst8b_SYM		/*  PHB */
#ifdef ASM
	ldil	l%dispatch,link
	extru	dbank,31,8,arg0
	INC_KPC_1
	b	push_8
	ldo	r%dispatch(link),link
#else
	INC_KPC_1;
	PUSH8(dbank);
#endif

inst8c_SYM		/*  STY abs */
	GET_ABS_ADDR();
	STY_INST(0);

inst8d_SYM		/*  STA abs */
	GET_ABS_ADDR();
	STA_INST(0);

inst8e_SYM		/*  STX abs */
	GET_ABS_ADDR();
	STX_INST(0);


inst8f_SYM		/*  STA long */
	GET_LONG_ADDR();
	STA_INST(0);


inst90_SYM		/*  BCC disp8 */
#ifdef ASM
	COND_BR1
	bb,<	psr,31,inst90_2_SYM
	COND_BR2

inst90_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x01) == 0);
#endif


inst91_SYM		/*  STA (Dloc),y */
	GET_DLOC_IND_Y_ADDR_FOR_WR();
	STA_INST(0);

inst92_SYM		/*  STA (Dloc) */
	GET_DLOC_IND_ADDR();
	STA_INST(0);

inst93_SYM		/*  STA (Disp8,s),y */
	GET_DISP8_S_IND_Y_ADDR();
	STA_INST(0);

inst94_SYM		/*  STY Dloc,x */
	GET_DLOC_X_ADDR();
	STY_INST(1);

inst95_SYM		/*  STA Dloc,x */
	GET_DLOC_X_ADDR();
	STA_INST(1);

inst96_SYM		/*  STX Dloc,Y */
	GET_DLOC_Y_ADDR();
	STX_INST(1);

inst97_SYM		/*  STA [Dloc],Y */
	GET_DLOC_L_IND_Y_ADDR();
	STA_INST(0);

inst98_SYM		/*  TYA */
#ifdef ASM
# ifdef ACC8
	extru	yreg,31,8,zero
	INC_KPC_1
	extru	yreg,24,1,neg
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	yreg,31,16,zero
	INC_KPC_1
	extru	yreg,16,1,neg
	b	dispatch
	zdep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	arg = yreg;
	LDA_INST();
#endif

inst99_SYM		/*  STA abs,y */
	GET_ABS_INDEX_ADDR_FOR_WR(yreg)
	STA_INST(0);

inst9a_SYM		/*  TXS */
#ifdef ASM
	copy	xreg,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	INC_KPC_1
	b	dispatch
	nop
#else
	stack = xreg;
	if(psr & 0x100) {
		stack = 0x100 | (stack & 0xff);
	}
	INC_KPC_1;
#endif


inst9b_SYM		/*  TXY */
#ifdef ASM
	extru	xreg,24,1,neg
	INC_KPC_1
	extru,<> psr,27,1,0		;skip next if 8bit
	extru	xreg,16,1,neg
	copy	xreg,yreg
	b	dispatch
	copy	xreg,zero
#else
	SET_INDEX_REG(xreg, yreg);
	INC_KPC_1;
#endif


inst9c_SYM		/*  STZ Abs */
	GET_ABS_ADDR();
	STZ_INST(0);

inst9d_SYM		/*  STA Abs,X */
	GET_ABS_INDEX_ADDR_FOR_WR(xreg);
	STA_INST(0);

inst9e_SYM		/*  STZ Abs,X */
	GET_ABS_INDEX_ADDR_FOR_WR(xreg);
	STZ_INST(0);

inst9f_SYM		/*  STA Long,X */
	GET_LONG_X_ADDR_FOR_WR();
	STA_INST(0);


insta0_SYM		/*  LDY #imm */
#ifdef ASM
	INC_KPC_2
	bb,>=	psr,27,insta0_16bit_SYM
	ldb	1(scratch1),zero

	extru	zero,24,1,neg
	b	dispatch
	copy	zero,yreg
insta0_16bit_SYM
	ldb	2(scratch1),scratch1
	INC_KPC_1
	CYCLES_PLUS_1
	extru	scratch1,24,1,neg
	dep	scratch1,23,8,zero
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_2;
	if((psr & 0x10) == 0) {
		GET_2BYTE_ARG;
		CYCLES_PLUS_1
		INC_KPC_1;
	} else {
		GET_1BYTE_ARG;
	}
	SET_INDEX_REG(arg, yreg);
#endif


insta1_SYM		/*  LDA (Dloc,X) */
/*  called with arg = val to LDA in */
	GET_DLOC_X_IND_RD();
	LDA_INST();

insta2_SYM		/*  LDX #imm */
#ifdef ASM
	ldb	1(scratch1),zero
	bb,>=	psr,27,insta2_16bit_SYM
	INC_KPC_2;

	extru	zero,24,1,neg
	b	dispatch
	copy	zero,xreg
insta2_16bit_SYM
	ldb	2(scratch1),scratch1
	INC_KPC_1
	CYCLES_PLUS_1
	extru	scratch1,24,1,neg
	dep	scratch1,23,8,zero
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_2;
	if((psr & 0x10) == 0) {
		GET_2BYTE_ARG;
		CYCLES_PLUS_1
		INC_KPC_1;
	} else {
		GET_1BYTE_ARG;
	}
	SET_INDEX_REG(arg, xreg);
#endif

insta3_SYM		/*  LDA Disp8,S */
/*  called with arg = val to LDA in */
	GET_DISP8_S_RD();
	LDA_INST();

insta4_SYM		/*  LDY Dloc */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_WR()
	b	get_yreg_from_mem
	nop
#else
	C_LDY_DLOC();
#endif

insta5_SYM		/*  LDA Dloc */
/*  called with arg = val to LDA in */
	GET_DLOC_RD();
	LDA_INST();

insta6_SYM		/*  LDX Dloc */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_WR()
	b	get_xreg_from_mem
	nop
#else
	C_LDX_DLOC();
#endif

insta7_SYM		/*  LDA [Dloc] */
	GET_DLOC_L_IND_RD();
	LDA_INST();

insta8_SYM		/*  TAY */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,insta8_16bit_SYM
	extru	acc,31,8,zero

	extru	acc,24,1,neg
	b	dispatch
	copy	zero,yreg

insta8_16bit_SYM
	extru	acc,31,16,zero
	extru	acc,16,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(acc, yreg);
#endif

insta9_SYM		/*  LDA #imm */
	GET_IMM_MEM();
	LDA_INST();

instaa_SYM		/*  TAX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instaa_16bit_SYM
	extru	acc,31,8,zero

	extru	acc,24,1,neg
	b	dispatch
	copy	zero,xreg

instaa_16bit_SYM
	extru	acc,31,16,zero
	extru	acc,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(acc, xreg);
#endif

instab_SYM		/*  PLB */
#ifdef ASM
	INC_KPC_1
	bl	pull_8,link
	CYCLES_PLUS_1

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,dbank
#else
	INC_KPC_1;
	CYCLES_PLUS_1
	PULL8(dbank);
	SET_NEG_ZERO8(dbank);
#endif

instac_SYM		/*  LDY abs */
#ifdef ASM
	GET_ABS_ADDR()
	b	get_yreg_from_mem
	nop
#else
	C_LDY_ABS();
#endif


instad_SYM		/*  LDA abs */
	GET_ABS_RD();
	LDA_INST();

instae_SYM		/*  LDX abs */
#ifdef ASM
	GET_ABS_ADDR()
	b	get_xreg_from_mem
	nop
#else
	C_LDX_ABS();
#endif

instaf_SYM		/*  LDA long */
	GET_LONG_RD();
	LDA_INST();


instb0_SYM		/*  BCS disp8 */
#ifdef ASM
	COND_BR1
	bb,>=	psr,31,instb0_2_SYM
	COND_BR2

instb0_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x01));
#endif

instb1_SYM		/*  LDA (Dloc),y */
	GET_DLOC_IND_Y_RD();
	LDA_INST();

instb2_SYM		/*  LDA (Dloc) */
	GET_DLOC_IND_RD();
	LDA_INST();

instb3_SYM		/*  LDA (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	LDA_INST();

instb4_SYM		/*  LDY Dloc,x */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_X_WR();
	b	get_yreg_from_mem
	nop
#else
	C_LDY_DLOC_X();
#endif

instb5_SYM		/*  LDA Dloc,x */
	GET_DLOC_X_RD();
	LDA_INST();

instb6_SYM		/*  LDX Dloc,y */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_Y_WR();
	b	get_xreg_from_mem
	nop
#else
	C_LDX_DLOC_Y();
#endif

instb7_SYM		/*  LDA [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	LDA_INST();

instb8_SYM		/*  CLV */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,25,1,psr		/* clear overflow */
#else
	psr = psr & ~0x40;
	INC_KPC_1;
#endif

instb9_SYM		/*  LDA abs,y */
	GET_ABS_Y_RD();
	LDA_INST();

instba_SYM		/*  TSX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instba_16bit_SYM
	extru	stack,31,8,zero

	extru	stack,24,1,neg
	b	dispatch
	copy	zero,xreg
instba_16bit_SYM
	copy	stack,zero
	extru	stack,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(stack, xreg);
#endif

instbb_SYM		/*  TYX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instbb_16bit_SYM
	copy	yreg,xreg

/*  8 bit */
	extru	yreg,24,1,neg
	b	dispatch
	copy	yreg,zero
instbb_16bit_SYM
	extru	yreg,16,1,neg
	b	dispatch
	copy	yreg,zero
#else
	INC_KPC_1;
	SET_INDEX_REG(yreg, xreg);
#endif

instbc_SYM		/*  LDY Abs,X */
#ifdef ASM
	GET_ABS_INDEX_ADDR_FOR_RD(xreg)
	b	get_yreg_from_mem
	nop
#else
	C_LDY_ABS_X();
#endif

instbd_SYM		/*  LDA Abs,X */
	GET_ABS_X_RD();
	LDA_INST();

instbe_SYM		/*  LDX Abs,y */
#ifdef ASM
	GET_ABS_INDEX_ADDR_FOR_RD(yreg)
	b	get_xreg_from_mem
	nop
#else
	C_LDX_ABS_Y();
#endif

instbf_SYM		/*  LDA Long,X */
	GET_LONG_X_RD();
	LDA_INST();


instc0_SYM		/*  CPY #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	bb,>=	psr,27,instc0_16bit_SYM
	INC_KPC_2;
	CMP_INDEX_REG_MEAT8(yreg)
instc0_16bit_SYM
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	INC_KPC_1
	dep	scratch1,23,8,ret0
	CMP_INDEX_REG_MEAT16(yreg)
#else
	C_CPY_IMM();
#endif


instc1_SYM		/*  CMP (Dloc,X) */
/*  called with arg = val to CMP in */
	GET_DLOC_X_IND_RD();
	CMP_INST();

instc2_SYM		/*  REP #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_2;
	dep	neg,24,1,psr
	CYCLES_PLUS_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	andcm	psr,ret0,ret0
	ldi	0,zero
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr
	b	update_system_state
	extru	ret0,24,1,neg
#else
	GET_1BYTE_ARG;
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_2;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	psr = psr & ~(arg & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
#endif


instc3_SYM		/*  CMP Disp8,S */
/*  called with arg = val to CMP in */
	GET_DISP8_S_RD();
	CMP_INST();

instc4_SYM		/*  CPY Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	CMP_INDEX_REG_LOAD(instc4_16bit_SYM, yreg)
#else
	C_CPY_DLOC();
#endif


instc5_SYM		/*  CMP Dloc */
	GET_DLOC_RD();
	CMP_INST();

instc6_SYM		/*  DEC Dloc */
	GET_DLOC_RD();
	DEC_INST(1);

instc7_SYM		/*  CMP [Dloc] */
	GET_DLOC_L_IND_RD();
	CMP_INST();

instc8_SYM		/*  INY */
#ifdef ASM
	INC_KPC_1
	addi	1,yreg,yreg
	bb,>=	psr,27,instc8_16bit_SYM
	extru	yreg,31,8,zero

	extru	yreg,24,1,neg
	b	dispatch
	copy	zero,yreg

instc8_16bit_SYM
	extru	yreg,31,16,zero
	extru	yreg,16,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(yreg + 1, yreg);
#endif

instc9_SYM		/*  CMP #imm */
	GET_IMM_MEM();
	CMP_INST();

instca_SYM		/*  DEX */
#ifdef ASM
	INC_KPC_1
	addi	-1,xreg,xreg
	bb,>=	psr,27,instca_16bit_SYM
	extru	xreg,31,8,zero

	extru	xreg,24,1,neg
	b	dispatch
	copy	zero,xreg

instca_16bit_SYM
	extru	xreg,31,16,zero
	extru	xreg,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(xreg - 1, xreg);
#endif

instcb_SYM		/*  WAI */
#ifdef ASM
	ldil	l%g_wait_pending,scratch1
	CYCLES_FINISH
	ldi	1,scratch2
	b	dispatch
	stw	scratch2,r%g_wait_pending(scratch1)
#else
	g_wait_pending = 1;
	CYCLES_FINISH
#endif

instcc_SYM		/*  CPY abs */
#ifdef ASM
	GET_ABS_ADDR()
	CMP_INDEX_REG_LOAD(instcc_16bit_SYM, yreg)
#else
	C_CPY_ABS();
#endif




instcd_SYM		/*  CMP abs */
	GET_ABS_RD();
	CMP_INST();

instce_SYM		/*  DEC abs */
	GET_ABS_RD();
	DEC_INST(0);


instcf_SYM		/*  CMP long */
	GET_LONG_RD();
	CMP_INST();


instd0_SYM		/*  BNE disp8 */
#ifdef ASM
	COND_BR1
	comib,=	0,zero,instd0_2_SYM
	COND_BR2

instd0_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(zero != 0);
#endif

instd1_SYM		/*  CMP (Dloc),y */
	GET_DLOC_IND_Y_RD();
	CMP_INST();

instd2_SYM		/*  CMP (Dloc) */
	GET_DLOC_IND_RD();
	CMP_INST();

instd3_SYM		/*  CMP (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	CMP_INST();

instd4_SYM		/*  PEI Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	bl	get_mem_long_16,link
	CYCLES_PLUS_1

/*  push ret0 */
	extru	ret0,31,16,arg0
	ldil	l%dispatch,link
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	GET_DLOC_ADDR()
	GET_MEMORY16(arg, arg, 1);
	CYCLES_PLUS_1;
	PUSH16_UNSAFE(arg);
#endif

instd5_SYM		/*  CMP Dloc,x */
	GET_DLOC_X_RD();
	CMP_INST();

instd6_SYM		/*  DEC Dloc,x */
	GET_DLOC_X_RD();
	DEC_INST(1);

instd7_SYM		/*  CMP [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	CMP_INST();

instd8_SYM		/*  CLD */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,28,1,psr		/* clear decimal */
#else
	psr = psr & (~0x8);
	INC_KPC_1;
#endif

instd9_SYM		/*  CMP abs,y */
	GET_ABS_Y_RD();
	CMP_INST();

instda_SYM		/*  PHX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instda_16bit_SYM
	ldil	l%dispatch,link

	extru	xreg,31,8,arg0
	b	push_8
	ldo	r%dispatch(link),link

instda_16bit_SYM
	extru	xreg,31,16,arg0
	b	push_16
	ldo	r%dispatch(link),link
#else
	INC_KPC_1;
	if(psr & 0x10) {
		PUSH8(xreg);
	} else {
		PUSH16(xreg);
	}
#endif

instdb_SYM		/*  STP */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_1
	b	dispatch_done
	depi	RET_STP,3,4,ret0
#else
	CYCLES_FINISH
	FINISH(RET_STP, 0);
#endif

instdc_SYM		/*  JML (Abs) */
#ifdef ASM
	ldb	1(scratch1),arg0
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	bl	get_mem_long_24,link
	dep	scratch1,23,8,arg0

	b	dispatch
	copy	ret0,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	GET_MEMORY24(arg, kpc, 1);
#endif

instdd_SYM		/*  CMP Abs,X */
	GET_ABS_X_RD();
	CMP_INST();

instde_SYM		/*  DEC Abs,X */
	GET_ABS_X_RD_WR();
	DEC_INST(0);

instdf_SYM		/*  CMP Long,X */
	GET_LONG_X_RD();
	CMP_INST();


inste0_SYM		/*  CPX #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	bb,>=	psr,27,inste0_16bit_SYM
	INC_KPC_2;
	CMP_INDEX_REG_MEAT8(xreg)
inste0_16bit_SYM
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	INC_KPC_1
	dep	scratch1,23,8,ret0
	CMP_INDEX_REG_MEAT16(xreg)
#else
	C_CPX_IMM();
#endif


inste1_SYM		/*  SBC (Dloc,X) */
/*  called with arg = val to SBC in */
	GET_DLOC_X_IND_RD();
	SBC_INST();

inste2_SYM		/*  SEP #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_2;
	dep	neg,24,1,psr
	CYCLES_PLUS_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	or	psr,ret0,ret0
	ldi	0,zero
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr
	b	update_system_state
	extru	ret0,24,1,neg
#else
	GET_1BYTE_ARG;
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_2;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	psr = psr | (arg & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
#endif


inste3_SYM		/*  SBC Disp8,S */
/*  called with arg = val to SBC in */
	GET_DISP8_S_RD();
	SBC_INST();

inste4_SYM		/*  CPX Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	CMP_INDEX_REG_LOAD(inste4_16bit_SYM, xreg)
#else
	C_CPX_DLOC();
#endif


inste5_SYM		/*  SBC Dloc */
/*  called with arg = val to SBC in */
	GET_DLOC_RD();
	SBC_INST();

inste6_SYM		/*  INC Dloc */
	GET_DLOC_RD();
	INC_INST(1);

inste7_SYM		/*  SBC [Dloc] */
	GET_DLOC_L_IND_RD();
	SBC_INST();

inste8_SYM		/*  INX */
#ifdef ASM
	INC_KPC_1
	addi	1,xreg,xreg
	bb,>=	psr,27,inste8_16bit_SYM
	extru	xreg,31,8,zero

	extru	xreg,24,1,neg
	b	dispatch
	copy	zero,xreg

inste8_16bit_SYM
	extru	xreg,31,16,zero
	extru	xreg,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(xreg + 1, xreg);
#endif

inste9_SYM		/*  SBC #imm */
	GET_IMM_MEM();
	SBC_INST();

instea_SYM		/*  NOP */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	nop
#else
	INC_KPC_1;
#endif

insteb_SYM		/*  XBA */
#ifdef ASM
	extru	acc,16,1,neg		/* Z and N reflect status of low 8 */
	CYCLES_PLUS_1			/*   bits of final acc value! */
	copy	acc,scratch1		/* regardlessof ACC 8 or 16 bit */
	extru	acc,23,8,acc
	INC_KPC_1
	copy	acc,zero
	b	dispatch
	dep	scratch1,23,8,acc
#else
	tmp1 = acc & 0xff;
	CYCLES_PLUS_1
	acc = (tmp1 << 8) + (acc >> 8);
	INC_KPC_1;
	SET_NEG_ZERO8(acc & 0xff);
#endif

instec_SYM		/*  CPX abs */
#ifdef ASM
	GET_ABS_ADDR()
	CMP_INDEX_REG_LOAD(instec_16bit_SYM, xreg)
#else
	C_CPX_ABS();
#endif




insted_SYM		/*  SBC abs */
	GET_ABS_RD();
	SBC_INST();

instee_SYM		/*  INC abs */
	GET_ABS_RD();
	INC_INST(0);


instef_SYM		/*  SBC long */
	GET_LONG_RD();
	SBC_INST();


instf0_SYM		/*  BEQ disp8 */
#ifdef ASM
	COND_BR1
	comib,<> 0,zero,instf0_2_SYM
	COND_BR2

instf0_2_SYM
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(zero == 0);
#endif

instf1_SYM		/*  SBC (Dloc),y */
	GET_DLOC_IND_Y_RD();
	SBC_INST();

instf2_SYM		/*  SBC (Dloc) */
	GET_DLOC_IND_RD();
	SBC_INST();

instf3_SYM		/*  SBC (Disp8,s),y */
	GET_DISP8_S_IND_Y_RD();
	SBC_INST();

instf4_SYM		/*  PEA Abs */
#ifdef ASM
	ldb	1(scratch1),arg0
	ldil	l%dispatch,link
	ldb	2(scratch1),scratch1
	INC_KPC_3
	CYCLES_PLUS_1
	ldo	r%dispatch(link),link
	b	push_16_unsafe
	dep	scratch1,23,8,arg0
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	INC_KPC_3;
	PUSH16_UNSAFE(arg);
#endif

instf5_SYM		/*  SBC Dloc,x */
	GET_DLOC_X_RD();
	SBC_INST();

instf6_SYM		/*  INC Dloc,x */
	GET_DLOC_X_RD();
	INC_INST(1);

instf7_SYM		/*  SBC [Dloc],Y */
	GET_DLOC_L_IND_Y_RD();
	SBC_INST();

instf8_SYM		/*  SED */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,28,1,psr		/* set decimal */
#else
	INC_KPC_1;
	psr |= 0x8;
#endif

instf9_SYM		/*  SBC abs,y */
	GET_ABS_Y_RD();
	SBC_INST();

instfa_SYM		/*  PLX */
#ifdef ASM
	bb,<	psr,27,instfa_8bit_SYM
	CYCLES_PLUS_1

	INC_KPC_1
	bl	pull_16,link
	nop

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,xreg

instfa_8bit_SYM
	INC_KPC_1
	bl	pull_8,link
	nop

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	CYCLES_PLUS_1;
	if(psr & 0x10) {
		PULL8(xreg);
		SET_NEG_ZERO8(xreg);
	} else {
		PULL16(xreg);
		SET_NEG_ZERO16(xreg);
	}
#endif

instfb_SYM		/*  XCE */
#ifdef ASM
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_1
	extru	psr,23,1,scratch1	/* e bit */
	dep	psr,23,1,psr		/* copy carry to e bit */
	b	update_system_state
	dep	scratch1,31,1,psr	/* copy e bit to carry */
#else
	tmp2 = psr;
	INC_KPC_1;
	psr = (tmp2 & 0xfe) | ((tmp2 & 1) << 8) | ((tmp2 >> 8) & 1);
	UPDATE_PSR(psr, tmp2);
#endif

instfc_SYM		/*  JSR (Abs,X) */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	kpc,15,8,scratch2
	ldb	2(scratch1),scratch1
	dep	scratch2,15,16,ret0
	INC_KPC_2;
	dep	scratch1,23,8,ret0
	add	xreg,ret0,arg0
	bl	get_mem_long_16,link
	extru	arg0,31,24,arg0

	CYCLES_PLUS_2
	extru	kpc,31,16,arg0
	ldil	l%dispatch,link
	dep	ret0,31,16,kpc
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	GET_2BYTE_ARG;
	INC_KPC_2;
	tmp1 = kpc;
	arg = (kpc & 0xff0000) + ((arg + xreg) & 0xffff);
	GET_MEMORY16(arg, tmp2, 1);
	kpc = (kpc & 0xff0000) + tmp2;
	CYCLES_PLUS_2
	PUSH16_UNSAFE(tmp1);
#endif

instfd_SYM		/*  SBC Abs,X */
	GET_ABS_X_RD();
	SBC_INST();

instfe_SYM		/*  INC Abs,X */
	GET_ABS_X_RD_WR();
	INC_INST(0);

instff_SYM		/*  SBC Long,X */
	GET_LONG_X_RD();
	SBC_INST();

