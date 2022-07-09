#define TID_H	0x00048005
#define DLP	0x484E4441
#define INT	0x42383841
#define WBUFF	0x00681000
#define ENTRY 	0x0ffffe48

#define POP_R0PC 0x00146760
#define POP_R1PC 0x001d53f8
#define POP_R2PC 0x00207d31
#define POP_R3PC 0x0010d44c
#define POP_R4R8PC 0x0010d20c
#define ROP_POPR3_ADDSPR3_POPPC 0x0014660c + 4
#define STACK_PIVOT ROP_POPR3_ADDSPR3_POPPC
#define PIVOT_OFFSET -0x110
#define ROP_MOV_R3_R6_BLX_ADDRR5 0x001264AC
#define ROP_STR_R12R0  0x0010df64
#define ROP_MOV_R1_0 0x0019DFB4       			@sp!,{ r4 pc }
#define ROP_MOV_R3_4_JUNKTOR0 0x001317A4 @ LDMFD  	SP!, {R4-R8,PC}
#define ROP_MOV_R2_4_BLX_R3 0x00104bbc

#define svcSleepThread 0x001bdbd0
#define STR_YS 0x0028cb60
#define GARBAGE 0xADDEADDE
#define NULL 0
#define GSPGPU_SERVHANDLEADR 0x002993c4
#define GSPGPU_WriteHWRegs 0x00136200      		@sp!,{ r4 pc }
#define GSPGPU_SetLcdForceBlack 0x001c13a4  		@sp!,{ r4-r6 pc }
#define MAGENTA_ADDR 0x002A800C
#define HWREGS_ADDR 0x00202A04 

#define AM_TwlExport 0x00156d28 + 0x10