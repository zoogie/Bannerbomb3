#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/svc.h>
#include <ctr/srv.h>

extern Handle gspGpuHandle;

Result GSPGPU_ImportDisplayCaptureInfo(Handle* handle, GSP_CaptureInfo *captureinfo)
{
	if(!handle)handle=&gspGpuHandle;
	
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

Result GSPGPU_SaveVramSysArea(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00190000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RestoreVramSysArea(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x001A0000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(Handle* handle, u8 flags)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; //request header code
	cmdbuf[1]=flags;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetBufferSwap(Handle* handle, u32 screenid, GSP_FramebufferInfo *framebufinfo)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	if(!handle)handle=&gspGpuHandle;

	cmdbuf[0] = 0x00050200;
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], framebufinfo, sizeof(GSP_FramebufferInfo));
	
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(Handle* handle, u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize)
{
	if(!handle)handle=&gspGpuHandle;
	
	if(datasize>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x20084; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=datasize;
	cmdbuf[3]=(datasize<<14)|2;
	cmdbuf[4]=(u32)data;
	cmdbuf[5]=(masksize<<14)|0x402;
	cmdbuf[6]=(u32)maskdata;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(Handle* handle, u32 regAddr, u32* data, u8 size)
{
	if(!handle)handle=&gspGpuHandle;
	
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x40080; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=(size<<14)|2;
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}
