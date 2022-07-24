import os,sys,struct

def padto(f, newsize):
	with open(f,"rb+") as f:
		buff=f.read()
		size=newsize-len(buff)
		f.write(b"\x00"*size)
def inject(ffrom, fto, offset):
	with open(ffrom,"rb") as f:
		buff=f.read()
	with open(fto,"rb+") as f:
		f.seek(offset)
		f.write(buff)

padto("bb3.bin", 0x80200)
inject("otherapp_template/otherapp_template.bin","bb3.bin",0x8000)
inject("slot1.bin","bb3.bin",0xC000)
#padto("bb3.bin", 0x80200)