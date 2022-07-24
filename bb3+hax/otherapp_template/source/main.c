#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/svc.h>
#include <ctr/APT.h>
#include <ctr/FS.h>
#include <ctr/GSP.h>
#include "text.h"
#include "rop_jpn_bin.h"
#include "rop_usa_bin.h"
#include "rop_eur_bin.h"
#include "rop_kor_bin.h"

#define HID_PAD (*(vu32*)0x1000001C)
#define GSPGPU_FlushDataCache 0x0013e46c
#define GSPGPU_SERVHANDLEADR 0x002993c4
#define HAXX 0x58584148
#define slot1 (u8*)0x006AA000+0xC000
#define AM_TwlExport 0x00156d28
u32 VRAM=(u32)0x31000000;
u8 *workbuf;
//Handle *cfgHandle= (Handle*)0x00600000;
Handle cfgHandle;
Handle srvHandle;
u8 region;
#define CLEAR (PAD_L | PAD_R | PAD_Y | PAD_LEFT)   //combo for deleting all wifi slots

typedef enum
{
	PAD_A = (1<<0),
	PAD_B = (1<<1),
	PAD_SELECT = (1<<2),
	PAD_START = (1<<3),
	PAD_RIGHT = (1<<4),
	PAD_LEFT = (1<<5),
	PAD_UP = (1<<6),
	PAD_DOWN = (1<<7),
	PAD_R = (1<<8),
	PAD_L = (1<<9),
	PAD_X = (1<<10),
	PAD_Y = (1<<11)
}PAD_KEY;

#define BIT(n) (1U<<(n))
typedef enum
{
	IPC_BUFFER_R  = BIT(1),                     ///< Readable
	IPC_BUFFER_W  = BIT(2),                     ///< Writable
	IPC_BUFFER_RW = IPC_BUFFER_R | IPC_BUFFER_W ///< Readable and Writable
} IPC_BufferRights;

typedef struct
{
	u64 titleID; ///< The title's ID.
	u64 size;    ///< The title's installed size.
	u16 version; ///< The title's version.
	u8 unk[6];   ///< Unknown title data.
} AM_TitleEntry;

static inline u32 IPC_Desc_Buffer(size_t size, IPC_BufferRights rights)
{
	return (size << 4) | 0x8 | rights;
}


int _strlen(const char* str)
{
	int l=0;
	while(*(str++))l++;
	return l;
}

void _strcpy(char* dst, char* src)
{
	while(*src)*(dst++)=*(src++);
	*dst=0x00;
}

void _strappend(char* str1, char* str2)
{
	_strcpy(&str1[_strlen(str1)], str2);
}

Result _srv_RegisterClient(Handle* handleptr)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10002; //request header code
	cmdbuf[1]=0x20;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handleptr)))return ret;

	return cmdbuf[1];
}

Result _initSrv(Handle* srvHandle)
{
	Result ret=0;
	if(svc_connectToPort(srvHandle, "srv:"))return ret;
	return _srv_RegisterClient(srvHandle);
}

Result _srv_getServiceHandle(Handle* handleptr, Handle* out, char* server)
{
	u8 l=_strlen(server);
	if(!out || !server || l>8)return -1;

	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x50100; //request header code
	_strcpy((char*)&cmdbuf[1], server);
	cmdbuf[3]=l;
	cmdbuf[4]=0x0;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handleptr)))return ret;

	*out=cmdbuf[3];

	return cmdbuf[1];
}

Result _GSPGPU_ImportDisplayCaptureInfo(Handle* handle, GSP_CaptureInfo *captureinfo)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00180000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	ret = cmdbuf[1];

	if(ret==0)
	{
		memcpy(captureinfo, &cmdbuf[2], 0x20);
	}

	return ret;
}

u8 *GSP_GetTopFBADR()
{
	GSP_CaptureInfo capinfo;
	u32 ptr;

	//u32 *paramblk = (u32*)*((u32*)0xFFFFFFC);
	Handle* gspHandle=(Handle*)GSPGPU_SERVHANDLEADR;
	//#define gspHandle (Handle*)0x002993c4


	if(_GSPGPU_ImportDisplayCaptureInfo(gspHandle, &capinfo)!=0)return NULL;

	ptr = (u32)capinfo.screencapture[0].framebuf0_vaddr;
	if(ptr>=0x1f000000 && ptr<0x1f600000)return NULL;//Don't return a ptr to VRAM if framebuf is located there, since writing there will only crash.


	return (u8*)ptr;
}

Result GSP_FlushDCache(u32* addr, u32 size)
{
	
	//Handle* gspHandle=(Handle*)GSPGPU_SERVHANDLEADR;
	Result (*_GSP_FlushDCache)(u32* addr, u32 size);
	//u32 *paramblk = (u32*)*((u32*)0xFFFFFFC);
	_GSP_FlushDCache=(void*)GSPGPU_FlushDataCache;
	return _GSP_FlushDCache(addr, size);
}

const u8 hexTable[]=
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

void hex2str(char* out, u32 val)
{
	int i;
	for(i=0;i<8;i++){out[7-i]=hexTable[val&0xf];val>>=4;}
	out[8]=0x00;
}

void _memcpy(u8 *dest, u8 *src, u32 size){
	
	while(size--) *(dest++)=*(src++);
	
}

void renderString(char* str, int x, int y)
{
	//u8 *ptr = GSP_GetTopFBADR();
	u8 *ptr=(u8*)VRAM;
	if(ptr==NULL)return;
	drawString(ptr,str,x,y);
	GSP_FlushDCache((u32*)ptr, 240*400*3);
}

void centerString(char* str, int y)
{
	//u8 *ptr = GSP_GetTopFBADR();
	u8 *ptr=(u8*)VRAM;
	//volatile u32 lol=*(u32*)0;
	int x=200-(_strlen(str)*4);
	if(ptr==NULL)return;
	drawString(ptr,str,x,y);
	GSP_FlushDCache((u32*)ptr, 240*400*3);
}

void drawHex(u32 val, int x, int y)
{
	char str[9];

	hex2str(str,val);
	renderString(str,x,y);
}

void clearScreen(u8 shade)
{
	//u8 *ptr = GSP_GetTopFBADR();
	u8 *ptr=(u8*)VRAM;
	if(ptr==NULL)return;
	memset(ptr, shade, 240*400*3);
	GSP_FlushDCache((u32*)ptr, 240*400*3);
}

void drawTitleScreen(char* str)
{
	clearScreen(0x00);
	centerString("BB3 multihax by zoogie",0);
	centerString("https://3ds.hacks.guide/",1*8);
	centerString("Help: https://discord.gg/C29hYvh",2*8);
	renderString(str, 0, 7*8);
}

Result NS_RebootSystem(Handle handle, u32 command)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = command; //0x160000 = reboot   0xE0000 = shutdown
	
	if((ret = svc_sendSyncRequest(handle)))return ret;

	return (Result)cmdbuf[1];
}

Result _GSPGPU_SetBufferSwap(Handle handle, u32 screenid, GSP_FramebufferInfo framebufinfo)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050200;
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], &framebufinfo, sizeof(GSP_FramebufferInfo));
	
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result _GSPGPU_SetLcdForceBlack(Handle handle, u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; // 0xB0040
	cmdbuf[1]=flags;

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result _CFG_GetConfigInfoBlk4(u32 size, u32 blkID, u8* outData, Handle cfgHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x8010082; // 0x4010082
	cmdbuf[1] = size;
	cmdbuf[2] = blkID;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4] = (u32)outData;

	if((ret=svc_sendSyncRequest(cfgHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result _CFG_SetConfigInfoBlk4(u32 size, u32 blkID, u8* inData, Handle cfgHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] =  0x8020082; // 0x4020082
	cmdbuf[1] = blkID;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[4] = (u32)inData;

	if((ret=svc_sendSyncRequest(cfgHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result _CFG_UpdateConfigSavegame(Handle cfgHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x8030000; // changed from 0x8030000 to be compatible with both cfg:i and cfg:s

	if((ret=svc_sendSyncRequest(cfgHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGU_SecureInfoGetRegion(u8* region, Handle cfgHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x20000; // 0x20000

	if((ret = svc_sendSyncRequest(cfgHandle)))return ret;

	*region = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result check_slots(){
	u8 *zerobuf=(u8*)(workbuf+0xC00);
	memset(zerobuf, 0, 0x500);
	int x=30*8;
	int y=4*8;
	renderString("Slot Status:      ", x, y);
	for(int i=0; i<3; i++){
		y+=1*8;
		drawHex(i+1, x+8, y);
		 _CFG_GetConfigInfoBlk4(0xC00, 0x80000+i, workbuf, cfgHandle);
		 renderString("          ", x, y);
		 svc_sleepThread(100*1000*1000);
		if(*(u32*)(workbuf+0x420) == HAXX){
			renderString("  Haxx    ", x, y);
		}
		else if(memcmp(zerobuf, workbuf, 0x500)){
			renderString("  User    ", x, y);
		}
		else{
			renderString("  None    ", x, y);
		}
	}
	
	return 0;
}

Result restore_slots(){
	for(int i=0; i<3; i++){
		memset(workbuf, 0, 0xC00);
		_CFG_GetConfigInfoBlk4(0xC00, 0x80000+i, workbuf, cfgHandle);
		if(*(u32*)(workbuf+0x420) == HAXX){
			memcpy(workbuf, workbuf+0x500, 0x500); //restore backup slot to wifi slot
			memset(workbuf+0x500, 0, 0x500);       //clear slot backup to zeros
			_CFG_SetConfigInfoBlk4(0xC00, 0x80000+i, workbuf, cfgHandle); //commit workbuf to slot
		}
		else{
			//pass
		}
	}
	 _CFG_UpdateConfigSavegame(cfgHandle);
	check_slots();
	
	return 0;
}

Result inject_slots(){
	for(int i=0; i<3; i++){
		memset(workbuf, 0, 0xC00);
		_CFG_GetConfigInfoBlk4(0xC00, 0x80000+i, workbuf, cfgHandle);
		if(*(u32*)(workbuf+0x420) == HAXX){
			//pass
		}
		else{
			memcpy(workbuf+0x500, workbuf, 0x500); //backup user slot to slot+0x500
			memcpy(workbuf, slot1, 0x500);         //write slot1 to workbuf
			_CFG_SetConfigInfoBlk4(0xC00, 0x80000+i, workbuf, cfgHandle); //commit workbuf to slot
		}
	}
	 _CFG_UpdateConfigSavegame(cfgHandle);
	 check_slots();
	 
	 return 0;
}

Result clear_slots(){
	int count=0;
	while(1){
		if(HID_PAD == CLEAR){
			count++;
			if(count > 5*60) break;  //if combo held for 5 seconds, proceed.
		}
		else{
			return 0;
		}
		svc_sleepThread(17*1000*1000);
	}
	
	Result (* const delete_slot)(u32) = (void *)0x001F86B0;  //this is amazing. works perfectly, and even deletes the annoying nvram backup.
	
	delete_slot(0);
	delete_slot(1);
	delete_slot(2);
	 
	check_slots();

	renderString("All slots cleared!", 0, 16*8);
	svc_sleepThread(2000*1000*1000);
	renderString("                  ", 0, 16*8);

	return 0;
}

Result menuhax67(u16 version){
	Result res=0;
	u32 base_addr=0;  //0x3093d0
	//u8 region=*(u8*)0x297600;  //a nice little hack, mset already has a global ready for us. thanks mset.
	//u8 region=0xff;
	bool isnew=*(u32*)0x1FF80030 == 6;
	int y=4*8;
	//Handle *cptr;

	u8 *data=workbuf+0x10;
	u32 *rop=(u32*)(workbuf+0x100);
	u32 fail=1;
	
	res = CFGU_SecureInfoGetRegion(&region, cfgHandle);
	
	if(region > 6){
		//printf("Error: issue with version check\n");
		return 4;
	}
	
	if(region==0){
		base_addr=0x00347a10;
		memcpy(rop, rop_jpn_bin, rop_jpn_bin_size); 
		//printf("JPN\n");
		if(version != 31745){
			//printf("Error: unsupported menu r%d, expected r31\n", menuversion);
			return 3;
		}
	}
	else if(region==1){
		base_addr=0x00346a10;
		memcpy(rop, rop_usa_bin, rop_usa_bin_size); 
		//printf("USA\n");
		if(version != 29697){
			//printf("Error: unsupported menu r%d, expected r29\n", menuversion);
			return 3;
		}
	}
	else if(region==2 || region==3){
		base_addr=0x00347a10;
		memcpy(rop, rop_eur_bin, rop_eur_bin_size); 
		//printf("EUR\n");
		if(version != 29696){
			//printf("Error: unsupported menu r%d, expected r29\n", menuversion);
			return 3;
		}
	}
	else if(region==5){
		base_addr=0x00346a10;
		memcpy(rop, rop_kor_bin, rop_kor_bin_size); 
		//printf("KOR\n");
		if(version != 15361){
			//printf("Error: unsupported menu r%d, expected r15\n", menuversion);
			return 3;
		}
	}
	else{
		//printf("Error: region not supported\n");    
		return 1;
	}
	
	base_addr+=0xA8;
	
	if(isnew){
		//printf("NEW3DS\n");
		for(int i=0;i<0xC0/4;i++){
			if(rop[i]==0x35040000){ //find the old3ds linearmem address and patch it to new3ds address
				rop[i]=0x38C40000;
				fail=0;
			}
		}
		if(fail==1){
			//printf("Error: new3ds address location not found\n");
			return 2;
		}
	}
	else{
		//printf("OLD3DS\n");
		fail=0;
	}
	
	res = _CFG_GetConfigInfoBlk4(2, 0x50001, data, cfgHandle);
	drawHex(res, 8*8, y+12*8); //ns:s get result
	//while(1)svc_sleepThread(0xffffffff);
	data[1]=0xE;
	res = _CFG_SetConfigInfoBlk4(2, 0x50001, data, cfgHandle);
	
	res = _CFG_GetConfigInfoBlk4(8, 0x50009, data, cfgHandle);
	*(u32*)(data+4)=base_addr;
	res = _CFG_SetConfigInfoBlk4(8, 0x50009, data, cfgHandle);
	
	res = _CFG_SetConfigInfoBlk4(0xc0, 0xc0000, (u8*)rop, cfgHandle);//36857a10 08557a10
	
	res = _CFG_UpdateConfigSavegame(cfgHandle);  //note that this is the cfg:i version of this function, so it won't work with anything but mset
	//printf("done %08X\n", (int)res);   //easy workaround is to patch header 00 00 03 08 --> 00 00 03 04 in the binary (first occurrence)
	                                   //not really in the mood to make a local libctru or bother the libctru maintainers
	return res;
}

Result uninstall(){
	Result res;
	//u8 data[0xc0]={0};
	u8 *data=(u8*)workbuf;
	//int y=4*8;
	
	//Handle *cptr;
	
	//_initSrv(cptr);

	res = _CFG_GetConfigInfoBlk4(2, 0x50001, data, cfgHandle);
	data[1]=0x3;            //reverts brightness level to 3 (range 1-5)
	res = _CFG_SetConfigInfoBlk4(2, 0x50001, data, cfgHandle);
	
	res = _CFG_GetConfigInfoBlk4(8, 0x50009, data, cfgHandle);
	*(u32*)(data+4)=0x00000101;  //don't know what this is, but 0x101 is the value that's usually there
	res = _CFG_SetConfigInfoBlk4(8, 0x50009, data, cfgHandle);

	memset(data, 0, 0xC0);  //reverts parental controls to a blank state
	data[0x9]=0x14;
	
	res = _CFG_SetConfigInfoBlk4(0xc0, 0xc0000, data, cfgHandle);
	
	res = _CFG_UpdateConfigSavegame(cfgHandle);
	
	return res;
}

Result render(int cursor){
	int y=12*8;
	renderString("  Install      *HAX       ", 0, y);
	renderString("    Uninstall  *HAX       ", 0, y+8);
	renderString("  Install      unSAFE_MODE", 0, y+16);
	renderString("    Uninstall  unSAFE_MODE", 0, y+24);
	renderString("  Dump DSiWare (fredtool)", 0, y+32);
	renderString("  Exit                   ", 0, y+40);
	renderString("->", 0, y+(cursor*8));
	return 0;
}

Result confirm(int y){
	renderString("Press A to confirm     ",0 , y);
	
	while(1){
		if(HID_PAD & PAD_A) break;
	}
	return 0;
}

Result _AM_GetTitleInfo(u32 mediatype, u32 titleCount, u64 *titleIds, AM_TitleEntry *titleInfo, Handle amHandle) //not necessary and a "big" size cost, but it's the right thing to do if it can be squeezed in.
{
	
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030084; // 0x00030084
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleEntry),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleInfo;

	if((ret = svc_sendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
	
	
	return 0;
}

Result _AM_ExportTwlBackup(u64 titleID, u8 operation, u32 *workbuf, u32 workbuf_size, char *path, Handle amHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u16 *filepath=(u16*)(workbuf-0x100);

	u32 len=_strlen(path); //"sdmc:/42383841.bin"
	if(len > 255) len=255;
	
	for(int i=0; i < len ; i++){
		filepath[i]=*(path+i) & 0xFF;
	}
	filepath[len]=0;
	len=(len+1)*2;

	cmdbuf[0] = 0x001B0144;
	cmdbuf[1] = titleID & 0xffffffff;
	cmdbuf[2] = (u32)(titleID >> 32);
	cmdbuf[3] = 38;
	cmdbuf[4] = workbuf_size;
	cmdbuf[5] = operation;
	cmdbuf[6] = (38 << 4) | 0x8 | 2;
	cmdbuf[7] = (u32)filepath;
	cmdbuf[8] = (workbuf_size << 4) | 0x8 | 4;
	cmdbuf[9] = (u32)workbuf;

	if((ret = svc_sendSyncRequest(amHandle))) return ret;
	
	return (Result)cmdbuf[1];
}
/*
Result export(u32 tidlow, u32 tidhigh, char *export_path, u8 *wbuff){
	Result (* _export)(u32,u32,char*,u8*) = (void *)AM_TwlExport;  //this is amazing. works perfectly, and even deletes the annoying nvram backup.
	return _export(tidlow, tidhigh, export_path, wbuff);
}
*/

int main(int loaderparam, char** argv)
{
	workbuf=(u8*)0x00620000;
	//u8 *slot1=(u8*)(0x00682000+0x1F000);
	//Handle nptr;
	//Handle cfgHandle;  using defines to avoid globals that require DATA and BSS section
	Handle nsHandle;
	Handle amHandle;
	//Handle cfgHandle;
	Result res;
	int cursor=0;
	int oldcurs=cursor;
	int y=4*8;
	int iscfw=0;
	AM_TitleEntry t;
	//u8 region=*(u8*)0x297600; //I'm not crazy about this, but if mset offsets won't work for a codebin, then the ropbin before it definitely should have failed too so no net loss of trust.
	region=0xff;
	t.version=0xffff;
	
	u64 menus[7]={  //index will correspond to region value
		0x0004003000008202LL, //JPN
		0x0004003000008F02LL, //USA
		0x0004003000009802LL, //EUR
		0x0004003000009802LL, //AUS same as EUR
		0x000400300000A102LL, //CHN
		0x000400300000A902LL, //KOR
		0x000400300000B102LL  //TWN
	};
	
	u64 dsint=0x0004800542383841LL;
	u64 dsdlp=0x00048005484E4441LL;
	u64 krdlp=0x00048005484E444BLL;
	
	Handle* gspHandle=(Handle*)GSPGPU_SERVHANDLEADR;
	u32 *linear_buffer = (u32*)VRAM;
	_GSPGPU_SetLcdForceBlack(*gspHandle, 0); //our bb3 loader sets this to black, so we have to undo it to see anything

	// put framebuffers in linear mem so they're writable
	u32* top_framebuffer = linear_buffer;
	u32* low_framebuffer = (linear_buffer+0x46500);
	_GSPGPU_SetBufferSwap(*gspHandle, 0, (GSP_FramebufferInfo){0, (u32*)top_framebuffer, (u32*)top_framebuffer, 240 * 3, (1<<8)|(1<<6)|1, 0, 0});
	_GSPGPU_SetBufferSwap(*gspHandle, 1, (GSP_FramebufferInfo){0, (u32*)low_framebuffer, (u32*)low_framebuffer, 240 * 3, 1, 0, 0});
	
	 
	drawTitleScreen("");
	
	 
	_initSrv(&srvHandle);
	//gHandle=aptr;
	
	res = _srv_getServiceHandle(&srvHandle, &amHandle, "am:net");	
	if(!res) iscfw=1;
	else {
		res = _srv_getServiceHandle(&srvHandle, &amHandle, "am:sys");	
		renderString("am:sys    ", 0, y+0*8);
		drawHex(res, 8*8, y+0*8); //ns:s get result
	}
	
	//_initSrv(&nptr);
	res = _srv_getServiceHandle(&srvHandle, &nsHandle, "ns:s");
	renderString("ns:s    ", 0, y+1*8);
	drawHex(res, 8*8, y+1*8); //ns:s get result
	
	res = _srv_getServiceHandle(&srvHandle, &cfgHandle, "cfg:i");
	renderString("cfg:i   ", 0, y+2*8);
	drawHex(res, 8*8,  y+2*8); //cfg:i get result
	if(res){
		renderString("WHAT IS WRONG WITH THE ELF?", 0, y+2*8);
		renderString("Hold power to turn off :(", 0, y+2*8);
		while(1) svc_sleepThread(100*1000*1000);
	}
	
	if(iscfw){
		renderString("cfw installed already!    ", 0,  y+4*8);	
	}

	res = CFGU_SecureInfoGetRegion(&region, cfgHandle);
	if(!res) res = _AM_GetTitleInfo(0, 1, &menus[region], &t, amHandle);
	
	render(cursor);
	check_slots();
	
	while(1){
		svc_sleepThread(17*1000*1000);
		if     (HID_PAD & PAD_UP)   cursor--;
		else if(HID_PAD & PAD_DOWN) cursor++;
		
		if      (cursor < 0) cursor=5;
		else if (cursor > 5) cursor=0;
		
		if(cursor ^ oldcurs){
			render(cursor); 
			svc_sleepThread(250*1000*1000);
		}
		
		if(HID_PAD & PAD_A) break;
		oldcurs=cursor;
		//clear_slots();
	}
	
	switch(cursor){
		
		case 0:
		if(!iscfw){
			menuhax67(t.version);
			renderString("*HAX INSTALLED!!", 0, 20*8);   //try to be super obvious what's happened 
			svc_sleepThread(500*1000*1000);		    //so hopefully no unnecessary trips to discord due to confusion
			renderString("Rebooting...   ", 0, 22*8);
			svc_sleepThread(1000*1000*1000);
			NS_RebootSystem(nsHandle, 0x160000);            //this is a power down
			while(1) svc_sleepThread(100*1000*1000); 
		}
		else{
			renderString("No need to rehack!", 0, 20*8);
			svc_sleepThread(2000*1000*1000);	
			break;
		}
	
		case 1:
		uninstall();
		renderString("Uninstalled!!  ", 0, 20*8);   //try to be super obvious what's happened 
		svc_sleepThread(500*1000*1000);		    //so hopefully no unnecessary trips to discord due to confusion
		//confirm(17*8);
		renderString("Rebooting now...       ", 0, 22*8);
		svc_sleepThread(2000*1000*1000);
		break;
		
		case 2:
		if(!iscfw){
			inject_slots();
			renderString("unSAFE_MODE INSTALLED!!", 0, 20*8);   //try to be super obvious what's happened 
			svc_sleepThread(500*1000*1000);		    //so hopefully no unnecessary trips to discord due to confusion
			//confirm(17*8);
			renderString("Shutting down now...   ", 0, 22*8);
			svc_sleepThread(2000*1000*1000);
			NS_RebootSystem(nsHandle, 0xE0000);                 //this is a power down
			while(1) svc_sleepThread(100*1000*1000); 
		}
		else{
			renderString("No need to rehack!", 0, 20*8);
			svc_sleepThread(2000*1000*1000);	
			break;
		}
		
		case 3:
		restore_slots();
		renderString("Wifi slots restored!!  ", 0, 20*8);   //try to be super obvious what's happened 
		svc_sleepThread(500*1000*1000);		    //so hopefully no unnecessary trips to discord due to confusion
		//confirm(17*8);
		renderString("Rebooting now...       ", 0, 22*8);
		svc_sleepThread(2000*1000*1000);
		break;
		
		case 4:
		if(!iscfw){
			y=180;
			
			res = _AM_ExportTwlBackup(dsint, 1, (u32*)low_framebuffer, 0x00020000, "sdmc:/42383841.bin", amHandle);
			drawHex(res, 10, y+=10); if(!res) renderString("OK - sd:/42383841.bin", 10, y+=10);
			svc_sleepThread(2000*1000*1000); if(!res) goto reboot;
			
			res = _AM_ExportTwlBackup(dsdlp, 1, (u32*)low_framebuffer, 0x00020000, "sdmc:/484E4441.bin", amHandle);
			drawHex(res, 10, y+=10); if(!res) renderString("OK - sd:/484E4441.bin", 10, y+=10);
			svc_sleepThread(2000*1000*1000); if(!res) goto reboot;
			
			res = _AM_ExportTwlBackup(krdlp, 1, (u32*)low_framebuffer, 0x00020000, "sdmc:/484E444B.bin", amHandle);	
			drawHex(res, 10, y+=10); if(!res) renderString("OK - sd:/484E444B.bin", 10, y+=10);
			svc_sleepThread(2000*1000*1000);	
		}
		else{
			renderString("No need to rehack!", 0, 20*8);
			svc_sleepThread(2000*1000*1000);	
			break;
		}
		
		reboot:
		renderString("Rebooting now...", 10, y+=10);
		break;
		
		case 5:
		renderString("Rebooting now...       ", 0, 20*8);
		svc_sleepThread(2000*1000*1000);
		break;
		
		default:;
	}
	
	NS_RebootSystem(nsHandle, 0x160000);
	while(1) svc_sleepThread(100*1000*1000);
	return 0;
}