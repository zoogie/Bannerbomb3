#pragma once

#define ROP_BXR1 POP_R4LR_BXR1+4
#define ROP_BXLR ROP_LDR_R0FROMR0+4 //"bx lr"

#define ROP_EQBXLR_NE_CALLVTABLEFUNCPTR (IFile_Close+0x4) //Offset 0x4 in IFile_Close for ROP conditional execution. For condition-code EQ, bx-lr is executed, otherwise a vtable funcptr call with the r0 object is executed.

#ifdef THROWFATALERR_IPC
#define ROP_SENDCMDADDR THROWFATALERR_IPC+0x14 //Writes r0 to r4+0, then copies 0x80-bytes from r1 to r4+4. Then uses svcSendSyncRequest with handle *r5.
#endif

#ifdef ROP_POPR3_ADDSPR3_POPPC
#define STACKPIVOT_ADR ROP_POPR3_ADDSPR3_POPPC
#endif

#ifndef ROPBUFLOC
#define ROPBUFLOC(x) (ROPBUF + (x - _start))
#endif

@ Size: 0x8
.macro ROP_SETR0 value
#ifdef POP_R0PC
	.word POP_R0PC
	.word \value
#elif defined (ROP_LDRR0SP_POPR3PC)
	.word ROP_LDRR0SP_POPR3PC
	.word \value
#else
	#error "The gadget for setting r0 is not defined."
#endif
.endm

@ Size: 0x8
.macro ROP_SETR1 value
.word POP_R1PC
.word \value @ r1
.endm

@ Size: 0x8 + 0xc (0x14)
.macro ROP_SETLR lr
ROP_SETR1 ROP_POPPC

.word POP_R4LR_BXR1
.word 0 @ r4
.word \lr
.endm

@ Size: 0x34
.macro ROP_SETLR_OTHER lr
.word POP_R2R6PC
.word ROP_POPPC @ r2
.word 0 @ r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word POP_R4R8LR_BXR2
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6
.word 0 @ r7
.word 0 @ r8
.word \lr
.endm

@ Size: 0x14 + 0x8 + 0x4 (0x20)
.macro ROP_LOADR0_FROMADDR addr
ROP_SETLR ROP_POPPC

ROP_SETR0 \addr

.word ROP_LDR_R0FROMR0
.endm

.macro CALLFUNC funcadr, r0, r1, r2, r3, sp0, sp4, sp8, sp12
ROP_SETLR POP_R2R6PC

ROP_SETR0 \r0

ROP_SETR1 \r1

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr

.word \sp0
.word \sp4
.word \sp8
.word \sp12
.word 0 @ r6
.endm

@ This is basically: CALLFUNC funcadr, *r0, r1, r2, r3, sp0, sp4, sp8, sp12
.macro CALLFUNC_LOADR0 funcadr, r0, r1, r2, r3, sp0, sp4, sp8, sp12
ROP_LOADR0_FROMADDR \r0

ROP_SETLR POP_R2R6PC

ROP_SETR1 \r1

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr

.word \sp0
.word \sp4
.word \sp8
.word \sp12
.word 0 @ r6
.endm

@ This is basically: CALLFUNC funcadr, r0, *r1, r2, r3, sp0, sp4, sp8, sp12
.macro CALLFUNC_LDRR1 funcadr, r0, r1, r2, r3, sp0, sp4, sp8, sp12
ROP_SETLR ROP_POPPC

ROPMACRO_COPYWORD ROPBUFLOC(. + 0x8 + 0x14 + 0x8 + 0x4), \r1

ROP_SETLR POP_R2R6PC

ROP_SETR0 \r0

ROP_SETR1 0 @ Overwritten by the above rop.

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr

.word \sp0
.word \sp4
.word \sp8
.word \sp12
.word 0 @ r6
.endm

@ This is basically: CALLFUNC funcadr, *r0, *r1, r2, r3, sp0, sp4, sp8, sp12
#define CALLFUNC_LDRR0R1_R1OFFSET 0x14 + ROPMACRO_COPYWORD_SRCADDROFFSET
.macro CALLFUNC_LDRR0R1 funcadr, r0, r1, r2, r3, sp0, sp4, sp8, sp12
ROP_SETLR ROP_POPPC

ROPMACRO_COPYWORD ROPBUFLOC(. + 0x8 + 0x14 + 0x20 + 0x4), \r1

ROP_LOADR0_FROMADDR \r0

ROP_SETLR POP_R2R6PC

ROP_SETR1 0 @ Overwritten by the above rop.

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr

.word \sp0
.word \sp4
.word \sp8
.word \sp12
.word 0 @ r6
.endm

@ Size: 0x40
#define CALLFUNC_NOSP_FUNCADROFFSET 0x3C
.macro CALLFUNC_NOSP funcadr, r0, r1, r2, r3
ROP_SETLR ROP_POPPC

ROP_SETR0 \r0

ROP_SETR1 \r1

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr
.endm

@ This is is basically: CALLFUNC_NOSP funcadr, *r0, r1, r2, r3
.macro CALLFUNC_NOSP_LDRR0 funcadr, r0, r1, r2, r3
ROP_LOADR0_FROMADDR \r0

ROP_SETR1 \r1

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr
.endm

@ This is is basically: CALLFUNC_NOSP funcadr, r0, r1, *r2, r3
.macro CALLFUNC_NOSP_LOADR2 funcadr, r0, r1, r2, r3
ROP_SETLR ROP_POPPC

ROPMACRO_COPYWORD ROPBUFLOC(. + 0x8 + 0x8 + 0x8 + 0x4), \r2

ROP_SETR0 \r0

ROP_SETR1 \r1

.word POP_R2R6PC
.word \r2
.word \r3
.word 0 @ r4
.word 0 @ r5
.word 0 @ r6

.word \funcadr
.endm

.macro CALLFUNC_NOARGS funcadr
ROP_SETLR ROP_POPPC
.word \funcadr
.endm

#define CALLFUNC_R0R1_R0OFFSET 0x14 + 0x4
#define CALLFUNC_R0R1_R1OFFSET 0x14 + 0x8 + 0x4
.macro CALLFUNC_R0R1 funcadr, r0, r1
ROP_SETLR ROP_POPPC

ROP_SETR0 \r0

ROP_SETR1 \r1

.word \funcadr
.endm

.macro CALL_GXCMD4 srcadr, dstadr, cpysize
CALLFUNC GXLOW_CMD4, \srcadr, \dstadr, \cpysize, 0, 0, 0, 0, 0x8
.endm

@ This is basically: CALL_GXCMD4 *srcadr, dstadr, cpysize
.macro CALL_GXCMD4_LDRSRC srcadr, dstadr, cpysize
CALLFUNC_LOADR0 GXLOW_CMD4, \srcadr, \dstadr, \cpysize, 0, 0, 0, 0, 0x8
.endm

@ This is basically: CALL_GXCMD4 srcadr, *dstadr, cpysize
.macro CALL_GXCMD4_LDRDST srcadr, dstadr, cpysize
CALLFUNC_LDRR1 GXLOW_CMD4, \srcadr, \dstadr, \cpysize, 0, 0, 0, 0, 0x8
.endm

@ This is basically: CALL_GXCMD4 *srcadr, *dstadr, cpysize
.macro CALL_GXCMD4_LDRSRCDST srcadr, dstadr, cpysize
CALLFUNC_LDRR0R1 GXLOW_CMD4, \srcadr, \dstadr, \cpysize, 0, 0, 0, 0, 0x8
.endm

#ifndef ROP_POPR3_ADDSPR3_POPPC
.macro ROPMACRO_STACKPIVOT_PREPARE sp, pc
@ Write to the word which will be popped into sp.
ROPMACRO_WRITEWORD ROPBUFLOC(stackpivot_sploadword), \sp

@ Write to the word which will be popped into pc.
ROPMACRO_WRITEWORD ROPBUFLOC(stackpivot_pcloadword), \pc
.endm
#endif

#ifdef ROP_POPR3_ADDSPR3_POPPC
.macro ROPMACRO_STACKPIVOT_JUMP
.word ROP_POPR3_ADDSPR3_POPPC
.endm
#endif

.macro ROPMACRO_STACKPIVOT sp, pc
#ifndef ROP_POPR3_ADDSPR3_POPPC
	ROPMACRO_STACKPIVOT_PREPARE \sp, \pc

	ROPMACRO_STACKPIVOT_PREPAREREGS_BEFOREJUMP

	ROPMACRO_STACKPIVOT_JUMP
#else
	.word ROP_POPR3_ADDSPR3_POPPC
	.word \sp - ROPBUFLOC(. + 0x4)
#endif
.endm

.macro COND_THROWFATALERR
#ifdef ROP_COND_THROWFATALERR
	.word ROP_COND_THROWFATALERR

	.word 0 @ r3
	.word 0 @ r4
	.word 0 @ r5
#elif defined (ROP_COND_THROWFATALERR_ALT0)
	.word ROP_COND_THROWFATALERR_ALT0

	.word 0 @ r3, r0 is also loaded from here.
#else
	#error "ROP_COND_THROWFATALERR* isn't defined."
#endif
.endm

#define ROPMACRO_CMPDATA_CMPADDR_OFFSET 0x2C
#define ROPMACRO_CMPDATA_CMPWORD_OFFSET 0x38
.macro ROPMACRO_CMPDATA cmpaddr, cmpword, stackaddr_cmpmismatch
ROP_SETLR ROP_POPPC

ROP_LOADR0_FROMADDR \cmpaddr

ROP_SETR1 \cmpword

#ifdef ROP_CMPR0R1
.word ROP_CMPR0R1
.word 0
#elif defined (ROP_CMPR0R1_ALT0)
.word ROP_CMPR0R1_ALT0
#else
#error "ROP_CMPR0R1* isn't defined."
#endif

#ifndef ROP_POPR3_ADDSPR3_POPPC
ROPMACRO_STACKPIVOT_PREPARE \stackaddr_cmpmismatch, ROP_POPPC

ROPMACRO_STACKPIVOT_PREPAREREGS_BEFOREJUMP
#endif

ROP_SETR0 ROPBUFLOC(ropkit_cmpobject)

#ifdef ROP_POPR3_ADDSPR3_POPPC
ROP_SETLR POP_R1PC
#endif

.word ROP_EQBXLR_NE_CALLVTABLEFUNCPTR @ When the value at cmpaddr matches cmpword, continue the ROP, otherwise call the vtable funcptr which then does the stack-pivot.

#ifdef ROP_POPR3_ADDSPR3_POPPC
.word (\stackaddr_cmpmismatch) - ROPBUFLOC(. + 0x4)
#endif
.endm

@ Size: 0x14 + 0x8 + 0x8 + 0x4 (0x28)
.macro ROPMACRO_WRITEWORD addr, value
ROP_SETLR ROP_POPPC

ROP_SETR0 \addr

ROP_SETR1 \value

.word ROP_STR_R1TOR0
.endm

@ Size: 0x14 + 0x20 + 0x8 + 0x4 (0x40)
#define ROPMACRO_COPYWORD_SRCADDROFFSET 0x2c
#define ROPMACRO_COPYWORD_DSTADDROFFSET 0x38
.macro ROPMACRO_COPYWORD dstaddr, srcaddr
ROP_SETLR ROP_POPPC

ROP_LOADR0_FROMADDR \srcaddr

ROP_SETR1 \dstaddr

.word ROP_STR_R0TOR1
.endm

.macro ROPMACRO_COPYWORD_FROMR0 dstaddr
ROP_SETLR ROP_POPPC

.word ROP_LDR_R0FROMR0

ROP_SETR1 \dstaddr

.word ROP_STR_R0TOR1
.endm

#define ROPMACRO_LDDRR0_ADDR1_STRADDR_VALUEOFFSET 0x24 //Offset relative to the start of ROPMACRO_LDDRR0_ADDR1_STRADDR where the add-value is located.
.macro ROPMACRO_LDDRR0_ADDR1_STRADDR dstaddr, srcaddr, value
ROP_LOADR0_FROMADDR \srcaddr

ROP_SETR1 \value

.word ROP_ADDR0_TO_R1 @ r0 = *srcaddr + value

ROP_SETR1 \dstaddr

.word ROP_STR_R0TOR1 @ Write the above r0 value to *dstaddr.
.endm

.macro ROPMACRO_LDDRR0_ADDR1_STRVALUE addr, addval, writeval
ROP_LOADR0_FROMADDR \addr

ROP_SETR1 \addval

.word ROP_ADDR0_TO_R1 @ r0 = *addr + addval

ROP_SETR1 \writeval

.word ROP_STR_R1TOR0 @ Write the above writeval to <calculated r0>.
.endm

.macro ROPMACRO_IFile_Close IFile_ctx
ROP_LOADR0_FROMADDR \IFile_ctx

.word IFile_Close
.endm

