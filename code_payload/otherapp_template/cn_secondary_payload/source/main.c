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

#define HID_PAD (*(vu32*)0x1000001C)
#define GSPGPU_FlushDataCache 0x0013e46c
#define GSPGPU_SERVHANDLEADR 0x002993c4
#define VRAM ((u32)0x31000000)
//u8 *top_ptr=(u8*)1;

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
	centerString("Bannerbomb3",0);
	centerString("https://3ds.hacks.guide/",10);
	centerString("Help: https://discord.gg/C29hYvh",20);
	renderString(str, 0, 70);
}

Result NS_RebootSystem(Handle handle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x160000;//0xE0000; //the second number is the shutdown command header
	
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

Result _AM_ExportTwlBackup(u64 titleID, u8 operation, u32 *workbuf, u32 workbuf_size, const char *path, Handle amHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u16 filepath[256]={0};

	u32 len=_strlen(path); //"sdmc:/42383841.bin"
	if(len > 255) len=255;
	
	for(int i=0; i < len ; i++){
		filepath[i]=*(path+i) & 0xFF;
	}
	
	len=(len+1)*2;

	cmdbuf[0] = 0x001B0144;
	cmdbuf[1] = titleID & 0xffffffff;
	cmdbuf[2] = (u32)(titleID >> 32);
	cmdbuf[3] = len;
	cmdbuf[4] = workbuf_size;
	cmdbuf[5] = operation;
	cmdbuf[6] = (len << 4) | 0x8 | 2;
	cmdbuf[7] = (u32)filepath;
	cmdbuf[8] = (workbuf_size << 4) | 0x8 | 4;
	cmdbuf[9] = (u32)*workbuf;

	if((ret = svc_sendSyncRequest(amHandle))) return ret;
	
	return (Result)cmdbuf[1];
}

int main(int loaderparam, char** argv)
{
	u32 workbuf=0x00680000;
	Handle hptr;
	Handle nptr;
	Handle amHandle;
	Handle nsHandle;
	Result res;
	int y=40;
	u64 dsint=0x0004800542383841;
	u64 dsdlp=0x00048005484E4441;
	u64 krdlp=0x00048005484E444B;
	
	Handle* gspHandle=(Handle*)GSPGPU_SERVHANDLEADR;
	u32 *linear_buffer = (u32*)VRAM;

	// put framebuffers in linear mem so they're writable
	u32* top_framebuffer = linear_buffer;
	u32* low_framebuffer = (linear_buffer+0x46500);
	_GSPGPU_SetBufferSwap(*gspHandle, 0, (GSP_FramebufferInfo){0, (u32*)top_framebuffer, (u32*)top_framebuffer, 240 * 3, (1<<8)|(1<<6)|1, 0, 0});
	_GSPGPU_SetBufferSwap(*gspHandle, 1, (GSP_FramebufferInfo){0, (u32*)low_framebuffer, (u32*)low_framebuffer, 240 * 3, 1, 0, 0});
	
	drawTitleScreen("DSiWare exporting...");
	
	 _initSrv(&hptr);
	 
	res = _srv_getServiceHandle(&hptr, &amHandle, "am:sys");
	drawHex(res, 10, y); //am:sys get result
	
	_initSrv(&nptr);
	res = _srv_getServiceHandle(&nptr, &nsHandle, "ns:s");
	drawHex(res, 82, y); //ns:s get result
	
	y+=50;
	
	while(1){
		res = _AM_ExportTwlBackup(dsint, 5, &workbuf, 0x00020000, "sdmc:/42383841.bin", amHandle);
		drawHex((u32)dsint, 10, y);     drawHex(res, 82, y); if(!res) break;
		
		res = _AM_ExportTwlBackup(dsdlp, 5, &workbuf, 0x00020000, "sdmc:/484E4441.bin", amHandle);
		drawHex((u32)dsdlp, 10, y+=10); drawHex(res, 82, y); if(!res) break;
		
		res = _AM_ExportTwlBackup(krdlp, 5, &workbuf, 0x00020000, "sdmc:/484E444B.bin", amHandle);
		drawHex((u32)krdlp, 10, y+=10); drawHex(res, 82, y); break;
	}
	
	if(!res) centerString("Overall: Success!", y+=40);
	else	 centerString("Overall: FAIL", y+=40);
	centerString("Press A to exit", y+=20);
	
	while(1){
		svc_sleepThread(17*1000*1000);
		if(HID_PAD & PAD_A) break;
	}
 
	drawTitleScreen("Rebooting...");
	NS_RebootSystem(nsHandle);
	while(1) svc_sleepThread(100*1000*1000);
	return 0;
}
