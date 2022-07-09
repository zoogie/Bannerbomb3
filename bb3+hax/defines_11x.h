#define ROP_POPPC 0x0011a1d4
#define POP_R1PC 0x001d53f8
#define POP_R3PC 0x0010d44c
#define POP_R2R6PC 0x001bb020
#define POP_R4LR_BXR1 0x00119e4c
#define POP_R4R8LR_BXR2 0x0011f1e0
#define POP_R4R5R6PC 0x0010d008
#define POP_R4FPPC 0x0010e6c0
#define POP_R4R8PC 0x0010d20c

#define ROP_STR_R1TOR0 0x00119e14
#define ROP_STR_R0TOR1 0x0010d374
#define ROP_LDR_R0FROMR0 0x0010d364
#define ROP_ADDR0_TO_R1 0x00111940

#define MEMCPY 0x001d0d5c

#define svcSleepThread 0x001bdbd0

#define GSPGPU_FlushDataCache 0x0013e46c
#define GSPGPU_SERVHANDLEADR 0x002993c4

#define IFile_Read 0x001c3140
#define IFile_Write 0x001c73fc

#define ROP_POPR3_ADDSPR3_POPPC 0x0014660c
#define POP_R0PC 0x00146760
#define ROP_LDRR1R1_STRR1R0 0x001adf34
//      FAIL:                           #define POP_R5R6PC
//      FAIL:                           #define ROP_CMPR0R1
#define ROP_CMPR0R1_ALT0 0x001e4f84
#define MEMSET32_OTHER 0x001d5a60
#define svcControlMemory 0x001d3e90
#define ROP_INITOBJARRAY 0x001c5865
#define svcCreateThread 0x0010c698
#define svcConnectToPort 0x001c62a4
#define svcGetProcessId 0x0012bddc
//      FAIL:                           #define THROWFATALERR_IPC
#define SRV_GETSERVICEHANDLE 0x001d3f18
#define CFGIPC_SecureInfoGetRegion 0x00118768
#define ROP_COND_THROWFATALERR 0x001d4570
#define GXLOW_CMD4 0x0013e570
#define GSP_SHAREDMEM_SETUPFRAMEBUF 0x0012e0fc
#define GSPTHREAD_OBJECTADDR 0x00296580
#define FS_MountSdmc 0x001a1654
//      FAIL:                           #define FS_MountSavedata
#define IFile_Open 0x001c790c
#define IFile_Close 0x001c78c8
#define IFile_Seek 0x001b335c

/*
sub_239638:
  80106e8
  2e00000
  8010674
  297600 (getRegion result)
  26B460 (table of tid high/low pairs from versionDB title, corresponds to getRegion with a "times 8"
  v13 = sub_1C7C6C((int)"cver:", (int)dword_26B460, dword_26B460[2], dword_26B460[3], 0, 5, 5);
  v25 = 0;
  v14 = (_DWORD *)sub_1D2B8C((int)L"cver:/version.bin", &v25, 0, 0);
*/
